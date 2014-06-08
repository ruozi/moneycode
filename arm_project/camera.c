#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <assert.h>

#include <getopt.h>           

#include <fcntl.h>            

#include <unistd.h>

#include <errno.h>

#include <malloc.h>

#include <sys/stat.h>

#include <sys/types.h>

#include <sys/time.h>

#include <sys/mman.h>

#include <sys/ioctl.h>

#include <asm/types.h>        

#include <linux/videodev2.h>

extern "C"{
#include <jpeglib.h>
}



#define OUTPUT_BUF_SIZE  4096

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define WIDTH 640

#define HEIGHT 480


const static unsigned char dht_data[] = {
	0xff, 0xc4, 0x01, 0xa2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
	0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x01, 0x00, 0x03,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05,
	0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04,
	0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22,
	0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15,
	0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
	0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
	0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
	0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95,
	0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
	0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2,
	0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5,
	0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
	0xfa, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05,
	0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11, 0x04,
	0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22,
	0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33,
	0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25,
	0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
	0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
	0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
	0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94,
	0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
	0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
	0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa
};



struct buffer {

	void   *start;

	size_t length;

};



typedef struct {

	struct jpeg_destination_mgr pub;



	JOCTET * buffer; 



	unsigned char *outbuffer;

	int outbuffer_size;

	unsigned char *outbuffer_cursor;

	int *written;



}mjpg_destination_mgr;



typedef mjpg_destination_mgr *mjpg_dest_ptr;



static const char *           dev_name        = "/dev/video0";

static int              fd              = -1;

struct buffer *         buffers         = NULL;

static unsigned int     n_buffers       = 0;

FILE *file_fd;

static unsigned long file_length;

static unsigned char *file_name;



METHODDEF(void) init_destination(j_compress_ptr cinfo) {

	mjpg_dest_ptr dest = (mjpg_dest_ptr) cinfo->dest;

	dest->buffer = (JOCTET *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE, OUTPUT_BUF_SIZE * sizeof(JOCTET));

	*(dest->written) = 0;

	dest->pub.next_output_byte = dest->buffer;

	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

}



METHODDEF(boolean) empty_output_buffer(j_compress_ptr cinfo) {

	mjpg_dest_ptr dest = (mjpg_dest_ptr) cinfo->dest;

	memcpy(dest->outbuffer_cursor, dest->buffer, OUTPUT_BUF_SIZE);

	dest->outbuffer_cursor += OUTPUT_BUF_SIZE;

	*(dest->written) += OUTPUT_BUF_SIZE;

	dest->pub.next_output_byte = dest->buffer;

	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;



	return TRUE;

}



METHODDEF(void) term_destination(j_compress_ptr cinfo) {

	mjpg_dest_ptr dest = (mjpg_dest_ptr) cinfo->dest;

	size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

	/* Write any data remaining in the buffer */

	memcpy(dest->outbuffer_cursor, dest->buffer, datacount);

	dest->outbuffer_cursor += datacount;

	*(dest->written) += datacount;

}



void dest_buffer(j_compress_ptr cinfo, unsigned char *buffer, int size, int *written) {

	mjpg_dest_ptr dest;

	if (cinfo->dest == NULL) {

		cinfo->dest = (struct jpeg_destination_mgr *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(mjpg_destination_mgr));

	}



	dest = (mjpg_dest_ptr)cinfo->dest;

	dest->pub.init_destination = init_destination;

	dest->pub.empty_output_buffer = empty_output_buffer;

	dest->pub.term_destination = term_destination;

	dest->outbuffer = buffer;

	dest->outbuffer_size = size;

	dest->outbuffer_cursor = buffer;

	dest->written = written;

}







