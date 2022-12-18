
#ifndef _MAIN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#ifdef __c_plusplus

typedef struct {
    uint8_t if_type:4;
    uint8_t if_num:4;
    uint8_t flags;
    uint16_t len;
    uint16_t offset;
    uint16_t checksum;
    uint16_t seq_num;
    uint8_t reserved2;
    union {
        uint8_t reserved3;
        uint8_t hci_pkt_type;
        uint8_t priv_pkt_type;
    };
} esp_payload_header, *pesp_payload_header;

#endif

#endif
