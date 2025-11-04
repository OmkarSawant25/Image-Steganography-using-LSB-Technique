stego = $(patsubst %.c, %.o, $(wildcard *.c))
stegnography : $(stego)
	gcc -o $@ $^
clean :
	rm *.out *.o
