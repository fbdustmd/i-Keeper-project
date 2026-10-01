#include "capture.h"
#include "output.h"
#include <pcap.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* 실제 드라이버 대신 실패를 주입한다. 장치를 열거나 트래픽을 만들지 않는다. */
enum Scenario { NORMAL, INIT_ERROR, LIST_ERROR, OPEN_ERROR, LINK_ERROR,
    COMPILE_ERROR, FILTER_ERROR, NONBLOCK_ERROR, READ_ERROR, READ_END,
    BAD_LENGTH, TRUNCATED, IDLE };
static enum Scenario scenario;
static int opened, closed, freed_list, freed_code;
static int fake_handle;
static char device_name[] = "test-interface";
static pcap_if_t device = { .name = device_name };
static unsigned char data[64];
static struct pcap_pkthdr packet;

int pcap_init(unsigned int encoding, char *error)
{
    assert(encoding == PCAP_CHAR_ENC_UTF_8);
    strcpy(error, "모의 초기화 오류");
    return scenario == INIT_ERROR ? -1 : 0;
}
int pcap_findalldevs(pcap_if_t **devices, char *error)
{
    strcpy(error, "모의 목록 오류");
    *devices = scenario == LIST_ERROR ? NULL : &device;
    return scenario == LIST_ERROR ? -1 : 0;
}
void pcap_freealldevs(pcap_if_t *devices) { assert(devices == &device); ++freed_list; }
pcap_t *pcap_open_live(const char *name, int snaplen, int promisc, int timeout, char *error)
{
    assert(strcmp(name, device_name) == 0 && snaplen == 65535 && promisc == 0 && timeout > 0);
    if (scenario == OPEN_ERROR) { strcpy(error, "모의 권한 오류"); return NULL; }
    ++opened;
    return (pcap_t *)&fake_handle;
}
void pcap_close(pcap_t *handle) { assert(handle == (pcap_t *)&fake_handle); ++closed; }
int pcap_datalink(pcap_t *handle) { (void)handle; return scenario == LINK_ERROR ? -1 : 1; }
const char *pcap_datalink_val_to_name(int value) { (void)value; return "EN10MB"; }
char *pcap_geterr(pcap_t *handle) { (void)handle; return "모의 캡처 오류"; }
int pcap_compile(pcap_t *handle, struct bpf_program *program, const char *expression,
                 int optimize, bpf_u_int32 mask)
{
    (void)handle; (void)program; (void)expression; (void)optimize; (void)mask;
    return scenario == COMPILE_ERROR ? -1 : 0;
}
int pcap_setfilter(pcap_t *handle, struct bpf_program *program)
{ (void)handle; (void)program; return scenario == FILTER_ERROR ? -1 : 0; }
void pcap_freecode(struct bpf_program *program) { (void)program; ++freed_code; }
int pcap_setnonblock(pcap_t *handle, int nonblock, char *error)
{
    (void)handle; assert(nonblock == 1); strcpy(error, "모의 비차단 오류");
    return scenario == NONBLOCK_ERROR ? -1 : 0;
}
int pcap_next_ex(pcap_t *handle, struct pcap_pkthdr **header, const u_char **bytes)
{
    (void)handle;
    if (scenario == READ_ERROR) { return -1; }
    if (scenario == READ_END) { return -2; }
    if (scenario == IDLE) { return 0; }
    packet.caplen = scenario == BAD_LENGTH ? 65 : 32;
    packet.len = scenario == TRUNCATED ? 64 : 32;
    *header = &packet;
    *bytes = data;
    return 1;
}
int main(void)
{
    CaptureOptions options = {device_name, "ip", 2, 1};
    assert(capture_run(NULL) == 1);
    for (scenario = NORMAL; scenario <= IDLE; ++scenario) {
        opened = closed = freed_list = freed_code = 0;
        int expected = scenario == NORMAL || scenario == TRUNCATED || scenario == IDLE ? 0 : 1;
        assert(capture_run(&options) == expected);
        assert(opened == closed);
        assert(freed_list == (scenario == INIT_ERROR || scenario == LIST_ERROR ? 0 : 1));
        assert(freed_code == (scenario == NORMAL || scenario >= FILTER_ERROR ? 1 : 0));
    }
    scenario = NORMAL;
    options.interface_name = "missing";
    opened = closed = 0;
    assert(capture_run(&options) == 1 && opened == 0 && closed == 0);
    output_printf(stdout, "캡처 모의 테스트 통과: 13개 시나리오, 잘린 길이, 실패 경로 자원 해제\n");
    return 0;
}
