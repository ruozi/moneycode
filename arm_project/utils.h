#ifndef __UTILS_H__
#define __UTILS_H__

#define PROC_OK 0
#define PROC_NOK 1

int trim_space(char * str);

int smart_proc(char * ocr_output);

void decorate_buffer(char *input,char *output, int n);

int d_copy(const char *srcpath,const char *dstpath);

#endif
