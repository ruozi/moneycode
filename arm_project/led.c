#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>


static int setstat(int fd,int value,int no)
{

	
         if(value ==1)
            value = 0;
         else
            value = 1;
//	printf("value is %d, no is %d \n",value,no);

 	 ioctl(fd,value,no);
 	return 0;
}

static int write_data(int fd, unsigned char data)
{
	int i = 0;
        int value  = 0 ;
	int mdelay =0;     
 
	setstat(fd,1,14); //rs  = 0	Êı¾İ
	setstat(fd,0,13); //w/r  = 0

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


static int write_com(int fd, unsigned char data)
{
        int i = 0;
        int value  = 0 ;
        int mdelay =0;

        setstat(fd,0,14); //rs  = 0	Ö¸Áî
	setstat(fd,0,13); //w/r  = 0

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


static void InitLcd(int  fd)							   // åˆå§‹åŒ–LCD
{
	
	write_com(fd,0x30);
	write_com(fd,0x30);
	write_com(fd,0x30);
	write_com(fd,0x38);
	write_com(fd,0x0c);
	write_com(fd,0x01);
	write_com(fd,0x06);
}

int backlight_on(int fd)
{
	setstat(fd,0,15);
}

int backlight_off(int fd)
{
	setstat(fd,1,15);
}

int illumination_on(int fd)
{
	setstat(fd,0,16);
}

int illumination_off(int fd)
{
	setstat(fd,1,16);
}

#ifdef __DEBUG__
int main(int argc, char **argv)
{
	int on;
	int led_no;
	int fd,num;
	int mdelay =0;     
	int a, b;
	
	fd = open("/dev/leds0", 0);
	if (fd < 0) {
		fd = open("/dev/leds", 0);
	}
	if (fd < 0) {
		perror("open device leds");
		exit(1);
	}
	a= strtoul(argv[1], 0, 10);
	b= strtoul(argv[2], 0, 10);
	printf("argv[1] = %d\n", a);	
	printf("argv[2] = %d\n", b);	
	setstat(fd,b,a); //w/r  = 0

	close(fd);
	return 0;
}
#endif
