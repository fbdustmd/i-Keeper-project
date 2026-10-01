#ifndef NETSENTRY_CAPTURE_H
#define NETSENTRY_CAPTURE_H

#include <stdint.h>

typedef struct {
    const char *interface_name;
    const char *filter;
    uint32_t packet_limit;
    uint32_t duration_seconds;
} CaptureOptions;

int capture_list_interfaces(void);
int capture_run(const CaptureOptions *options);

#endif
