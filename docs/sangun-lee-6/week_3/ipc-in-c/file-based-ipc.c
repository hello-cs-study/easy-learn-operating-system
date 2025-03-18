/**
 * 헤더 파일
 * stdio.h : 표준 입출력을 위한 헤더
 * unistd.h : fork(), getpid() 같은 Unix 시스템 호출 사용
 * stdlib.h : malloc(), free(), exit(), atoi() 등의 표준 라이브러리 함수 포함
 * memory.h : 메모리 관련 함수 (memset(), memcpy() 등)
 * sys/wait.h : 부모 프로세스가 자식 프로세스를 기다리는 wait() 함수 포함
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/wait.h>

/** 주어진 파일의 내용을 읽어서 메모리(버퍼)에 저장하는 함수 */
char* read_file(char* file_name) {  

    long lSize;
    char* buffer;

    /** 파일 읽기
     * FILE*을 사용해 바이너리 모드(rb)로 파일을 열기
     * file 포인터가 NULL이면 파일을 열 수 없는 상황이므로 오류 처리
    */
    FILE* file = fopen(file_name, "rb"); // 
    if (!file) {
        fclose(file);
        perror("Occurred an error while reading file!");
        exit(1);
    }

    /** 파일 크기 계산(lSize)
     * fseek()을 이용해 파일의 끝으로 이동 후 ftell()로 파일 크기(lSize)를 얻음
     * rewind()를 이용해 다시 파일의 처음으로 이동
    */
    fseek( file , 0L , SEEK_END);
    lSize = ftell( file );
    rewind( file );

    /** 메모리 할당
     * calloc()을 사용해 파일 크기만큼 메모리를 할당 (초기화 포함)
     * 메모리 할당 실패 시 오류 출력 후 종료
    */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) { 
        fclose(file);
        fputs("Memory alloc fails!", stderr);
        exit(1);
    }
    
    /** 파일 내용을 읽어서 buffer에 저장
     * fread()를 이용해 파일의 모든 내용을 buffer에 저장
     * 실패 시 오류 출력 후 종료
    */
    if( 1!=fread( buffer , lSize, 1 , file) ) {
        fclose(file);
        free(buffer);
        fputs("Entire read fails!", stderr);
        exit(1);
    }

    /** 파일을 닫고 buffer 반환 */
    fclose(file);
    return buffer;
}

static int myCompare (const void * a, const void * b) {    //https://www.geeksforgeeks.org/c-program-sort-array-names-strings/
    return strcmp (*(const char **) a, *(const char **) b);
}

/** 파일 이름 정렬 */
void sort(char *arr[], int n) {
    qsort (arr, n, sizeof (const char *), myCompare);
}

int psearch1a(char* color_name, int input_file_count, char* input_filenames[], char* output_filename){

    /**
     * input_file_count만큼 자식 프로세스 생성
    */
    for (int i = 0; i < input_file_count; ++i) {
        char *reading_file = input_filenames[i];
        pid_t pid = fork();

        // fork() 에러
        if (pid < 0) { 
            fprintf(stderr, "Fork Failed!");
            return 1;
        }
        // Child Process 
        else if (pid == 0) {    

            // 파일을 읽어서 내용을 저장
            char *file_content = read_file(reading_file);
            int color_count = 0;       // 파일 내 해당 색상이 등장한 횟수
            int total_color_count = 0; // 파일 내 총 단어 개수

            // strtok로 파일의 각 단어를 토큰화
            // 토큰과 color_name이 일치하면 color_count 증가
            char *color = strtok(file_content, " \n");
            while (color != NULL) {
                if (strcmp(color, color_name) == 0) {
                    color_count++;
                }
                total_color_count++;
                color = strtok(NULL, " \n");
            }

            // 임시 파일(temp_i.txt)을 생성하고 결과를 저장
            char temp_output_filename[30];
            sprintf(temp_output_filename, "%s_%d.txt", "temp", i);

            FILE *temp_output_file = fopen(temp_output_filename , "w");
            if (temp_output_file == NULL) {
                perror("Error occurred when writing file!");
                exit(0);
            }
            fprintf(temp_output_file, "%s  %s  %d/%d\n",reading_file, color_name,  color_count, total_color_count);
            fclose(temp_output_file);

            // 자식 프로세스 종료
            exit(0);
        }
    }

    // Parent process

    // wait()를 사용해 모든 자식 프로세스가 종료될 때까지 대기
    int status;   
    for (int j = input_file_count; j > 0 ; j--) {
        wait(&status);
    }

    // 최종 결과를 저장할 final_output_content를 초기화
    char* final_output_content;
    final_output_content = realloc(NULL, sizeof(char*));

    // 임시 파일들(temp_i.txt)을 읽음
    // 파일 내용을 읽고 최종 결과에 저장 후 임시 파일 삭제
    for (int i = 0; i < input_file_count; ++i) {
        char temp_output_filename[30];
        sprintf(temp_output_filename, "%s_%d.txt", "temp", i);

        final_output_content = realloc(final_output_content, sizeof(char*) * strlen(read_file(temp_output_filename)));

        strcat(strcat(final_output_content, read_file(temp_output_filename)),"\n");
        remove(temp_output_filename);
    }

    // 최종 결과를 출력할 파일 생성
    // 파일 생성 후 저장
    // 파일 닫기
    FILE *final_output_file = fopen(output_filename , "w");
    if (final_output_file == NULL) {
        perror("Error occurred when writing file!");
        exit(0);
    }
    fprintf(final_output_file, "%s", final_output_content);
    fclose(final_output_file);

    return 1;
}

int main(int argc, char *argv[]) {

    char* color_name = argv[1]; // 찾을 색상 이름
    int count_of_inout_file = atoi(argv[2]); // 입력 파일 개수
    char* input_filenames[count_of_inout_file]; // 입력 파일 이름
    char* output_filename = argv[count_of_inout_file+3]; // 최종 결과 출력 파일 이름

    for (int i = 0; i < count_of_inout_file; ++i) {
        input_filenames[i] = argv[i+3];
    }
    int n = sizeof(input_filenames)/sizeof(input_filenames[0]);
    sort(input_filenames, n);

    psearch1a(color_name, count_of_inout_file, input_filenames, output_filename);
}

