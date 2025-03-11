# memory layout 비교(C vs Java)

### C 언어의 메모리 레이아웃

- **C++**, **Rust**, **Go** 등 언어에서도 적용

| **Segment** | **저장되는 데이터** |
| ----------- | ------------------- |

| 코드 영역
(Text Segment) | 프로그램의 실행 가능한 코드 |
| 데이터 영역
(Data Segment) | 1. Initialized Data Segment :
초기화된 전역변수, 정적변수

2. Uninitialized Data Segment(BSS) :
   초기화되지 않은 전역변수, 정적변수 |
   | 힙 영역
   (Heap Segment) | 동적 메모리 할당 시 사용됨 |
   | 스택 영역
   (Stack Segment) | 함수 호출 시 지역변수, 매개변수, 반환 주소 등이 저장 |

### Java의 메모리 레이아웃

- JVM 기반 언어(ex. **Kotlin**)에서도 적용

| **Area** | **저장되는 데이터** |
| -------- | ------------------- |

| 메서드 영역
(Method Area) | 클래스 구조, 메서드 정보, 필드 정보, 정적변수 |
| 힙 영역
(Heap Area) | 객체와 배열
(GC에 의해 관리됨) |
| 스택 영역
(Stack Area) | 스레드마다 존재
스택 프레임에 지역변수, 매개변수, 연산 중간 결과 등이 저장됨 |
| PC 레지스터
(PC Register) | 각 스레드가 현재 실행 중인 JVM 명령의 주소를 저장 |
| 네이티브 메서드 스택
(Native Method Stack) | 다른 언어(C/C++)로 작성된 네이티브 코드를 실행할 때 사용 |

### C 언어

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 초기화되지 않은 전역 변수 (BSS 영역)
int global_var;

// 초기화된 전역 변수 (데이터 영역)
int initialized_global_var = 10;

void function() {
    // 스택에 저장되는 지역 변수
    int local_var = 5;

    // 힙에 메모리 할당
    char *heap_var = (char *)malloc(20 * sizeof(char));
    if (heap_var != NULL) {
        strcpy(heap_var, "Hello, World!");
        printf("%s\n", heap_var);
        // 메모리 해제
        free(heap_var);
    }
}

int main() {
    function();
    return 0;
}
```

### Java

```java
public class Main {
    // 메서드 영역에 저장되는 정적 변수
    static int staticVar = 10;

    public static void main(String[] args) {
        // 스택에 저장되는 지역 변수
        int localVar = 5;

        // 힙에 저장되는 객체
        String heapVar = new String("Hello, World!");

        System.out.println(heapVar);
    }
}
```
