# @file Makefile
# @author Skuratovich Aliaksandr <xskura01@fit.vutbr.cz>

.PHONY : all
.PHONY : clean
.PHONY : zip

ZIPNAME=xskura01
TARGET=ifj21

all: $(TARGET)

$(TARGET): src/*.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(TARGET) $(ZIPNAME)

zip:
	zip $(ZIPNAME) src/* Makefile rozdeleni rozsireni doc/dokumentace.pdf

