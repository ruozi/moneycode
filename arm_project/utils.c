#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "utils.h"

int trim_space(char * str)
{
	char *trim = (char *)malloc(15*sizeof(char));
	int i=0;
	int len=0;

	for(i=0;str[i]!='\0';i++)
	{
		if(str[i]!=' ')
			trim[len++]=str[i];
	}
	trim[len]='\0';
	strcpy(str,trim);
	free(trim);
}

int smart_proc(char * ocr_output)
{
	//cut to 10
	int i=0;
	int len=0;
	int alpha_count = 0;
	int res=PROC_OK;

	trim_space(ocr_output);

	len = strlen(ocr_output);

	if(len < 10){
		res = PROC_NOK;
		for(i=len;i<10;i++)
			ocr_output[i]='0';
	}else if(len>10)
		len=10;

	//Process first element
	if(ocr_output[0] == '0')
		ocr_output[0] = 'O';
	if(ocr_output[0] == '6')
		ocr_output[0] = 'B';
	if(ocr_output[0] == '8')
		ocr_output[0] = 'B';

	for(i=4;i<len;i++){
		if('I' == ocr_output[i])
			ocr_output[i] = '1';
		if('B' == ocr_output[i])
			ocr_output[i] = '8';
	}

	for(i=1;i<4;i++){
		if(ocr_output[i] <= 'Z' && ocr_output[i] >= 'A'){
			alpha_count++;
			break;
		}
	}

#ifdef __DEBUG_P__
	printf("Alpha count is %d\n",alpha_count);
#endif

	if(!alpha_count){
		for(i=1;i<4;i++)
			if('8' == ocr_output[i]){
				ocr_output[i]='B';
				alpha_count++;
#ifdef __DEBUG_P__
				printf("Modify 8 to B in position %d!\n",i+1);
#endif
				break;
			}
	}

	if(!alpha_count){
		for(i=1;i<4;i++)
			if('0' == ocr_output[i]){
				ocr_output[i]='O';
				alpha_count++;
#ifdef __DEBUG_P__
				printf("Modify 0 to O in position %d!\n",i+1);
#endif
				break;
			}
	}
	return res;
}

void decorate_buffer(char * input,char* output,int n)
{
	unsigned char SCAN_RES_HEAD[15]={0x02,0x00,0x1A,0xA1,0x04,0x35,0x35,0x1C,0x30,0x30,0x1C,0x00,0x15,0x01,0x23};
	unsigned char SCAN_RES_TAIL[2]={0x1C,0x03};

	SCAN_RES_HEAD[2]=0x0D+n*13;
	SCAN_RES_HEAD[12]=(13*n+2)/10*16+(13*n+2)%10;
	SCAN_RES_HEAD[13]=n;
#ifdef __DEBUG__
	printf("SCAN_RES_HEAD[2]:%.2X,SCAN_RES_HEAD[13]:%.2X\n",SCAN_RES_HEAD[2],SCAN_RES_HEAD[13]);
#endif

	memcpy(output,SCAN_RES_HEAD,15);
	memcpy(output+15,input,13*n);
	memcpy(output+13*n+15,SCAN_RES_TAIL,2);

	int i=0;
	unsigned char lrc=0x00;
	for(i=1;i<(13*n+17);i++)
		lrc^=output[i];
#ifdef __DEBUG__
	printf("lrc=%.2X\n",lrc);
#endif

	memcpy(output+13*n+17,&lrc,1);
}

int d_copy(const char* srcpath,const char* dstpath)
{
	struct dirent *sp,*tp;
	char spath[100],tpath[100],temp_spath[100],temp_tpath[100],command[100];
	struct stat sbuf,tbuf,temp_sbuf,temp_tbuf;
	char judge;
	DIR *dir_s,*dir_t;

	dir_s=opendir(srcpath);
	if(dir_s==NULL)
	{
		return 1;
	}
	if(stat(srcpath,&sbuf))
	{
		return 2;
	}
	dir_t=opendir(dstpath);
	if(dir_t==NULL)
	{
		sprintf(command,"mkdir %s",dstpath);
		system(command);
		system("sync");
		dir_t=opendir(dstpath);
	}
	else
	{
		chmod(dstpath,sbuf.st_mode);
		chown(dstpath,sbuf.st_uid,sbuf.st_gid);
	}
	strcpy(temp_spath,srcpath);
	strcpy(temp_tpath,dstpath);
	
#ifdef __DEBUG_P__
	printf("Begin copy ....../n");
#endif

	while((sp=readdir(dir_s))!=NULL)
	{
		if(strcmp(sp->d_name,".")!=0 && strcmp(sp->d_name,"..")!=0)
		{
#ifdef __DEBUG_P__
			printf("Processing file:%s\n",sp->d_name);
#endif

			strcat(temp_spath,"/");
			strcat(temp_spath,sp->d_name);
			strcat(temp_tpath,"/");
			strcat(temp_tpath,sp->d_name);
			lstat(temp_spath,&sbuf);
			temp_sbuf.st_mode=sbuf.st_mode;

			if((S_IFMT&temp_sbuf.st_mode)==S_IFREG)
			{
#ifdef __DEBUG_P__
				printf("Copy file %s...\n",temp_spath);
#endif

				sprintf(command,"mv %s %s",temp_spath,temp_tpath);
				system(command);
				system("sync");
			}
			strcpy(temp_tpath,dstpath);
			strcpy(temp_spath,srcpath);
		}
	}

#ifdef __DEBUG_P__
	printf("Copy end!\n");
#endif
	closedir(dir_t);
	closedir(dir_s);
	return 0;
}







