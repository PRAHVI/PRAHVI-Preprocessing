CC=g++ 
TARGET=libprahvi
OBJS=blurDetection.o boundingBoxDetection.o imageToText.o \
	 	prahvi.o scanner.o similarityDetection.o prahvi_wrapper.o
CFLAGS= -g -std=c++11 -Wall -pedantic $(shell pkg-config --cflags opencv tesseract)
LDFLAGS= -L$(PYTHON_LIB) -lpython$(PYTHON_VERSION) $(shell pkg-config --libs opencv tesseract)
PYTHON_VERSION=2.7
PYTHON_INCLUDE=$(VIRTUAL_ENV)/include/python$(PYTHON_VERSION)
PYTHON_LIB=$(VIRTUAL_ENV)/lib/python$(PYTHON_VERSION)/config

.PHONY: all clean

$(TARGET).so: $(OBJS)
	$(CC) -shared  $(OBJS) $(LDFLAGS) -o $(TARGET).so

%.o: %.cpp
	$(CC) -I$(PYTHON_INCLUDE) -fPIC -c $(CFLAGS) $<

all: $(PROG)

clean:
	rm -f $(OBJS) $(PROG)

