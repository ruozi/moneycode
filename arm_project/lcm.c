#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int setstat(int fd,int value,int no)
{


	if(value ==1)
		value = 0;
	else
		value = 1;
	//  printf("value is %d, no is %d \n",value,no);

	ioctl(fd,value,no);
	return 0;
}

int write_data(int fd, unsigned char data)
{
	int i = 0;
	int value  = 0 ;
	int mdelay =0;

	setstat(fd,1,14); //rs  = 0

	for(mdelay =1000000;mdelay >0 ;mdelay-- );
	for(i=11;i>3;i--)
	{
		value = (data>>(11-i))&0x1;
		//       printf(" data is %d,value[%d] = %d \r\n",data,(11-i),value );  
		setstat(fd,value,i);
	}

	setstat(fd,1,12); //E  = 1

	for(mdelay =10000;mdelay >0 ;mdelay-- );
	setstat(fd,0,12); //E  = 1

	for(mdelay =100000;mdelay >0 ;mdelay-- );
	return 0;
}

int write_com(int fd, unsigned char data)
{
	int i = 0;
	int value  = 0 ;
	int mdelay =0;

	setstat(fd,0,14); //rs  = 0

	for(i=11;i>3;i--)
	{
		value = (data>>(11-i))&0x1;

		//    printf(" data is %d,value[%d] = %d \r\n",data,(11-i),value );  



		setstat(fd,value,i);
	}


	setstat(fd,1,12); //E  = 1

	for(mdelay =10000;mdelay >0 ;mdelay-- );

	setstat(fd,0,12); //E  = 1

	for(mdelay =100000;mdelay >0 ;mdelay-- );
	return 0;
}


void InitLcd(int  fd)                              // 初始化LCD
{
	int mdelay=0;

	setstat(fd,0,13);
	setstat(fd,0,12);

	write_com(fd,0x30);
	write_com(fd,0x30);
	write_com(fd,0x30);
	write_com(fd,0x38);
	write_com(fd,0x0C);
	write_com(fd,0x01);
	write_com(fd,0x06);

	for(mdelay =10000000;mdelay >0 ;mdelay-- );

	write_com(fd,0x80);  //设置DDRAM地址,回到行头 
}

void InitLcd_Flash(int  fd)                              // 初始化LCD
{
	int mdelay=0;

	setstat(fd,0,13);
	setstat(fd,0,12);

	write_com(fd,0x30);
	write_com(fd,0x30);
	write_com(fd,0x30);
	write_com(fd,0x38);
	write_com(fd,0x0F);
	write_com(fd,0x01);
	write_com(fd,0x06);

	for(mdelay =10000000;mdelay >0 ;mdelay-- );

	write_com(fd,0x80);  //设置DDRAM地址,回到行头 
}

int print_ready(int fd)
{
	InitLcd(fd);

	write_data(fd,'O');
	write_data(fd,'K');

	write_com(fd,0x80+0x40);

	write_data(fd,'R');
	write_data(fd,'E');
	write_data(fd,'A');
	write_data(fd,'D');
	write_data(fd,'Y');

	return 0;
}

int print_next(int fd)
{
	InitLcd(fd);

	write_data(fd,'S');
	write_data(fd,'C');
	write_data(fd,'A');
	write_data(fd,'N');
	write_data(fd,' ');
	write_data(fd,'N');
	write_data(fd,'E');
	write_data(fd,'X');
	write_data(fd,'T');
	write_data(fd,' ');
	write_data(fd,'M');
	write_data(fd,'O');
	write_data(fd,'N');
	write_data(fd,'E');
	write_data(fd,'Y');
}

int print_scan(int fd,int num)
{
	InitLcd(fd);

	if(num/10)
		write_data(fd,num/10+'0');
	write_data(fd,num%10+'0');
	write_data(fd,' ');
	write_data(fd,'M');
	write_data(fd,'O');
	write_data(fd,'N');
	write_data(fd,'E');
	write_data(fd,'Y');
	write_data(fd,' ');
	write_data(fd,'T');
	write_data(fd,'O');
	write_data(fd,' ');
	write_data(fd,'S');
	write_data(fd,'C');
	write_data(fd,'A');
	write_data(fd,'N');
}

int print_complete(int fd)
{
	InitLcd(fd);

	write_data(fd,'C');
	write_data(fd,'O');
	write_data(fd,'M');
	write_data(fd,'P');
	write_data(fd,'L');
	write_data(fd,'E');
	write_data(fd,'T');
	write_data(fd,'E');
	write_data(fd,'!');
}

int print_recog(int fd)
{
	InitLcd(fd);

	write_data(fd,'R');
	write_data(fd,'E');
	write_data(fd,'C');
	write_data(fd,'O');
	write_data(fd,'G');
	write_data(fd,'N');
	write_data(fd,'I');
	write_data(fd,'Z');
	write_data(fd,'I');
	write_data(fd,'N');
	write_data(fd,'G');
	write_data(fd,'.');
	write_data(fd,'.');
}

int print_error(int fd)
{
	InitLcd(fd);

	write_data(fd,'E');
	write_data(fd,'R');
	write_data(fd,'R');
	write_data(fd,'O');
	write_data(fd,'R');

}
#ifdef __DEBUG__
int main(int argc, char** argv)
{
	int fd;

	fd=open("/dev/leds0",0);
	if(fd<0){
		fd=open("/dev/leds",0);
	}
	if(fd<0){
		perror("Open devices leds");
		exit(1);
	}

	InitLcd(fd);

	write_data(fd,'O');
	write_data(fd,'K');

	write_com(fd,0x80+0x40);

	write_data(fd,'R');
	write_data(fd,'E');
	write_data(fd,'A');
	write_data(fd,'D');
	write_data(fd,'Y');

	close(fd);
	
	return 0;
}

#endif


