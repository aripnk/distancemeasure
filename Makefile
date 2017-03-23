CC=g++ -std=c++11
CFLAGS=`pkg-config --cflags opencv --libs opencv`

All	: main
main	: manciksayang.cpp
	$(CC) manciksayang.cpp -o manciksayang $(CFLAGS) -levent  -pthread

clean	:
	rm -rf manciksayang
