CFLAGS= -g -Wall


GENERICS= ../message.o  ../bus-arith.o ../param.o ../option.o ../error.o

../%.o: ../%.c
	(cd ..; make $*.o )

mother: mother.c $(GENERICS)
	$(CC) $(CFLAGS) -I ..  $^ -o $@

clean:
	rm -f mother mother.exe *.o *~