static int decoder_mjpeg_decode( 
		unsigned char *out_buf, 
		unsigned char *in_buf,
		int buf_size)
{
	int pos = 0;
	int size_start = 0;
	unsigned char *pdeb = in_buf;
	unsigned char *pcur = in_buf;
	unsigned char *plimit = in_buf + buf_size;
	unsigned char *jpeg_buf;

	{
		printf("no huffman\n");

		/* find the SOF0(Start Of Frame 0) of JPEG */
		while ( (((pcur[0] << 8) | pcur[1]) != 0xffc0) && (pcur < plimit) )
		{
			pcur++;
		}

		/* SOF0 of JPEG exist */
		if (pcur < plimit)
		{
			printf("SOF0 existed at position\n");
			jpeg_buf = (unsigned char *)malloc(buf_size + sizeof(dht_data) + 10);

			if (jpeg_buf != NULL)
			{
				/* insert huffman table after SOF0 */
				size_start = pcur - pdeb;
				memcpy(jpeg_buf, in_buf, size_start);
				pos += size_start;

				memcpy(jpeg_buf + pos, dht_data, sizeof(dht_data));
				pos += sizeof(dht_data);

				memcpy(jpeg_buf + pos, pcur, buf_size - size_start);
				pos += buf_size - size_start;

				memcpy(out_buf,jpeg_buf,(buf_size+(sizeof(dht_data) + 10)));
				return (buf_size+(sizeof(dht_data) + 10));
				//            decoder_jpeg_decompress(out_buf, jpeg_buf, pos);

				free(jpeg_buf);
				jpeg_buf = NULL;
			}
		}
	}

}






//摄像头采集的YUYV格式转换为JPEG格式

int compress_yuyv_to_jpeg(unsigned char *buf, unsigned char *buffer, int size, int quality) {

	struct jpeg_compress_struct cinfo;

	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];

	unsigned char *line_buffer, *yuyv;

	int z;

	static int written;

	//int count = 0;

	//printf("%s\n", buf);

	line_buffer = (unsigned char *)calloc (WIDTH * 3, 1);

	yuyv = buf;//将YUYV格式的图片数据赋给YUYV指针

	printf("compress start...\n");

	cinfo.err = jpeg_std_error (&jerr);

	jpeg_create_compress (&cinfo);

	/* jpeg_stdio_dest (&cinfo, file); */

	dest_buffer(&cinfo, buffer, size, &written);



	cinfo.image_width = WIDTH;

	cinfo.image_height = HEIGHT;

	cinfo.input_components = 3;

	cinfo.in_color_space = JCS_RGB;



	jpeg_set_defaults (&cinfo);

	jpeg_set_quality (&cinfo, quality, TRUE);

	jpeg_start_compress (&cinfo, TRUE);



	z = 0;

	while (cinfo.next_scanline < HEIGHT) {

		int x;

		unsigned char *ptr = line_buffer;



		for (x = 0; x < WIDTH; x++) {

			int r, g, b;

			int y, u, v;



			if (!z)

				y = yuyv[0] << 8;

			else

				y = yuyv[2] << 8;

			u = yuyv[1] - 128;

			v = yuyv[3] - 128;



			r = (y + (359 * v)) >> 8;

			g = (y - (88 * u) - (183 * v)) >> 8;

			b = (y + (454 * u)) >> 8;



			*(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);

			*(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);

			*(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);



			if (z++) {

				z = 0;

				yuyv += 4;

			}

		}



		row_pointer[0] = line_buffer;

		jpeg_write_scanlines (&cinfo, row_pointer, 1);

	}



	jpeg_finish_compress (&cinfo);

	jpeg_destroy_compress (&cinfo);



	free (line_buffer);

	return (written);

}



//读取一帧的内容

static int read_frame (void)

