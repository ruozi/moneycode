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

	strcpy(output,api_digit->GetUTF8Text());
	memcpy(output,api_alpha->GetUTF8Text(),4);

	printf("OCR output:%s\n",output);

	api_alpha->End();
	api_digit->End();
	pixDestroy(&image);

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
