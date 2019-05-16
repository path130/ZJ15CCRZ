PREFIX = arm-none-linux-gnueabi-
all: libfacedetector.a

libfacedetector.a: libfacedetector.cpp
	$(PREFIX)g++ -o libfacedetector.o -c libfacedetector.cpp -I./
	$(PREFIX)ar cr libfacedetector.a libfacedetector.o

clean:
	rm -f *.o ~* libfacedetector.a 
