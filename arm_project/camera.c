#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
extern "C"{
#include <jpeglib.h>
}
#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

//#include <linux/delay.h>

//#include "videodev2.h"

#define FONTDATAMAX 4096
#define V4L2_DEV_NODE "/dev/video0"

static unsigned short image_width = 640;
static unsigned short image_height = 480;
static unsigned char *image_buffer;
static unsigned short optimization = 75;

int write_JPEG_file (const char * filename, int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL)
	{
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width; 	/* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

	return 0;
	/* And we're done! */
}

static void save_picture(unsigned char *src,const char *imgname)
{
	int y, x;
	unsigned long rgb_data;
	unsigned short w = image_width;
	unsigned short h = image_height;
	unsigned char *src_y = src;
	unsigned char *src_cb = src_y+image_width*image_height;
	unsigned char *src_cr = src_cb+image_width*image_height/2;
	//int palette = ;

	image_buffer = (unsigned char*)malloc(w*h*3);	//RGB24
	if(image_buffer==NULL) {
		printf("allocate memory fail in saving picture!\n");
		return;
	}

	memset(image_buffer, 0, w*h*3);

	for(y=0; y<h; y++)
	{
		for(x=0; x<w; x++) 
		{
			image_buffer[(y*w+x)*3]   = (((__u16 *)src)[y*w+x]&0xf800)>>8;
			image_buffer[(y*w+x)*3+1] = (((__u16 *)src)[y*w+x]&0x07e0)>>3;
			image_buffer[(y*w+x)*3+2] = (((__u16 *)src)[y*w+x]&0x001f)<<3;
		}
	}

	write_JPEG_file(imgname, optimization);
	free(image_buffer);
}


int get_pic(const char *imgname)
{
	struct v4l2_format fmt;
	struct v4l2_requestbuffers requestbuffers;
	unsigned char str[] = "中";
	unsigned char is_ok;
	int v4l2_fd = -1;
	unsigned char *buf;
	int i;

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	FILE* file_fd = fopen("rgb565.jpg", "w");

	/* A1 打开摄像头驱动 */
	v4l2_fd = open(V4L2_DEV_NODE, O_RDWR);
	if (v4l2_fd < 0)
	{
		printf(" open video ERR\n");
		return -1;
	}

	/* A2 vidioc_querycap(查看是否是摄像头设备) */
	/* A3 列举支持哪种格式 */
	/* A4 返回当前所使用的格式 */
	/* A6 设置格式 */
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;

	if((ioctl(v4l2_fd, VIDIOC_S_FMT, &fmt))<0)
	{
		printf("Error: failed to set video format.\n");
		return -1;
	}

	/* A7 APP调用该ioctl让驱动程序分配若干个缓存, APP将从这些缓存中读到视频数据 */
	requestbuffers.count = 4;
	requestbuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	requestbuffers.memory = V4L2_MEMORY_MMAP;
	//V4L2_MEMORY_MMAP;
	if((ioctl(v4l2_fd, VIDIOC_REQBUFS, &requestbuffers))<0)
	{
		printf("Error: failed to request buffers.\n");
		return -1;
	}

	/* A8 开始采集数据 */
	if((ioctl(v4l2_fd, VIDIOC_STREAMON, &type))<0)
	{
		printf("error ioctl streamon\n");
		return 0;
	}

	/* 显示出来 */
	int buf_size = fmt.fmt.pix.sizeimage;
	printf("app:buf_size = %d\n", buf_size);
	buf = (unsigned char*)malloc(buf_size);
	memset(buf, 0, buf_size);

	for(i = 0; i < 5 ; i++)
	{
		read(v4l2_fd, buf, buf_size);
	}

	printf("read: %s %d\n", __func__, __LINE__);
	read(v4l2_fd, buf, buf_size);
	printf("save: %s %d\n", __func__, __LINE__);
	save_picture(buf,imgname);
	printf("start write: %s %d\n", __func__, __LINE__);
	fwrite(buf, buf_size, 1, file_fd);
	printf("write ok: %s %d\n", __func__, __LINE__);
	//memcpy(fbmem, buf, buf_size);


	if((ioctl(v4l2_fd, VIDIOC_STREAMON, &type))<0)
	{
		printf("error ioctl streamon\n");
		//return 0;
	}
	else
		printf("success ioctl streamon\n");

	remove("rgb565.jpg");

	free(buf);
	close(v4l2_fd);
	return 0;
}

#ifdef __DEBUG__
int main(int argc,char **argv)
{
	get_pic("test.jpg");
	return 0;
}
#endif
