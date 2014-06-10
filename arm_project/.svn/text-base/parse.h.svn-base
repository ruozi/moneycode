#ifndef __PARSE_H__
#define __PARSE_H__
#include <stdint.h>

// buffer status code
#define STATUS_LENGTH_NOT_READ  0x10
#define STATUS_LENGTH_READ      0x20
#define STATUS_FINISHED         0x40
#define STATUS_LRC				0x80

// operaton code 
#define OPERATION_TEST_REQ 0x01
#define OPERATION_TEST_RES 0x02
#define OPERATION_SCAN_REQ 0x03
#define OPERATION_SCAN_RES 0x04

// buffer status
#define PACKET_LEN_POS 1
#define PACKET_LEN_LEN 2
#define PACKET_LEN_TOTAL 5

typedef struct ReadBuffer {
    uint16_t len;
    uint16_t left;
    uint8_t status;
	uint8_t number;
    uint8_t buf[64];
} ReadBuffer;

int parse(ReadBuffer*);

void init_buffer(ReadBuffer*);


#endif
