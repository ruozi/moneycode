#include <stdio.h>
#include <string.h>
#include "recog.h"

int recog(const char * input_img_name,char *output)
{
	tesseract::TessBaseAPI *api_alpha = new tesseract::TessBaseAPI();
	tesseract::TessBaseAPI *api_digit = new tesseract::TessBaseAPI();
	if (api_alpha->Init(NULL,"money")){
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}

	if (api_digit->Init(NULL,"money")){
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}

	api_alpha->SetVariable("tessedit_char_whitelist","0123456789ABCDEFGHIJKLMNPQRSTUVWXYZ");
	api_digit->SetVariable("tessedit_char_whitelist","0123456789");

	Pix *image = pixRead(input_img_name);

	api_alpha->SetImage(image);
	api_digit->SetImage(image);

	char *digits = (char *)malloc(15*sizeof(char));
	strcpy(digits,api_digit->GetUTF8Text());

	char *trim = (char *)malloc(15*sizeof(char));
	int i=0;
	int len=0;
	for(i=0;digits[i]!='\0';i++)
	{
		if(digits[i]>='0' && digits[i]<='9')
			trim[len++]=digits[i];
	}
	strcpy(digits,trim);
	free(trim);

	memcpy(output,api_alpha->GetUTF8Text(),4);
	memcpy(output+4,digits+len-6,6);
#ifdef __DEBUG_P__
	printf("OCR digit:%s\n",api_digit->GetUTF8Text());
	printf("OCR digit length:%d\n",strlen(api_digit->GetUTF8Text()));
	printf("OCR digit trimmed:%s\n",digits);
	printf("OCR digit trimmed length:%d\n",strlen(digits));
	printf("OCR alpha:%s\n",api_alpha->GetUTF8Text());
	printf("OCR output:%s\n",output);
#endif

	api_alpha->End();
	api_digit->End();
	pixDestroy(&image);
    free(digits);

	delete(api_alpha);
	delete(api_digit);
	return 0;
}

#ifdef __DEBUG__
int main()
{
	char *text_out;

	tesseract::TessBaseAPI *api_alpha = new tesseract::TessBaseAPI();
	if (api_alpha->Init(NULL,"eng")){
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}

	Pix *image = pixRead("test.jpg");
	api_alpha->SetImage(image);

	text_out = api_alpha->GetUTF8Text();
	printf("OCR output:%s\n",text_out);

	api_alpha->End();
	delete[] text_out;
	pixDestroy(&image);

	return 0;

}
#endif
