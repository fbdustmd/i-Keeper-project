#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(FILE *stream)
{
    fputs("NetSentry - 학습용 네트워크 분석\n"
          "사용법: netsentry [--help]\n"
          "Phase 0: 프로젝트 기본 구조. 패킷 캡처는 아직 구현되지 않았습니다.\n",
          stream);
}

int main(int argc, char *argv[])
{
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
        print_usage(stdout);
        return EXIT_SUCCESS;
    }

    fputs("오류: 지원하지 않는 인자입니다.\n", stderr);
    print_usage(stderr);
    return EXIT_FAILURE;
}
