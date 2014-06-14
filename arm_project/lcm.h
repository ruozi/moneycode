#ifndef __LCM_H__
#define __LCM_H__

int setstat(int fd,int value,int no);

int write_data(int fd, unsigned char data);

int write_com(int fd, unsigned char data);

void InitLcd(int fd);

void InitLcd_Flash(int fd);

int print_ready(int fd);

int print_next(int fd);

int print_scan(int fd,int num);

int print_complete(int fd);

int print_recog(int fd);

int print_error(int fd);

int print_nosdcard(int fd);

int print_dup(int fd);

int print_noudisk(int fd);

int print_copy(int fd);
#endif
