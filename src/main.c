#include "capture.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(FILE *stream)
{
    fputs("NetSentry - 학습용 네트워크 분석\n"
          "사용법: netsentry [--help]\n"
          "        netsentry --list\n"
          "        netsentry --interface NAME [--count N] [--duration SECONDS] [--filter BPF]\n"
          "기본 제한: 100개 또는 30초 중 먼저 도달한 조건. Ctrl+C로 종료합니다.\n"
          "--count: 1~1000000, --duration: 1~86400초. 인터페이스를 직접 지정하세요.\n"
          "Phase 1: 길이와 링크 타입만 출력하며 원본 패킷 내용은 출력하지 않습니다.\n",
          stream);
}

static bool parse_positive(const char *text, uint32_t maximum, uint32_t *output)
{
    uint32_t value = 0;
    if (*text == '\0') {
        return false;
    }
    for (; *text != '\0'; ++text) {
        if (*text < '0' || *text > '9') {
            return false;
        }
        uint32_t digit = (uint32_t)(*text - '0');
        if (value > (maximum - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    *output = value;
    return value != 0;
}

static bool parse_options(int argc, char *argv[], CaptureOptions *options)
{
    bool count_seen = false;
    bool duration_seen = false;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc || argv[i + 1][0] == '\0') {
            return false;
        }
        const char *name = argv[i];
        const char *value = argv[i + 1];
        if (strcmp(name, "--interface") == 0 && options->interface_name == NULL && value[0] != '-') {
            options->interface_name = value;
        } else if (strcmp(name, "--count") == 0 && !count_seen) {
            count_seen = true;
            if (!parse_positive(value, 1000000, &options->packet_limit)) {
                return false;
            }
        } else if (strcmp(name, "--duration") == 0 && !duration_seen) {
            duration_seen = true;
            if (!parse_positive(value, 86400, &options->duration_seconds)) {
                return false;
            }
        } else if (strcmp(name, "--filter") == 0 && options->filter == NULL && strlen(value) <= 4096) {
            options->filter = value;
        } else {
            return false;
        }
    }
    return options->interface_name != NULL;
}

int main(int argc, char *argv[])
{
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
        print_usage(stdout);
        return EXIT_SUCCESS;
    }

    if (argc == 2 && strcmp(argv[1], "--list") == 0) {
        return capture_list_interfaces();
    }
    CaptureOptions options = {NULL, NULL, 100, 30};
    if (!parse_options(argc, argv, &options)) {
        fputs("오류: 인자·범위·중복 옵션을 확인하세요.\n", stderr);
        print_usage(stderr);
        return EXIT_FAILURE;
    }
    return capture_run(&options);
}
