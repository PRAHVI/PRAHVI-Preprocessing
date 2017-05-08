CC=g++ 
TARGET1=libprahvi3
TARGET2=libprahvi4
OBJS1=blurDetection1.o boundingBoxDetection1.o imageToText1.o \
	 	prahvi1.o scanner1.o similarityDetection1.o prahvi_wrapper1.o
OBJS2=blurDetection2.o boundingBoxDetection2.o imageToText2.o \
	 	prahvi2.o scanner2.o similarityDetection2.o prahvi_wrapper2.o
OBJS=blurDetection.o boundingBoxDetection2.o imageToText2.o \
	 	prahvi2.o scanner2.o similarityDetection2.o prahvi_wrapper2.o

SOURCES=blurDetection.cpp boundingBoxDetection.cpp imageToText.cpp \
	 	prahvi.cpp scanner.cpp similarityDetection.cpp prahvi_wrapper.cpp

TESSERACT3_INCLUDE=-I/home/abemillan/Developer/PRAHVI/dependencies/include/tesseract-3.05
TESSERACT4_INCLUDE=-I/home/abemillan/Developer/PRAHVI/dependencies/include/tesseract-4.00
OPENCV_INCLUDE=-I/home/abemillan/Developer/PRAHVI/dependencies/include/opencv
LEPTONICA_INCLUDE=-I/home/abemillan/Developer/PRAHVI/dependencies/include/leptonica

TESSERACT3_LIB=-L/home/abemillan/Developer/PRAHVI/dependencies/lib/tesseract-3.05 -ltesseract
TESSERACT4_LIB=-L/home/abemillan/Developer/PRAHVI/dependencies/lib/tesseract-4.00 -ltesseract
OPENCV_LEPT_LIB=-L/home/abemillan/Developer/PRAHVI/dependencies/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -llept


CFLAGS1= -g -std=c++11 -Wall -pedantic $(TESSERACT3_INCLUDE) $(OPENCV_INCLUDE) $(LEPTONICA_INCLUDE)
CFLAGS2= -g -std=c++11 -Wall -pedantic $(TESSERACT4_INCLUDE) $(OPENCV_INCLUDE) $(LEPTONICA_INCLUDE)
LDFLAGS1= -L$(PYTHON_LIB) -lpython$(PYTHON_VERSION) $(TESSERACT3_LIB) $(OPENCV_LEPT_LIB)
LDFLAGS2= -L$(PYTHON_LIB) -lpython$(PYTHON_VERSION) $(TESSERACT4_LIB) $(OPENCV_LEPT_LIB)

PYTHON_VERSION=2.7
PYTHON_INCLUDE=$(VIRTUAL_ENV)/include/python$(PYTHON_VERSION)
PYTHON_LIB=$(VIRTUAL_ENV)/lib/python$(PYTHON_VERSION)/config

.PHONY: all clean

all: $(TARGET1).so $(TARGET2).so

$(TARGET2).so: $(OBJS2)
	$(CC) -shared  $(OBJS2) $(LDFLAGS2) -o $(TARGET2).so

$(TARGET1).so: $(OBJS1)
	$(CC) -shared  $(OBJS1) $(LDFLAGS1) -o $(TARGET1).so

%1.o: %.cpp
	$(CC) -I$(PYTHON_INCLUDE) -fPIC -c $(CFLAGS1) $< -o $@

%2.o: %.cpp
	$(CC) -I$(PYTHON_INCLUDE) -fPIC -c $(CFLAGS2) $< -o $@

clean:
	rm -f *.o $(TARGET1).so $(TARGET2).so

