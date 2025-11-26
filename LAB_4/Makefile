all: compile_demo1 compile_demo2 runtime_demo

libprime_sort1.so: src/library_No1.c 
	gcc -o libprime_sort1.so -fPIC -shared src/library_No1.c -lm

libprime_sort2.so: src/library_No2.c 
	gcc -o libprime_sort2.so -fPIC -shared src/library_No2.c 
	
compile_demo1: programm_No1.c libprime_sort1.so
	gcc programm_No1.c -L. -lprime_sort1 -lm -Wl,-rpath,. -o compile_demo1

compile_demo2: programm_No1.c libprime_sort2.so
	gcc programm_No1.c -L. -lprime_sort2 -lm -Wl,-rpath,. -o compile_demo2

runtime_demo: programm_No2.c
	gcc -o runtime_demo -ldl programm_No2.c -lm

clean:
	rm -f *.so compile_demo1 compile_demo2 runtime_demo

.PHONY: all clean