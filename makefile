OBJECT:=i2d_util.o
OBJECT+=i2d_opt.o

all: clean i2d

i2d: $(OBJECT)
	$(CC) $(CFLAGS) -o $@ i2d.c $^ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean

clean:
	@rm -f *.o
	@rm -f i2d