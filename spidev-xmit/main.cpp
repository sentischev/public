
#include "main.h"

static void print_usage() {
    printf("Usage: [-hdmsbpco]\n");
    printf("  -h        print help info\n");
    printf("  -d        e.g. /dev/spidev1.0\n");
    printf("  -m        SPI mode: 0, 1, 2 or 3\n");
    printf("  -s        frequency, hz\n");
    printf("  -b        transmit/receive buffer size\n");
    printf("  -p        print receive buffer?\n");
    printf("  -c        print receive buffer bytes count\n");
    printf("  -o        output buffer file\n");
    printf("Default:\n");
    printf("  -d /dev/spidev1.0\n");
    printf("  -m 2\n");
    printf("  -s 1000000\n");
    printf("  -b 1600\n");
    printf("  -p 1\n");
    printf("  -c 1600\n");
    printf("  -o /dev/null\n");
}

int main(int argc, char *argv[]) {
    print_usage();

    return 0;
}
