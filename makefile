CFLAGS+=$(shell pkg-config jansson --cflags)
LDLIBS+=$(shell pkg-config jansson --libs)

OBJECT:=i2d_util.o
OBJECT+=i2d_opt.o
OBJECT+=i2d_item.o
OBJECT+=i2d_rbt.o
OBJECT+=i2d_script.o
OBJECT+=i2d_json.o

all: clean i2d

debug:
	$(MAKE) clean
	$(MAKE) i2d CFLAGS=-Di2d_debug\ -g\ -Wall
	valgrind --leak-check=yes ./i2d -j i2d.json -i ~/Desktop/rathena/db/pre-re/item_db.txt

i2d: $(OBJECT)
	$(CC) $(CFLAGS) -o $@ i2d.c $^ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean

clean:
	@rm -f *.o
	@rm -f i2d
