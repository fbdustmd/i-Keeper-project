#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(FILE *stream)
{
    fputs("NetSentry - learning-oriented network analysis\n"
          "Usage: netsentry [--help]\n"
          "Phase 0: project skeleton. Packet capture is not implemented yet.\n",
          stream);
}

int main(int argc, char *argv[])
{
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
        print_usage(stdout);
        return EXIT_SUCCESS;
    }

    fputs("Error: unsupported arguments.\n", stderr);
    print_usage(stderr);
    return EXIT_FAILURE;
}
