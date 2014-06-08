#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include <stdio.h>

void init_buffer(ReadBuffer * buf)
{
	memset(buf,0,sizeof(ReadBuffer));
	buf->status=STATUS_LENGTH_NOT_READ;
	buf->len = 0;
	buf->left = 0;
	buf->number = 0;
}

int parse( ReadBuffer * buf){
    if( NULL == buf) 
        return -2;
    int ret = 0;
    if( STATUS_LENGTH_NOT_READ == buf->status){
        if(buf->len >= PACKET_LEN_POS + PACKET_LEN_LEN){
            uint16_t packet_len = ((uint16_t)buf->buf[PACKET_LEN_POS] << 8) | (uint16_t)buf->buf[PACKET_LEN_POS+1] + PACKET_LEN_TOTAL;
            if(buf->len < packet_len){
                buf->left = packet_len - buf->len; 
                buf->status = STATUS_LENGTH_READ;
            }
            else if(buf->len == packet_len - 1){
                buf->left = 1;
                buf->status = STATUS_LRC;
            }
			else if(buf->len == packet_len){
				buf->left = 0;
				buf->status = STATUS_FINISHED;
			}
            else
                ret = -1;
        }
    } else if ( STATUS_LENGTH_READ == buf->status) {
		uint16_t packet_len = ((uint16_t)buf->buf[PACKET_LEN_POS] << 8) | (uint16_t)buf->buf[PACKET_LEN_POS+1] + PACKET_LEN_TOTAL;
        buf->left =  packet_len - buf->len;
        if(1 == buf->left){
            buf->status = STATUS_LRC;
        }
		else if(0 == buf->left){
			buf->status = STATUS_FINISHED;
		}else if(0 > buf->left){
            ret = -1;
        }
        else;
    } else if( STATUS_LRC == buf->status) {
		uint16_t packet_len = ((uint16_t)buf->buf[PACKET_LEN_POS] << 8) | (uint16_t)buf->buf[PACKET_LEN_POS+1] + PACKET_LEN_TOTAL;
		uint8_t lrc = 0x00;
		uint16_t len; 
#ifdef __DEBUG__
		printf("%d\n",packet_len);
#endif
		for( len = packet_len-1; --len > 0; lrc ^= buf->buf[len] ){
#ifdef __DEBUG__
			printf("len = %d , lrc = %.2X\n",len,lrc);
#endif
		}
		buf->buf[packet_len-1] = lrc;
		buf->len++;
		buf->left--;
		buf->status = STATUS_FINISHED;
	}else if( STATUS_FINISHED == buf->status){
		int op_code = buf->buf[4];
		switch(op_code)
		{
			case OPERATION_TEST_REQ:
			case OPERATION_TEST_RES:
				ret = op_code;
				break;
			case OPERATION_SCAN_REQ:
				ret = op_code;
				buf->number = 10*(buf->buf[14]-'0')+(buf->buf[15]-'0');
				break;
			case OPERATION_SCAN_RES:
				ret = op_code;
				break;
			default:
				ret = -1;
				break;
		}
	}else
		ret = -1;
	return ret;
}

#ifdef __DEBUG__
int main()
{
	ReadBuffer *buffer = (ReadBuffer *)malloc(sizeof(ReadBuffer));
	buffer->len = 33;
	buffer->left = 1;
	buffer->status = STATUS_LENGTH_READ;
	buffer->buf[0] = 0x02;
	buffer->buf[1] = 0x00;
	buffer->buf[2] = 0x1D;
	buffer->buf[3] = 0xA1;
	buffer->buf[4] = 0x03;
	buffer->buf[5] = 0x35;
	buffer->buf[6] = 0x35;
	buffer->buf[7] = 0x1C;
	buffer->buf[8] = 0x30;
	buffer->buf[9] = 0x30;
	buffer->buf[10] = 0x30;
	buffer->buf[11] = 0x30;
	buffer->buf[12] = 0x30;
	buffer->buf[13] = 0x30;
	buffer->buf[14] = 0x30;
	buffer->buf[15] = 0x35;
	buffer->buf[16] = 0x30;
	buffer->buf[17] = 0x30;
	buffer->buf[18] = 0x30;
	buffer->buf[19] = 0x30;
	buffer->buf[20] = 0x1C;
	buffer->buf[21] = 0x32;
	buffer->buf[22] = 0x30;
	buffer->buf[23] = 0x31;
	buffer->buf[24] = 0x34;
	buffer->buf[25] = 0x2D;
	buffer->buf[26] = 0x30;
	buffer->buf[27] = 0x33;
	buffer->buf[28] = 0x2D;
	buffer->buf[29] = 0x31;
	buffer->buf[30] = 0x35;
	buffer->buf[31] = 0x1C;
	buffer->buf[32] = 0x03;

	while(!parse(buffer));

	int i=0;
	for(i=0;i<buffer->len;i++)
		fprintf(stdout,"%.2X  ",buffer->buf[i]);

	printf("\n length=%d",buffer->len);

	free(buffer);
	return 0;
}
#endif

