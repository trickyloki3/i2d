ifeq ($(BUILD), debug)
CFLAGS+=-Di2d_debug -g -Wall
else
CFLAGS+=-O1
endif

LDLIBS+=-ljansson
LDLIBS+=-lm

OBJECT:=i2d_util.o
OBJECT+=i2d_range.o
OBJECT+=i2d_logic.o
OBJECT+=i2d_rbt.o
OBJECT+=i2d_item.o
OBJECT+=i2d_skill.o
OBJECT+=i2d_mob.o
OBJECT+=i2d_produce.o
OBJECT+=i2d_mercenary.o
OBJECT+=i2d_pet.o
OBJECT+=i2d_constant.o
OBJECT+=i2d_db.o
OBJECT+=i2d_json.o
OBJECT+=i2d_script.o
OBJECT+=i2d_print.o
OBJECT+=i2d_data.o


all: clean i2d

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
