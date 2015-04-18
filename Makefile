CCFlags=-std=gnu99 -Wall -Wpedantic -Wextra `pcre-config --cflags` -g 
LinkFlags=
CSV_GREP_FILES = bin/obj/csvgrep.o bin/obj/csv_tokenizer.o
CSV_CHOP_FILES = bin/obj/csvchop.o bin/obj/csv_tokenizer.o
CSV_PIPE_FILES = bin/obj/csvpipe.o
CSV_UNPIPE_FILES = bin/obj/csvunpipe.o

.PHONY: test clean test-csvgrep test-csvchop

all: bin/csvchop bin/csvgrep bin/csvpipe bin/csvunpipe

bin/csvchop: $(CSV_CHOP_FILES) Makefile
	$(CC) -o $@ $(LinkFlags) $(CSV_CHOP_FILES) 

bin/csvpipe: $(CSV_PIPE_FILES) Makefile
	$(CC) -o $@ $(LinkFlags) $(CSV_PIPE_FILES) 

bin/csvunpipe: $(CSV_UNPIPE_FILES) Makefile
	$(CC) -o $@ $(LinkFlags) $(CSV_UNPIPE_FILES) 

bin/csvgrep: $(CSV_GREP_FILES) Makefile
	$(CC) -o $@ $(LinkFlags) `pcre-config --libs` $(CSV_GREP_FILES) 

bin/obj/%.o: src/%.c bin/obj/ Makefile
	$(CC) -c -o $@ $(CCFlags) $< 

bin/obj/: 
	mkdir -p bin/obj

test: test-csvgrep test-csvchop

test-csvgrep: bin/csvgrep
	cd test && ./runtest.sh csvgrep

test-csvchop: bin/csvchop
	cd test && ./csvchop.sh
	
test-csvpipe: bin/csvpipe
	cd test && ./runtest.sh csvpipe

test-csvunpipe: bin/csvunpipe
	cd test && ./runtest.sh csvunpipe

clean:
	rm -f bin/csv*
	rm -f bin/obj/*.o
