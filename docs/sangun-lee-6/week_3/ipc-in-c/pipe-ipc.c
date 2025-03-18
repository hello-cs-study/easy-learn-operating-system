/**
 * 익명 파이프를 이용한 ipc
 * 프로그램 실행 방법
 * gcc pipe-ipc.c -o pipe-ipc
 * ./pipe-ipc red 2 color1.txt color2.txt output-pipe.txt
*/

/**
 * `stdio.h`: 표준 입출력 함수(`printf()`, `fopen()`, `fprintf()` 등)
 * `unistd.h`: `pipe()`, `fork()`, `read()`, `write()` 같은 시스템 호출 제공
 * `stdlib.h`: `exit()`, `malloc()`, `atoi()` 등의 함수 포함
 * `memory.h`: 메모리 관련 함수 (`memset()`, `memcpy()` 등)
 * `sys/wait.h`: `wait()`을 사용하여 부모가 자식 프로세스 종료를 기다리도록 함
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/wait.h>

/** 파이프에서 데이터를 읽고 쓰는 데 사용할 매크로 상수
 * `READ = 0` → 파이프의 읽기 끝(index 0)
 * `WRITE = 1` → 파이프의 쓰기 끝(index 1)
*/
#define READ 0
#define WRITE 1

char* read_file(char* file_name) {

    long lSize;
    char* buffer;
    FILE* file = fopen(file_name, "rb");

    if (!file) {
        fclose(file);
        perror("Occurred an error while reading file!");
        exit(1);
    }

    fseek( file , 0L , SEEK_END);
    lSize = ftell( file );
    rewind( file );

    /* Allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) {
        fclose(file);
        fputs("Memory alloc fails!", stderr);
        exit(1);
    }

    /* Copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , file) ) {
        fclose(file);
        free(buffer);
        fputs("Entire read fails!", stderr);
        exit(1);
    }

    fclose(file);
    return buffer;
}

static int myCompare (const void * a, const void * b) {    
    return strcmp (*(const char **) a, *(const char **) b);
}

void sort(char *arr[], int n) {
    qsort (arr, n, sizeof (const char *), myCompare);
}

int psearch1c(char* color_name, int input_file_count, char* input_filenames[], char* output_filename) {

    /** 각 파일마다 파이프를 하나씩 생성하기 위해 `input_file_count` 크기의 2차원 배열 선언
     * `pfd[i][0]`: `i`번째 파이프의 읽기(READ) 끝
     * `pfd[i][1]`: `i`번째 파이프의 쓰기(WRITE) 끝
    */
    int pfd[input_file_count][2];


    /**
     * 각 파일의 수만큼 pipe와 자식 프로세스 생성
    */
    for (int i = 0; i < input_file_count; ++i) {
        if (pipe(pfd[i]) != 0) {
            perror("Pipe Failed!");
            exit(1);
        }

        char *reading_file = input_filenames[i];

        pid_t pid = fork();

        if (pid < 0) { 
            fprintf(stderr, "Fork Failed!");
            return 1;
        }
        // Child Process 
        else if (pid == 0) {

            // 읽기 파이프 닫기
            /** 읽기 파이프 닫기
             * 자식 프로세스는 데이터를 부모 프로세스에게 보내는 역할만 하므로, 읽기 파이프(READ)를 닫음
            */ 
            close(pfd[i][READ]);

            /** 파일에서 색상 검색 */
            char *file_content = read_file(reading_file);
            int color_count = 0;
            int total_color_count = 0;

            char *color = strtok(file_content, " \n");

            while (color != NULL) {
                if (strcmp(color, color_name) == 0) {
                    color_count++;
                }
                total_color_count++;
                color = strtok(NULL, " \n");
            }

            char new_message[100];
            sprintf(new_message, "%s  %s  %d/%d\n",reading_file, color_name,  color_count, total_color_count);

            /** 파이프를 통해 부모 프로세스에게 데이터 전송 */
            if (write(pfd[i][WRITE], new_message, 100) == -1) {
                perror("Write didn't return expected value!\n");
                exit(2);
            }

            /** 쓰기 파이프 닫기 */
            close(pfd[i][WRITE]);
            // 프로세스 종료
            exit(0);
        }
    }

    // Parent process
    
    // 최종 결과를 동적 할당
    char* final_output = (char*) malloc(sizeof(char)*100*input_file_count);

    for (int k = 0; k < input_file_count; ++k) {
        // 쓰기 파이프 닫기
        close(pfd[k][WRITE]);

        // 각 자식 파이프의 데이터를 읽음
        char* temp_output = (char*) malloc(sizeof(char)*100);
        if (read(pfd[k][READ], temp_output, 100) == -1) {
            perror("Read didn't return expected value!");
            exit(3);
        }
        // 해당 읽기 파이프 닫기
        close(pfd[k][READ]);
        // 데이터 복사
        strcat(final_output, temp_output);
    }

    int status;
    for (int j = 0; j < input_file_count; ++j) {
        wait(&status);
    }


    FILE *final_output_file = fopen(output_filename , "w");

    if (final_output_file == NULL) {
        perror("Error occurred when writing file!");
        exit(0);
    }
    fprintf(final_output_file, "%s", final_output);
    fclose(final_output_file);

    free(final_output);
    return 0;
}


int main(int argc, char *argv[]) {


    char* color_name = argv[1];
    int count_of_inout_file = atoi(argv[2]);
    char* input_filenames[count_of_inout_file];
    char* output_filename = argv[count_of_inout_file+3];

    for (int i = 0; i < count_of_inout_file; ++i) {
        input_filenames[i] = argv[i+3];
    }
    int n = sizeof(input_filenames)/sizeof(input_filenames[0]);
    sort(input_filenames, n);

    psearch1c(color_name, count_of_inout_file, input_filenames, output_filename);

}