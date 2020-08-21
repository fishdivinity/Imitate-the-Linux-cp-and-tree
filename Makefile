objects= main.o thread_pool.o dir.o copy.o perrnum.o rrand.o

cp:$(objects)
	cc -o cp $(objects) -pthread
	
$(objects):currency.h

.PHONY:clean
clean:
	-rm $(objects)
