#include "capture.h"
#include "output.h"

#include <pcap.h>
#include <windows.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static volatile LONG stop_requested;

static BOOL WINAPI handle_console_event(DWORD event)
{
    if (event == CTRL_C_EVENT || event == CTRL_BREAK_EVENT) {
        /* 핸들 해제는 캡처 루프에서만 수행한다. */
        InterlockedExchange(&stop_requested, 1);
        return TRUE;
    }
    return FALSE;
}

static int initialize_capture(void)
{
    char error[PCAP_ERRBUF_SIZE] = {0};
    if (pcap_init(PCAP_CHAR_ENC_UTF_8, error) != 0) {
        output_printf(stderr, "오류: Npcap 초기화 실패: %s\n", error);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int capture_list_interfaces(void)
{
    pcap_if_t *devices = NULL;
    char error[PCAP_ERRBUF_SIZE] = {0};
    if (initialize_capture() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (pcap_findalldevs(&devices, error) != 0) {
        output_printf(stderr, "오류: 인터페이스 목록 조회 실패: %s\n", error);
        return EXIT_FAILURE;
    }
    output_printf(stdout, "인터페이스 목록 (--interface에 이름을 그대로 지정):\n");
    for (const pcap_if_t *device = devices; device != NULL; device = device->next) {
        output_printf(stdout, "%s\n  %s\n", device->name,
               device->description != NULL ? device->description : "설명 없음");
    }
    if (devices == NULL) {
        output_printf(stdout, "사용 가능한 인터페이스가 없습니다.\n");
    }
    pcap_freealldevs(devices);
    return EXIT_SUCCESS;
}

static int check_interface(const char *name)
{
    pcap_if_t *devices = NULL;
    char error[PCAP_ERRBUF_SIZE] = {0};
    int found = 0;
    if (pcap_findalldevs(&devices, error) != 0) {
        output_printf(stderr, "오류: 인터페이스 목록 조회 실패: %s\n", error);
        return EXIT_FAILURE;
    }
    for (const pcap_if_t *device = devices; device != NULL; device = device->next) {
        if (strcmp(name, device->name) == 0) {
            found = 1;
            break;
        }
    }
    pcap_freealldevs(devices);
    if (!found) {
        output_printf(stderr, "오류: 지정한 인터페이스가 없습니다. --list로 이름을 확인하세요.\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int apply_filter(pcap_t *handle, const char *expression)
{
    struct bpf_program program;
    if (expression == NULL) {
        return EXIT_SUCCESS;
    }
    if (pcap_compile(handle, &program, expression, 1, PCAP_NETMASK_UNKNOWN) != 0) {
        output_printf(stderr, "오류: 캡처 필터 문법 오류: %s\n", pcap_geterr(handle));
        return EXIT_FAILURE;
    }
    int status = pcap_setfilter(handle, &program);
    pcap_freecode(&program);
    if (status != 0) {
        output_printf(stderr, "오류: 캡처 필터 적용 실패: %s\n", pcap_geterr(handle));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int format_capture_time(const struct timeval *timestamp, char *buffer, size_t capacity)
{
    /* pcap_open_live의 기본 단위는 초 + 마이크로초다. 출력 시각으로 대체하지 않는다. */
    if (timestamp->tv_sec < 0 || timestamp->tv_usec < 0 || timestamp->tv_usec >= 1000000) {
        return EXIT_FAILURE;
    }
    time_t seconds = (time_t)timestamp->tv_sec;
    const struct tm *local = localtime(&seconds);
    char date[32];
    if (local == NULL || strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", local) == 0) {
        return EXIT_FAILURE;
    }
    int length = snprintf(buffer, capacity, "%s.%06ld", date, (long)timestamp->tv_usec);
    return length >= 0 && (size_t)length < capacity ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int capture_loop(pcap_t *handle, const CaptureOptions *options)
{
    ULONGLONG started = GetTickCount64();
    uint32_t count = 0;
    int result = EXIT_SUCCESS;
    const char *reason = "패킷 수 제한";
    for (;;) {
        if (InterlockedCompareExchange(&stop_requested, 0, 0) != 0) {
            reason = "종료 요청";
            break;
        }
        if (GetTickCount64() - started >= (ULONGLONG)options->duration_seconds * 1000) {
            reason = "시간 제한";
            break;
        }
        struct pcap_pkthdr *header = NULL;
        const u_char *bytes = NULL;
        int status = pcap_next_ex(handle, &header, &bytes);
        if (status == 0) {
            Sleep(10);
            continue;
        }
        if (status != 1) {
            output_printf(stderr, "오류: 실시간 캡처 읽기 실패 (%d): %s\n", status, pcap_geterr(handle));
            reason = "캡처 오류";
            result = EXIT_FAILURE;
            break;
        }
        if (header == NULL || bytes == NULL || header->caplen > header->len) {
            output_printf(stderr, "오류: 잘못된 캡처 메타데이터입니다.\n");
            reason = "캡처 오류";
            result = EXIT_FAILURE;
            break;
        }
        char captured_at[40];
        if (format_capture_time(&header->ts, captured_at, sizeof(captured_at)) != EXIT_SUCCESS) {
            output_printf(stderr, "오류: 잘못된 캡처 시각입니다.\n");
            reason = "캡처 오류";
            result = EXIT_FAILURE;
            break;
        }
        /* Npcap 소유 버퍼를 복사하거나 원본 내용을 출력하지 않는다. */
        ++count;
        output_printf(stdout, "패킷 #%" PRIu32 " 캡처 길이=%" PRIu32 " 원래 길이=%" PRIu32
                      " 캡처 시각=%s\n",
               count, (uint32_t)header->caplen, (uint32_t)header->len, captured_at);
        if (count >= options->packet_limit) {
            break;
        }
    }
    output_printf(stdout, "종료: %s, 패킷=%" PRIu32 "\n", reason, count);
    return result;
}

int capture_run(const CaptureOptions *options)
{
    if (options == NULL || options->interface_name == NULL ||
        options->packet_limit == 0 || options->duration_seconds == 0) {
        output_printf(stderr, "오류: 캡처 설정이 올바르지 않습니다.\n");
        return EXIT_FAILURE;
    }
    if (initialize_capture() != EXIT_SUCCESS ||
        check_interface(options->interface_name) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    InterlockedExchange(&stop_requested, 0);
    /* 부모 프로세스에서 상속한 Ctrl+C 무시 설정도 해제한다. */
    if (!SetConsoleCtrlHandler(NULL, FALSE) ||
        !SetConsoleCtrlHandler(handle_console_event, TRUE)) {
        output_printf(stderr, "오류: 콘솔 종료 처리기를 등록하지 못했습니다.\n");
        return EXIT_FAILURE;
    }
    char error[PCAP_ERRBUF_SIZE] = {0};
    int result = EXIT_FAILURE;
    pcap_t *handle = pcap_open_live(options->interface_name, 65535, 0, 100, error);
    if (handle == NULL) {
        output_printf(stderr, "오류: 인터페이스 열기 실패. 장치·Npcap 서비스·접근 권한을 확인하세요: %s\n", error);
        goto cleanup;
    }
    if (error[0] != '\0') {
        output_printf(stderr, "경고: %s\n", error);
    }
    int link_type = pcap_datalink(handle);
    if (link_type < 0) {
        output_printf(stderr, "오류: 링크 타입 조회 실패: %s\n", pcap_geterr(handle));
        goto cleanup;
    }
    if (apply_filter(handle, options->filter) != EXIT_SUCCESS) {
        goto cleanup;
    }
    /* 읽기 timeout만으로는 무트래픽 종료를 보장할 수 없어 비차단 모드를 쓴다. */
    if (pcap_setnonblock(handle, 1, error) != 0) {
        output_printf(stderr, "오류: 비차단 캡처 설정 실패: %s\n", error);
        goto cleanup;
    }
    const char *link_name = pcap_datalink_val_to_name(link_type);
    output_printf(stdout, "캡처 시작: %s\n", options->interface_name);
    output_printf(stdout, "링크 타입: %d (%s)\n", link_type, link_name != NULL ? link_name : "알 수 없음");
    fflush(stdout);
    result = capture_loop(handle, options);

cleanup:
    if (handle != NULL) {
        pcap_close(handle);
    }
    SetConsoleCtrlHandler(handle_console_event, FALSE);
    output_printf(stdout, "캡처 자원 정리 완료\n");
    return result;
}
