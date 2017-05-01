CC=g++
PROG=executable
OBJS=blurDetection.o boundingBoxDetection.o getImage.o imageToText.o \
	   	main.o prahvi.o scanner.o similarityDetection.o tfidf.o
CFLAGS= -g -std=c++11 -Wall -pedantic $(shell pkg-config --cflags opencv tesseract)
LDFLAGS= $(shell pkg-config --libs opencv tesseract)

.PHONY: all clean

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $<

all: $(PROG)

clean:
	rm -f $(OBJS) $(PROG)

#/all: executable
#/
#/executable: $(OBJECTS)
#/	$(CC) $(CPPFLAGS) $(LDLIBS) -o executable $(OBJECTS)
#/
#/main.o: main.cpp
#/
#/blurDetection.o: blurDetection.cpp blurDetection.hpp
#/
#/boundingBoxDetection.o: boundingBoxDetection.cpp boundingBoxDetection.hpp
#/
#/getImage.o: getImage.cpp getImage.hpp
#/
#/imageToText.o: imageToText.cpp imageToText.hpp
#/
#/prahvi.o: prahvi.cpp prahvi.hpp
#/
#/scanner.o: scanner.cpp scanner.hpp
#/
#/similarityDetection.o: similarityDetection.cpp similarityDetection.hpp
#/
#/tfidf.o: tfidf.cpp tfidf.hpp
#/
