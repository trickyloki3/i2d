CFLAGS+=$(shell pkg-config jansson --cflags)
LDLIBS+=$(shell pkg-config jansson --libs)

OBJECT:=i2d_util.o
OBJECT+=i2d_opt.o
OBJECT+=i2d_item.o
OBJECT+=i2d_rbt.o
OBJECT+=i2d_script.o
OBJECT+=i2d_json.o
OBJECT+=i2d_range.o
OBJECT+=i2d_logic.o

all: clean i2d i2d_test

i2d: $(OBJECT)
	$(CC) $(CFLAGS) -o $@ i2d.c $^ $(LDFLAGS) $(LDLIBS)

i2d_test: $(OBJECT)
	$(CC) $(CFLAGS) -o $@ i2d_test.c $^ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean

clean:
	@rm -f *.o
	@rm -f i2d
	@rm -f i2d_test
