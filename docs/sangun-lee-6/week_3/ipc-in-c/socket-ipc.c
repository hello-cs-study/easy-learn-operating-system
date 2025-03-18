/** UNIX 도메인 소켓을 이용한 IPC */
/**
 * sys/socket.h : 소켓 관련 함수 제공
 * sys/un.h : UNIX 도메인 소켓 주소 구조체 sockaddr_un 제공
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

/** 매크로 정의
 * SOCKET_PATH : UNIX 소켓 파일 경로
 * BUFFER_SIZE : 버퍼 크기
*/
#define SOCKET_PATH "/tmp/ipc_socket"
#define BUFFER_SIZE 1024

void psearch1d(char* color_name, int input_file_count, char* input_filenames[], char* output_filename) {

    /** 소켓 변수 선언
     * server_fd: 서버용 소켓의 파일 디스크립터
     * client_fd: 클라이언트 연결을 수락할 때 생성되는 소켓의 파일 디스크립터
     * buffer: IPC로 수신된 메시지를 저장할 버퍼
     * message: IPC로 보낼 메시지를 저장할 버퍼
    */
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    /** 소켓 생성
     * UNIX 도메인 소켓(AF_UNIX) 을 생성
     * SOCK_STREAM: TCP 방식의 스트림형 통신 (순차적이고 신뢰성 있는 데이터 전달)
     */
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    /** 서버 소켓 주소 설정 및 기존 소켓 삭제
     * memset: server_addr 구조체 초기화 (오류 방지)
     * unlink(SOCKET_PATH): 기존의 동일한 이름의 소켓 파일을 삭제해 충돌 방지
    */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);
    unlink(SOCKET_PATH); // 기존 소켓 파일 삭제

    /** 소켓 바인딩
     * 소켓 파일(`/tmp/ipc_socket`)과 소켓 fd를 연결
    */
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket bind failed");
        exit(1);
    }

    /** 소켓 대기상태로 전환
     * 소켓이 클라이언트의 연결 요청을 기다림
    */
    if (listen(server_fd, input_file_count) == -1) {
        perror("Socket listen failed");
        exit(1);
    }

    for (int i = 0; i < input_file_count; ++i) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork Failed");
            exit(1);
        }
        /** 자식 프로세스
        */ 
        else if (pid == 0) {
            int sock_fd;
            struct sockaddr_un server_addr;
            char message[BUFFER_SIZE];

            /** 자식 프로세스 소켓 생성
             * 자식은 자신의 클라이언트용 소켓을 만들어 부모와 통신
            */
            if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
                perror("Socket creation failed in child");
                exit(1);
            }

            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sun_family = AF_UNIX;
            strcpy(server_addr.sun_path, SOCKET_PATH);

            /** 부모와 연결(connect)
             * 자식 프로세스가 부모가 생성한 서버 소켓에 연결
             * 데이터를 주고받기 위한 통신 채널
            */
            if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
                perror("Socket connection failed in child");
                exit(1);
            }

            // 파일에서 색상 검색
            FILE* file = fopen(input_filenames[i], "r");
            if (!file) {
                perror("File open error");
                exit(1);
            }

            int color_count = 0, total_color_count = 0;
            char word[100];
            while (fscanf(file, "%s", word) != EOF) {
                if (strcmp(word, color_name) == 0) {
                    color_count++;
                }
                total_color_count++;
            }
            fclose(file);

            /** 부모 프로세스에게 결과 전송 */
            sprintf(message, "%s  %s  %d/%d\n", input_filenames[i], color_name, color_count, total_color_count);
            send(sock_fd, message, strlen(message), 0);
            close(sock_fd);
            exit(0);
        }
    }

    /** 부모 프로세스 
    */

    /** 결과 파일 생성 */
    FILE* final_output_file = fopen(output_filename, "w");
    if (final_output_file == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    for (int i = 0; i < input_file_count; ++i) {
        /** 클라이언트 연결 수락
         * 부모 프로세스는 자식 프로세스에서 보내온 연결 요청을 수락
         * 수락할 때마다 별도의 파일 디스크립터(client_fd)가 생성됨
        */
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("Socket accept failed");
            exit(1);
        }

        /** 자식 프로세스로부터 데이터 수신 */
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_fd, buffer, BUFFER_SIZE, 0);
        fprintf(final_output_file, "%s", buffer);
        close(client_fd);
    }

    fclose(final_output_file);
    close(server_fd);
    unlink(SOCKET_PATH);  // 소켓 파일 삭제
}

int main(int argc, char *argv[]) {
    char* color_name = argv[1];
    int count_of_input_file = atoi(argv[2]);
    char* input_filenames[count_of_input_file];
    char* output_filename = argv[count_of_input_file+3];

    for (int i = 0; i < count_of_input_file; ++i) {
        input_filenames[i] = argv[i+3];
    }

    psearch1d(color_name, count_of_input_file, input_filenames, output_filename);
}
