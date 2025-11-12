all: parent child

parent: parent.c
	gcc  -o parent parent.c -lrt -lpthread

child: child.c
	gcc -o child child.c -lrt -lpthread
