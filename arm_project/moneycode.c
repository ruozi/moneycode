#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include "moneycode.h"
#include <pthread.h>

static void *thread_beep_twice(void *Fd)
{
	beep(*(int*)Fd);
	usleep(500000);
	beep(*(int*)Fd);

	return ((void *)0);
}

static void *thread_print_recog(void *Fd)
{
	print_recog(*(int*)Fd);

	return ((void *)0);
}

static int set_img_name(bool istest,bool isprocessed,char *name)
{
	time_t now;
	struct tm *timenow;

	time(&now);
	timenow = localtime(&now);

	if(istest)
	{
		if(isprocessed)
		{
			memset(name,0,64);
			sprintf(name,"/sdcard/test/%d-%d-%d-%d-%d-%d_test_proc.jpg",timenow->tm_year+1900,timenow->tm_mon,timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
		}
		else
		{
			memset(name,0,64);
			sprintf(name,"/sdcard/test/%d-%d-%d-%d-%d-%d_test.jpg",timenow->tm_year+1900,timenow->tm_mon,timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
		}
	}else
	{
		if(isprocessed)
		{
			memset(name,0,64);
			sprintf(name,"/sdcard/recog/%d-%d-%d-%d-%d-%d_proc.jpg",timenow->tm_year+1900,timenow->tm_mon,timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
		}
		else
		{
			memset(name,0,64);
			sprintf(name,"/sdcard/recog/%d-%d-%d-%d-%d-%d.jpg",timenow->tm_year+1900,timenow->tm_mon,timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
		}
	}
		
	return 0;
}
static void Error(const char *Msg)
{
	fprintf(stderr, "%s\n", Msg);
	fprintf(stderr, "stderror() is %s\n",strerror(errno));
	exit(1);
}

static int search(const unsigned char* set,unsigned char keyword,int * index)
{
	int i=0;
	for(i=0;i<36;i++)
		if(set[i] == keyword){
			*index = i;
			break;
		}
	if(36==i)
		return 1;

	return 0;
}

static inline void WaitFdWriteable(int Fd)
{
	fd_set WriteSetFD;
	FD_ZERO(&WriteSetFD);
	FD_SET(Fd,&WriteSetFD);
	if(select(Fd+1,NULL, &WriteSetFD,NULL,NULL)<0)
		Error(strerror(errno));
}

int main(int argc, char **argv)
{
	const unsigned char TEST_RES[15]={0x02,0x00,0x0A,0xA1,0x02,0x39,0x39,0x1C,0x30,0x30,0x1C,0x00,0x1C,0x03,0xB6};
	const unsigned char SEPARATOR[3]={0x7C,0xB1,0x23};
	const unsigned char INIT[10] = {'B','4','8','E','9','5','6','5','4','2'};
	const unsigned char CODESET[36] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};


	char *imgname = (char *)malloc(64*sizeof(char));
	char *imgprocname = (char *)malloc(64*sizeof(char));

	int CommFd;
	int ButtonsFd;
	int LcmFd;
	int PwmFd;
	int LedFd;

	int number = 0;
	int current = 0;
	int pos = 0;
	int index = 0;
	int status = STATUS_SERIAL_READY;
	int count_btn5 = 0;
	int count_btn6 = 0;

	ReadBuffer *buf = (ReadBuffer *)malloc(sizeof(ReadBuffer));
	char * output_buf;
	char * money_buf;
	char * ocr_output = (char *)malloc(15*sizeof(char));

	init_buffer(buf);
#ifdef __DEBUG_P__
	printf("status:%d\n",buf->status);
	if(buf->status==STATUS_LENGTH_NOT_READ)
		printf("status not read!\n");
	if(buf == NULL)
		printf("NULL!\n");
#endif
	struct termios TtyAttr;

	int DeviceSpeed = B38400;
	int ByteBits = CS8;
	const char *DeviceName = "/dev/ttySAC3";
	const char *ButtonName = "/dev/buttons";
	const char *LcmName	= "/dev/leds";

	LcmFd = open(LcmName,0);
	if(LcmFd<0)
		Error("Unable to open LCM");

	ButtonsFd = open(ButtonName,0);
	if (ButtonsFd < 0)
		Error("Unable to open device buttons");

	CommFd = open(DeviceName, O_RDWR, 0);
	if (CommFd < 0)
		Error("Unable to open device");
	if(fcntl(CommFd, F_SETFL, O_NONBLOCK) < 0)
		Error("Unable set to NONBLOCK mode");

	PwmFd = open("/dev/pwm",0);
	if (PwmFd < 0)
		Error("Unable to open beep");

	LedFd = open("/dev/leds",0);
	if(LedFd < 0)
		Error("Unable to open led");
	
	backlight_on(LedFd);
	illumination_on(LedFd);
	
	memset(&TtyAttr, 0, sizeof(struct termios));
	TtyAttr.c_iflag = IGNPAR;
	TtyAttr.c_cflag = DeviceSpeed | HUPCL | ByteBits | CREAD | CLOCAL;
	TtyAttr.c_cc[VMIN] = 1;

	if(tcsetattr(CommFd, TCSANOW, &TtyAttr) < 0)
		fprintf(stderr, "Warning: Unable to set comm port \n");

#ifdef __DEBUG_P__
	printf("Devices OK!\n");
#endif

	print_ready(LcmFd);

	while(1)
	{
		if(0!=access("/dev/sdcard",F_OK) && status!=STATUS_NO_SDCARD)
		{
			status = STATUS_NO_SDCARD;
			print_nosdcard(LcmFd);
		}

		if(-1!=access("/dev/sdcard",F_OK) && status == STATUS_NO_SDCARD)
		{
			status = STATUS_SERIAL_READY;
			print_ready(LcmFd);
		}

#ifdef __DEBUG_P__
		switch(status){
			case STATUS_SERIAL_READY:
				printf("Serial ready state!\n");
				break;
			case STATUS_SCAN_READY:
				printf("Scan ready state!\n");
				break;
			case STATUS_DISPLAY:
				printf("Display state!\n");
				break;
			case STATUS_MODIFICATION:
				printf("Modification state!\n");
				break;
			default:
				printf("Status Error!\n");
				exit(1);
		}
#endif
		fd_set ReadSetFD;

		FD_ZERO(&ReadSetFD);
		FD_SET(CommFd, &ReadSetFD);
		FD_SET(ButtonsFd,&ReadSetFD);
#define max(x,y) (((x)>=(y))? (x) : (y))
		if(select(max(CommFd,ButtonsFd)+1, &ReadSetFD, NULL, NULL, NULL) < 0)
		{
			Error(strerror(errno));
		}
#undef max

#ifdef __DEBUG_P__
		printf("Selected!\n");
#endif
		if(FD_ISSET(CommFd, &ReadSetFD))
		{
#ifdef __DEBUG_P__
			printf("Datas come from SerialPort!\n");
#endif
			int nread=read(CommFd,buf->buf+buf->len,16);
			buf->len+=nread;
#ifdef __DEBUG_P__
			printf("%d bytes readed\n",buf->len);
#endif
			if(0x02 != buf->buf[0] && buf->len >0){
				init_buffer(buf);
			}
			parse(buf);
#ifdef __DEBUG_P__
			printf("nread:%d\n",nread);
			printf("len:%d\n",buf->len);
			printf("left:%d\n",buf->left);
			printf("status:%d\n",buf->status);
			int i=0;
			for(i=0;i<buf->len;i++)
				printf("%.2X ",buf->buf[i]);
			printf("\n");
#endif
			if(buf->status == STATUS_FINISHED){
				int ret=parse(buf);
#ifdef __DEBUG_P__
				printf("FINISH!\n");
				printf("ret=%d\n",ret);
#endif
				if(ret == OPERATION_TEST_REQ){
#ifdef __DEBUG_P__
					printf("OPERATION_TEST_REQ\n");
#endif
					WaitFdWriteable(CommFd);
					write(CommFd,TEST_RES,15);
					init_buffer(buf);
					//end TEST_REQ
				}else if(ret == OPERATION_SCAN_REQ && status==STATUS_SERIAL_READY && buf->buf[5]==0x35)
				{
#ifdef __DEBUG_P__
					printf("OPERATION_SCAN_REQ\n");
#endif
					number = buf->number;

					if(number < 1)
					{
						print_error(LcmFd);
						WaitFdWriteable(CommFd);
						write(CommFd,TEST_RES,15);
						sleep(2);
						print_ready(LcmFd);
					}
					else{
						output_buf = (char *)malloc((13*(number+1)+5)*sizeof(char));
						money_buf = (char *)malloc(13*number*sizeof(char));
#ifdef __DEBUG_P__
						printf("SCAN REQ!number of money:%d\n",number);
						printf("length of output_buf :%d\n",13*(number+1)+5);
#endif
						//status changed from 0 to 1
#ifdef __DEBUG_P__
						printf("Go to scan ready status!");
#endif
						status = STATUS_SCAN_READY;

						print_scan(LcmFd,number);
					}

					init_buffer(buf);
				}else
					init_buffer(buf);
			}
		}

		if(FD_ISSET(ButtonsFd,&ReadSetFD)){

			char current_buttons[6];
			if(read(ButtonsFd,current_buttons,sizeof(current_buttons))!=sizeof(current_buttons)){
				Error("Unable to read buttons!");
				exit(1);
			}

			//Judge if button down
			int i=0;
			for(i=0;i<6;i++)
			{
				if('0'!=current_buttons[i])
				{
					beep(PwmFd);
					break;
				}
			}


			switch(status)
			{
				case STATUS_SERIAL_READY:
					//Only button1(test) is enabled
#ifdef __DEBUG_P__
					printf("Case serial ready!\n");
#endif
					if('0'!=current_buttons[0]){
#ifdef __DEBUG_P__
						printf("Button 1 down!\n");

						clock_t start,end;
						start=clock();
#endif
						pthread_t id;
						int ret;

						ret=pthread_create(&id,NULL,&thread_print_recog,&LcmFd);
						if(ret!=0)
							Error("Create thread error!");

						//Take photo ,recognize it and display it
						set_img_name(PHOTO_TEST,PHOTO_ORIGIN,imgname);
						set_img_name(PHOTO_TEST,PHOTO_PROCESSED,imgprocname);

						int ret_camera = get_pic(imgname);
						if(ret_camera == EXIT_FAILURE)
							Error("Image is not correctly taken!\n");
#ifdef __DEBUG_P__
						printf("Image is taken!\n");
#endif
						img_process(imgname,imgprocname);
#ifdef __DEBUG_P__
						printf("Image processed!\n");
#endif
						recog(imgprocname,ocr_output);
#ifdef __DEBUG_P__
						printf("Text is recognized!\n");
#endif
						if(PROC_NOK==smart_proc(ocr_output))
						{
							ret=pthread_create(&id,NULL,&thread_beep_twice,&PwmFd);
							if(ret!=0)
								Error("Create thread error!");
						}

#ifdef __DEBUG_P__
						printf("Result is decorated\n");

						end = clock();

						printf("Time cost:%d ms\n",(int)(end-start));
#endif
						InitLcd(LcmFd);
#ifdef __DEBUG_P__
						printf("LCD Initialized\n");
#endif
						int i=0;
						for(i=0;i<10;i++)
							write_data(LcmFd,ocr_output[i]);
#ifdef __DEBUG_P__
						printf("LCD displayed!\n");
#endif
					}else if('0'!=current_buttons[4]){
						//Press button 5 three times to copy test images.
#ifdef __DEBUG_P__
						printf("Button 5 down!\n");
#endif
						count_btn5++;
						if(3==count_btn5)
						{
							count_btn5=0;

							if(0!=access("/dev/udisk",F_OK))
							{
								print_noudisk(LcmFd);
							}else{
								print_copy(LcmFd);

								d_copy("/sdcard/test","/udisk/test");

								print_complete(LcmFd);
							}

						}
					}else if('0'!=current_buttons[5]){
						//Press button 6 three times to copy recog images.
#ifdef __DEBUG_P__
						printf("Button 6 down!\n");
#endif
						count_btn6++;
						if(3==count_btn6)
						{
							count_btn6=0;

							if(0!=access("/dev/udisk",F_OK))
							{
								print_noudisk(LcmFd);
							}else{
								print_copy(LcmFd);

								d_copy("/sdcard/recog","/udisk/recog");

								print_complete(LcmFd);
							}

						}

					}
					break;

				case STATUS_SCAN_READY:
					//Only button2(recog) is enabled
#ifdef __DEBUG_P__
					printf("Case scan ready!\n");
#endif
					if('0'!=current_buttons[1]){
#ifdef __DEBUG_P__
						printf("Button 2 down!\n");
						printf("Go to Display state!\n");
#endif
						//Take photo ,recognize it and display it with progress
						current++;

						pthread_t id;
						int ret;

						ret=pthread_create(&id,NULL,&thread_print_recog,&LcmFd);
						if(ret!=0)
							Error("Create thread error!");

						//get name
						set_img_name(PHOTO_RECOG,PHOTO_ORIGIN,imgname);
						set_img_name(PHOTO_RECOG,PHOTO_PROCESSED,imgprocname);

						int ret_camera = get_pic(imgname);
						if(ret_camera == EXIT_FAILURE)
							Error("Image is not correctly taken!\n");
#ifdef __DEBUG_P__
						printf("Image is taken!\n");
#endif
						img_process(imgname,imgprocname);
#ifdef __DEBUG_P__
						printf("Image processed!\n");
#endif
						recog(imgprocname,ocr_output);
#ifdef __DEBUG_P__
						printf("Text is recognized!\n");
#endif
						if(PROC_NOK==smart_proc(ocr_output))
						{
							ret=pthread_create(&id,NULL,&thread_beep_twice,&PwmFd);
							if(ret!=0)
								Error("Create thread error!");
						}

#ifdef __DEBUG_P__
						printf("Result is decorated\n");
#endif
						InitLcd(LcmFd);
#ifdef __DEBUG_P__
						printf("LCD Initialized\n");
#endif
						//Judge if it is same
						int i=0;
						int j=0;
						bool dup = false;
						for(i=0;i<current-1;i++)
						{
							for(j=0;j<10;j++)
							{
								if(money_buf[13*i+j] != ocr_output[j])
									break;
							}
							if(j >= 10)
							{
								dup=true;
								break;
							}
						}

						if(dup)
						{
							//duplicate
							current--;
							print_dup(LcmFd);

							sleep(1);
							print_next(LcmFd);
						}
						else
						{
							for(i=0;i<10;i++)
								write_data(LcmFd,ocr_output[i]);
							write_data(LcmFd,' ');

							if(current/10)
								write_data(LcmFd,current/10+'0');
							write_data(LcmFd,current % 10 +'0');
							write_data(LcmFd,'/');
							if(number/10)
								write_data(LcmFd,number/10+'0');
							write_data(LcmFd,number % 10 + '0');
#ifdef __DEBUG_P__
							printf("LCD displayed!\n");
#endif

							status = STATUS_DISPLAY;
						}
					}
					break;

				case STATUS_DISPLAY:
					//Button3(confirm),button4(modify) are enabled
#ifdef __DEBUG_P__
					printf("Case display!\n");
#endif
					if('0'!=current_buttons[1]){
#ifdef __DEBUG_P__
						printf("Button 2 down!\n");
						printf("Recog again!\n");
#endif
						//Take photo ,recognize it and display it with progress

						pthread_t id;
						int ret;

						ret=pthread_create(&id,NULL,&thread_print_recog,&LcmFd);
						if(ret!=0)
							Error("Create thread error!");

						//get name
						set_img_name(PHOTO_RECOG,PHOTO_ORIGIN,imgname);
						set_img_name(PHOTO_RECOG,PHOTO_PROCESSED,imgprocname);

						int ret_camera = get_pic(imgname);
						if(ret_camera == EXIT_FAILURE)
							Error("Image is not correctly taken!\n");
#ifdef __DEBUG_P__
						printf("Image is taken!\n");
#endif
						img_process(imgname,imgprocname);
#ifdef __DEBUG_P__
						printf("Image processed!\n");
#endif
						recog(imgprocname,ocr_output);
#ifdef __DEBUG_P__
						printf("Text is recognized!\n");
#endif
						if(PROC_NOK==smart_proc(ocr_output))
						{
							ret=pthread_create(&id,NULL,&thread_beep_twice,&PwmFd);
							if(ret!=0)
								Error("Create thread error!");
						}

#ifdef __DEBUG_P__
						printf("Result is decorated\n");
#endif
						InitLcd(LcmFd);
#ifdef __DEBUG_P__
						printf("LCD Initialized\n");
#endif
						//Judge if it is same
						int i=0;
						int j=0;
						bool dup = false;
						for(i=0;i<current-1;i++)
						{
							for(j=0;j<10;j++)
							{
								if(money_buf[13*i+j] != ocr_output[j])
									break;
							}
							if(j >= 10)
							{
								dup=true;
								break;
							}
						}

						if(dup)
						{
							//duplicate
							print_dup(LcmFd);

							sleep(1);
							print_next(LcmFd);
						}
						else
						{
							for(i=0;i<10;i++)
								write_data(LcmFd,ocr_output[i]);
							write_data(LcmFd,' ');

							if(current/10)
								write_data(LcmFd,current/10+'0');
							write_data(LcmFd,current % 10 +'0');
							write_data(LcmFd,'/');
							if(number/10)
								write_data(LcmFd,number/10+'0');
							write_data(LcmFd,number % 10 + '0');
#ifdef __DEBUG_P__
							printf("LCD displayed!\n");
#endif
						}
					}


					if('0'!=current_buttons[2]){
#ifdef __DEBUG_P__
						printf("Button 3 down!\n");
#endif
						//Judge if it is enough

						//money_buf
						memcpy(money_buf+13*(current-1),ocr_output,10);
						memcpy(money_buf+13*(current-1)+10,SEPARATOR,3);

						if(current == number){
							decorate_buffer(money_buf,output_buf,number);
#ifdef __DEBUG_P__
							printf("Output buffer created!\n");
#endif
							WaitFdWriteable(CommFd);
#ifdef __DEBUG_P__
							printf("SerialPort can be writed!output_buf:\n");
							int i=0;
							for(i=0;i<13*number+18;i++)
								printf("%.2X ",output_buf[i]);
							printf("\n");
#endif
							write(CommFd,output_buf,13*number+18);
#ifdef __DEBUG_P__
							printf("SerialPort writed!\n");
#endif
							current = 0;

							print_complete(LcmFd);

							status = STATUS_SERIAL_READY;
#ifdef __DEBUG_P__
							printf("Status changed to SERIAL_READY!\n");
#endif
						}else if(current<number){
							printf("Scan next ready!\n");

							print_next(LcmFd);

							status = STATUS_SCAN_READY;
#ifdef __DEBUG_P__
							printf("Status changed to SCAN_READY!\n");
#endif
						}else
							Error("Number of money wrong!\n");

						//End button 3
					}else if('0'!=current_buttons[3]){
#ifdef __DEBUG_P__
						printf("Button 4 down!\n");
#endif
						//Cursor blink and go to modification 
						InitLcd_Flash(LcmFd);

						int i=0;
						for(i=0;i<10;i++)
							write_data(LcmFd,ocr_output[i]);
						write_data(LcmFd,' ');

						if(current/10)
							write_data(LcmFd,current/10+'0');
						write_data(LcmFd,current % 10 +'0');
						write_data(LcmFd,'/');
						if(number/10)
							write_data(LcmFd,number/10+'0');
						write_data(LcmFd,number % 10 + '0');

						write_com(LcmFd,0x80);
#ifdef __DEBUG_P__
						printf("LCD displayed and flash!\n");
#endif
						status = STATUS_MODIFICATION;
					}
					break;

				case STATUS_MODIFICATION:
					//button5,6(l/r/u/p) and button 3(confirm) are enabled
#ifdef __DEBUG_P__
					printf("Case modification!\n");
#endif
					if('0'!=current_buttons[2])
					{
#ifdef __DEBUG_P__
						printf("Button 3 down!\n");
#endif
						//Judge if it is enough

						pos = 0;
						//money_buf
						memcpy(money_buf+13*(current-1),ocr_output,10);
						memcpy(money_buf+13*(current-1)+10,SEPARATOR,3);

						if(current == number){
							decorate_buffer(money_buf,output_buf,number);
#ifdef __DEBUG_P__
							printf("Output buffer created!\n");
#endif
							WaitFdWriteable(CommFd);
#ifdef __DEBUG_P__
							printf("SerialPort can be writed!output_buf:\n");
							int i=0;
							for(i=0;i<13*number+18;i++)
								printf("%.2X ",output_buf[i]);
							printf("\n");
#endif
							write(CommFd,output_buf,13*number+18);
#ifdef __DEBUG_P__
							printf("SerialPort writed!\n");
#endif
							current = 0;
							print_complete(LcmFd);
							status = STATUS_SERIAL_READY;
#ifdef __DEBUG_P__
							printf("Status changed to SERIAL_READY!\n");
#endif
						}else if(current<number){
							printf("Scan next ready!\n");

							print_next(LcmFd);

							status = STATUS_SCAN_READY;
#ifdef __DEBUG_P__
							printf("Status changed to SCAN_READY!\n");
#endif
						}else
							Error("Number of money wrong!\n");
						//end button 3

					}else if('0'!=current_buttons[4]){
#ifdef __DEBUG_P__
						printf("Button 5 down!\n");
#endif
						//left/right position
						pos++;
						if(10 == pos)
							pos = 0;
						write_com(LcmFd,0x80+pos);
						//button 5
					}else if('0'!=current_buttons[5]){
#ifdef __DEBUG_P__
						printf("Button 6 down!\n");
#endif
						//up/down modification

						if(search(CODESET,ocr_output[pos],&index)!=0)
							Error("Illegal key word!");

						index++;
						if(index == (pos < 4 ? 36 : 10))
						{
							if (0==pos)
								index = 10;
							else
								index = 0;
						}
						memcpy(ocr_output+pos,CODESET+index,1);
						write_data(LcmFd,CODESET[index]);

						write_com(LcmFd,0x80+pos);

						//button 6
					}
					break;

				default:
					Error("Status Error!");
			}//Switch status

		}//button

	}//while


	close(CommFd);
	close(ButtonsFd);
	close(LcmFd);

	ioctl(PwmFd,PWM_IOCTL_STOP);
	close(PwmFd);

	close(LedFd);

	free(ocr_output);
	free(buf);
	free(output_buf);
	free(money_buf);
	free(imgname);
	free(imgprocname);
	return 0;
}//main


