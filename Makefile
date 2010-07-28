CC = gcc
CC_PARAMS = -O3 -Wall -Werror -fopenmp -c
C_SOURCES = graymap.c graymap_alleg.c paledit.c mandelbrot.c
OBJ = $(C_SOURCES:%.c=%.o)
LIBS = `allegro-config --libs` -lm -lgomp

mandelbrot: $(OBJ)
	$(CC) $(LIBS) -o mandelbrot $(OBJ)

%.o:%.c
	$(CC) $(CC_PARAMS) -o $@ $<

doc:
	if [ ! -d doc ]; then mkdir doc; fi
	doxygen Doxyfile

clean:
	rm -r doc
	rm $(OBJ)
	rm mandelbrot
