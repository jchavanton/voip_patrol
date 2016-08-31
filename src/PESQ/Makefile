# CC = gcc
# SOURCES = $(wildcard *.c)
# # dsp.c pesqdsp.c pesqio.c pesqmain.c  pesqmod.c
# # dsp.h pesq.h pesqpar.h
# all:
# 	$(CC) 

CC = gcc
CC_FLAGS = -w

# File names
EXEC = run_pesq
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) -lm

# To obtain object files
%.o: %.c
	$(CC) -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)
