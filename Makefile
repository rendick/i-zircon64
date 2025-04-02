NAME=i-zircon64
CC=gcc
CFLAGS=-Wall -lxcb -lxcb-keysyms -lxcb-icccm -g

build: $(NAME)-bin
	$(CC) $(CFLAGS) -o $(NAME)-bin ./src/$(NAME).c 

clean:
	rm -f $(NAME)-bin