OBJECT:=i2d_util.o
OBJECT+=i2d_opt.o
OBJECT+=i2d_item.o
OBJECT+=i2d_rbt.o
OBJECT+=i2d_script.o

all: clean i2d

i2d: $(OBJECT)
	$(CC) $(CFLAGS) -o $@ i2d.c $^ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean

clean:
	@rm -f *.o
	@rm -f i2d
