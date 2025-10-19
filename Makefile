LIBRARIES := -lm -lGL -lGLU -lglut -ljpeg

.PHONY: all clean

all: glut-starter

glut-starter: glut-starter.c
	gcc -o glut-starter glut-starter.c $(LIBRARIES)

clean:
	rm -f glut-starter