{

	struct v4l2_buffer buf;

	int ret;

	unsigned int i;

	CLEAR (buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	buf.memory = V4L2_MEMORY_MMAP;

	int ff = ioctl (fd, VIDIOC_DQBUF, &buf); //出列采集的帧缓冲

	if(ff<0)

		printf("failture\n");

	unsigned char src[buf.length+1];

	unsigned char dest[buf.length+1];

	assert (buf.index < n_buffers);

	printf ("buf.index dq is %d,\n",buf.index);



	//memcpy(src, buffers[buf.index].start, buf.length);
	memcpy(src, buffers[0].start, buf.length);

	ret =	decoder_mjpeg_decode(dest,src,buf.length);
	// ret = compress_yuyv_to_jpeg(src, dest,(WIDTH * HEIGHT), 80);//数据转换


	printf("buf length  is %d \n",buf.length);

	// fwrite(src, buf.length, 1, file_fd);//转换后的数据写入
	fwrite(dest, ret, 1, file_fd);//转换后的数据写入

	ff=ioctl (fd, VIDIOC_QBUF, &buf); //重新入列

	if(ff<0)

		printf("failture VIDIOC_QBUF\n");

	return 1;

}









int get_pic(const char *imgname)
{

	struct v4l2_capability cap;

	struct v4l2_format fmt;

	unsigned int i;

	enum v4l2_buf_type type;



	file_fd = fopen(imgname, "w");

	fd = open (dev_name, O_RDWR | O_NONBLOCK, 0);

	int ff=ioctl(fd, VIDIOC_QUERYCAP, &cap);//获取摄像头参数



	if(ff<0)

		printf("failture VIDIOC_QUERYCAP\n");



	struct v4l2_fmtdesc fmt1;

	int ret;



	memset(&fmt1, 0, sizeof(fmt1));

	fmt1.index = 0;

	fmt1.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt1)) == 0) //查看摄像头所支持的格式

	{

		fmt1.index++;

		printf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",

				fmt1.pixelformat & 0xFF, (fmt1.pixelformat >> 8) & 0xFF,

				(fmt1.pixelformat >> 16) & 0xFF, (fmt1.pixelformat >> 24) & 0xFF,

				fmt1.description);

	}

	CLEAR (fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	fmt.fmt.pix.width       = 640;

	fmt.fmt.pix.height      = 480;

	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;  //V4L2_PIX_FMT_YUYV;

	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	/*

	   stream_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	   stream_fmt.fmt.pix.width = CapWidth;  
	   stream_fmt.fmt.pix.height = CapHeight;  
	   stream_fmt.fmt.pix.pixelformat = PIXELFMT;  
	   stream_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;  

	 */ 

	ff = ioctl (fd, VIDIOC_S_FMT, &fmt); //设置图像格式

	if(ff<0)
		printf("failture sssssssysssVIDIOC_S_FMT\n");






	ff = ioctl (fd, VIDIOC_G_FMT, &fmt); //设置图像格式

	if(ff<0)


		printf("failtur VIDIOC_G_FMT\n");


	printf("fmt.fmt.pix.width is %d,fmt.fmt.pix.height is %d \n", fmt.fmt.pix.width,fmt.fmt.pix.height);










	file_length = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height; //计算图片大小

	struct v4l2_requestbuffers req;

	CLEAR (req);

	req.count               = 1;

	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	req.memory              = V4L2_MEMORY_MMAP;



	ioctl (fd, VIDIOC_REQBUFS, &req);  //申请缓冲，count是申请的数量

	if(ff<0)

		printf("failture VIDIOC_REQBUFS\n");

	if (req.count < 1)

		printf("Insufficient buffer memory\n");

	buffers = (struct buffer *)calloc (req.count, sizeof (*buffers));//内存中建立对应空间





	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)

	{

		struct v4l2_buffer buf; 

		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		buf.memory      = V4L2_MEMORY_MMAP;

		buf.index       = n_buffers;

		if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)) //映射用户空间

			printf ("VIDIOC_QUERYBUF error\n");

		buffers[n_buffers].length = buf.length;

		buffers[n_buffers].start=mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset); //通过mmap建立映射关系

		if (MAP_FAILED == buffers[n_buffers].start)

			printf ("mmap failed\n");

	}



	for (i = 0; i < n_buffers; ++i)

	{

		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		buf.memory      = V4L2_MEMORY_MMAP;

		buf.index       = i;

		if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))//申请到的缓冲进入列队

			printf ("VIDIOC_QBUF failed\n");

	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) //开始捕捉图像数据

		printf ("VIDIOC_STREAMON failed\n");



	for (;;) //这一段涉及到异步IO

	{
		fd_set fds;

		struct timeval tv;

		int r;

		FD_ZERO (&fds);//将指定的文件描述符集清空

		FD_SET (fd, &fds);//在文件描述符集合中增加一个新的文件描述符

		/* Timeout. */

		tv.tv_sec = 10;

		tv.tv_usec = 0;

		r = select (fd + 1, &fds, NULL, NULL, &tv);//判断是否可读（即摄像头是否准备好），tv是定时

		if (-1 == r)

		{

			if (EINTR == errno)

				continue;

			printf ("select err\n");

		}

		if (0 == r)

		{

			fprintf (stderr, "select timeout\n");


			return (EXIT_FAILURE);

		}


		if (read_frame ())//如果可读，执行read_frame函数
			break;
	}
unmap:
	for (i = 0; i < n_buffers; ++i)
		if (-1 == munmap (buffers[i].start, buffers[i].length))
			printf ("munmap error");
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

	if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))   
		printf("VIDIOC_STREAMOFF"); 
	close (fd);
	fclose (file_fd);
	return 0;
}

#ifdef __DEBUG__
int main()
{
	get_pic("test-mmap.jpg");
	return 0;
}
#endif
