// macos에 따라 아래와 같이 실행해야함?
// gcc shared-memory-ipc.c -o shared-memory-ipc -lpthread

/**
 * `stdio.h`: 표준 입출력 함수(`printf()`, `fopen()` 등)
 * `unistd.h`: `fork()`, `getpid()` 같은 시스템 호출 제공
 * `stdlib.h`: `exit()`, `malloc()`, `atoi()` 등의 함수 포함
 * `memory.h`: 메모리 관련 함수(`memset()`, `memcpy()` 등)
 * `sys/wait.h`: `wait()`을 사용하여 부모가 자식 프로세스 종료를 기다리도록 함
 * `sys/mman.h`: `mmap()`을 사용하여 공유 메모리 생성
 * `semaphore.h`: 세마포어(`sem_t`)를 사용하여 공유 자원 접근을 제어
 * `fcntl.h`: 파일 제어 관련 함수 (`O_CREAT`, `O_RDWR` 등)
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

/** 메크로 : 공유메모리 크기 
 * 4KB 크기의 공유 메모리를 생성
 * 공유 메모리는 여러 프로세스가 데이터를 주고받는 공간으로 활용됨
*/
#define PAGESIZE 4096

int psearch1b(char* color_name, int input_file_count, char* input_filenames[], char* output_filename) {
    /**`mmap()`을 사용하여 프로세스 간 공유할 메모리를 생성
     * `NULL`: 운영체제가 적절한 주소를 할당
     * `PAGESIZE`: 4KB 크기의 메모리 공간을 생성
     * `PROT_READ | PROT_WRITE`: 읽기 및 쓰기가 가능하도록 설정
     * `MAP_SHARED`: 자식 프로세스와 공유 가능하도록 설정
     * `MAP_ANONYMOUS`: 파일이 아닌, 프로세스 간 공유 메모리로 사용
    */
    char* message_in_shared_memory = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (message_in_shared_memory == MAP_FAILED) {
        perror("mmap Failed!");
        exit(1);
    }

    /** 세마포어를 커널 영역에 생성 : 여러 프로세스가 공유 메로리에 동시 접근하지 않도록 보호
     * `"/mysemaphore"`: 이름이 있는(named) 세마포어 생성
     * `O_CREAT`: 세마포어가 없으면 생성
     * `0644`: 세마포어 접근 권한 설정
     * `1`: 초기값(`1`) → 한 번에 하나의 프로세스만 접근 가능
    */
    sem_t *mutex = sem_open("/mysemaphore", O_CREAT, 0644, 1);
    if (mutex == SEM_FAILED) {
        perror("Semaphore initialization Failed!");
        exit(1);
    }

    for (int i = 0; i < input_file_count; ++i) {
        /** 파일 개수만큼 fork()를 호출하여 각 파일을 처리하는 자식 프로세스를 생성 */
        char *reading_file = input_filenames[i];
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Fork Failed!");
            return 1;
        }
        // Child Process 
        else if (pid == 0) {
            // 세마포어 값 1 갑소, 값이 0이라면 다른 프로세스가 공유 메모리를 수정하는 동안 대기
            sem_wait(mutex);  

            // 현재 자식 프로세스가 처리할 파일을 열기
            FILE *file = fopen(reading_file, "r");
            if (!file) {
                perror("File open error!");
                exit(1);
            }

            /** 색상 검색 후 결과 저장 */
            int color_count = 0, total_color_count = 0;
            char word[100];
            while (fscanf(file, "%s", word) != EOF) {
                if (strcmp(word, color_name) == 0) {
                    color_count++;
                }
                total_color_count++;
            }
            fclose(file);

            /** 공유 메모리에 결과 기록 */
            char new_message[100];
            sprintf(new_message, "%s  %s  %d/%d\n", reading_file, color_name, color_count, total_color_count);
            strcat(message_in_shared_memory, new_message);

            // 세마포어 해제 및 자식 프로세스 종료(세마포어 값 1 증가)
            sem_post(mutex);  
            exit(0);
        }
    }

    // Parent Process

    // 자식 프로세스가 끝날 때까지 대기
    int status;
    for (int j = input_file_count; j > 0; j--) {
        wait(&status);
    }

    // 출력 파일 생성 및 공유 메모리의 결과 기록
    FILE *final_output_file = fopen(output_filename, "w");
    if (final_output_file == NULL) {
        perror("Error occurred when writing file!");
        exit(0);
    }
    fprintf(final_output_file, "%s", message_in_shared_memory);
    fclose(final_output_file);

    sem_close(mutex);  // 세마포어 닫기
    sem_unlink("/mysemaphore");  // 세마포어 제거

    return 0;
}

int main(int argc, char *argv[]) {
    char* color_name = argv[1];
    int count_of_input_file = atoi(argv[2]);
    char* input_filenames[count_of_input_file];
    char* output_filename = argv[count_of_input_file+3];

    for (int i = 0; i < count_of_input_file; ++i) {
        input_filenames[i] = argv[i+3];
    }

    psearch1b(color_name, count_of_input_file, input_filenames, output_filename);
}
