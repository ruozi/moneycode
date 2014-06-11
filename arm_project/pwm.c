#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define PWM_IOCTL_SET_FREQ		1
#define PWM_IOCTL_STOP			0

static int getch(void)
{
	struct termios oldt,newt;
	int ch;

	if (!isatty(STDIN_FILENO)) {
		fprintf(stderr, "this problem should be run at a terminal\n");
		exit(1);
	}
	// save terminal setting
	if(tcgetattr(STDIN_FILENO, &oldt) < 0) {
		perror("save the terminal setting");
		exit(1);
	}

	// set terminal as need
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	if(tcsetattr(STDIN_FILENO,TCSANOW, &newt) < 0) {
		perror("set terminal");
		exit(1);
	}

	ch = getchar();

	// restore termial setting
	if(tcsetattr(STDIN_FILENO,TCSANOW,&oldt) < 0) {
		perror("restore the termial setting");
		exit(1);
	}
	return ch;
}

static void set_buzzer_freq(int fd,int freq)
{
	// this IOCTL command is the key to set frequency
	int ret = ioctl(fd, PWM_IOCTL_SET_FREQ, freq);
	if(ret < 0) {
		perror("set the frequency of the buzzer");
		exit(1);
	}
}
static void stop_buzzer(int fd)
{
	int ret = ioctl(fd, PWM_IOCTL_STOP);
	if(ret < 0) {
		perror("stop the buzzer");
		exit(1);
	}
}

int beep(int fd)
{
	int freq = 1000 ;
 	int mdelay;	

	set_buzzer_freq(fd,freq);
	for(mdelay =1000000;mdelay>0;mdelay--);
	stop_buzzer(fd);

	return 0;
}

#ifdef __DEBUG__
int main(int argc,char **argv)
{
	int fd=open("/dev/pwm",0);
	if(fd < 0)
	{
		perror("Unable to open pwm");
		exit(1);
	}

	beep(fd);
	close(fd);
	return 0;
}
#endif
