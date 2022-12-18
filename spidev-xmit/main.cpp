
#include "main.h"

static void print_usage() {
    printf("Usage: [-hdmswbpcko]\n");
    printf("  -h        print help info\n");
    printf("  -d        e.g. /dev/spidev1.0\n");
    printf("  -m        SPI mode: 0, 1, 2 or 3\n");
    printf("  -s        frequency, hz\n");
    printf("  -w        bits per word\n");
    printf("  -b        transmit/receive buffer size\n");
    printf("  -p        print receive buffer?\n");
    printf("  -c        print receive buffer bytes count\n");
    printf("  -k        CS change flag\n");
    printf("  -o        output buffer file\n");
    printf("Default:\n");
    printf("  -d /dev/spidev1.0\n");
    printf("  -m 0\n");
    printf("  -s 1000000\n");
    printf("  -w 8\n");
    printf("  -b 16\n");
    printf("  -p 1\n");
    printf("  -c 16\n");
    printf("  -k 0\n");
    printf("  -o /dev/null\n");
}

char devname[64] = {0};
uint8_t mode = SPI_MODE_0;
uint32_t speed = 1000000;
uint8_t bits_per_word = 8;
unsigned buf_sz = 16;
int print_rx = 1;
int print_count = 16;
uint8_t cs_change = 0;

char txfilename[64] = {0};
char *tx_buf = NULL;
char *rx_buf = NULL;
FILE *tx_file = NULL;
struct spi_ioc_transfer xfer = {0};

int main(int argc, char *argv[]) {
    strcpy(devname, "/dev/spidev1.0");
    int opt = 0;
    int ret = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp("-d", argv[i])) {
            i++;
            sscanf(argv[i], "%s", devname);
        } else if (!strcmp("-m", argv[i])) {
            i++;
            sscanf(argv[i], "%d", &opt);
            switch (opt) {
            case 1:
                mode = SPI_MODE_1;
                break;
            case 2:
                mode = SPI_MODE_2;
                break;
            case 3:
                mode = SPI_MODE_3;
                break;
            default:
                mode = SPI_MODE_0;
            }
        } else if (!strcmp("-s", argv[i])) {
            i++;
            sscanf(argv[i], "%u", &speed);
        } else if (!strcmp("-w", argv[i])) {
            i++;
            sscanf(argv[i], "%hhu", &bits_per_word);
        } else if (!strcmp("-b", argv[i])) {
            i++;
            sscanf(argv[i], "%u", &buf_sz);
        } else if (!strcmp("-p", argv[i])) {
            i++;
            sscanf(argv[i], "%d", &print_rx);
        } else if (!strcmp("-c", argv[i])) {
            i++;
            sscanf(argv[i], "%d", &print_count);
        } else if (!strcmp("-k", argv[i])) {
            i++;
            sscanf(argv[i], "%hhu", &cs_change);
        } else if (!strcmp("-o", argv[i])) {
            i++;
            sscanf(argv[i], "%s", txfilename);
        } else {
            print_usage();
        }
    }

    int fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("Open %s: FAIL\n", devname);
        return -1;
    } else {
        printf("Open %s: OK\n", devname);
    }
    
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
    
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    rx_buf = new char[buf_sz];
    tx_buf = new char[buf_sz];

    memset(rx_buf, 0, buf_sz);
    memset(tx_buf, 0, buf_sz);

    xfer.rx_buf = (unsigned long long)rx_buf;
    xfer.tx_buf = (unsigned long long)tx_buf;
    xfer.len = buf_sz;
    xfer.delay_usecs =0;
    xfer.speed_hz = 0;
    xfer.cs_change = cs_change;
    xfer.bits_per_word = bits_per_word;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);

    if (print_rx) {
        printf("First %d received bytes are:\n", print_count);
        printf("   ");
        for (int i = 0; i < print_count; i++) {
            printf(" %02X", rx_buf[i]);
        }
        printf("\n");
    }

    delete []tx_buf;
    delete []rx_buf;

    close(fd);

    return 0;
}
