CROSS=arm-linux-
DEBUG=-D__DEBUG_P__
ROTATE=-D__ROTATE__

all: money

money: moneycode.c parse.c lcm.c utils.c camera.c recog.cpp imgprocess.cpp 
	$(CROSS)g++ -c moneycode.c $(DEBUG) 
	$(CROSS)g++ -c parse.c
	$(CROSS)g++ -c lcm.c
	$(CROSS)g++ -c utils.c $(DEBUG)
	$(CROSS)g++ -c camera.c
	$(CROSS)g++ -c recog.cpp
	$(CROSS)g++ -c imgprocess.cpp $(ROTATE)
	$(CROSS)g++ -o money moneycode.o parse.o lcm.o utils.o camera.o recog.o imgprocess.o -ltesseract -lopencv_core -lopencv_highgui -lopencv_imgproc -lrt -ljpeg

debug: moneycode.c parse.c camera.c
	gcc -c moneycode.c $(DEBUG)
	gcc -c parse.c
	gcc -c camera.c
	gcc -o money moneycode.o parse.o camera.o -ljpeg

clean:
	@rm -vf led *.o *~
