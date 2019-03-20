# C++ compiler to use
CC = gcc

# Compiler flags
CFLAGS = -std=c99

# Add options to generate dependencies
OUTPUT_OPTION=-MMD -MP -o $@

# Check for "debug=true" or "debug=1" flag, modify CFLAGS accordingly
ifneq (,$(filter $(debug),true 1))
   CFLAGS += -Wall -Wextra -g -DDEBUG
else
   CFLAGS += -O3
endif

# Libraries
LIBS = 

# Sources
SRCS = test.c \
       ini-reader.c

# Object files
OBJS = $(SRCS:.c=.o)

# Dependencies
DEPS = $(SRCS:.c=.d)

# Default values, to be overwritten from Makefile.local
INCLUDES = 
LFLAGS = 
MAIN = test

# Read definitions from Makefile.local, if it exists
-include Makefile.local

.PHONY: clean

default: all

# Main
all: $(MAIN)
	@echo Compilation finished.

-include $(DEPS)

# Link object files into executable
$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# Lib only
ini-reader: ini-reader.o
	@echo "Done."

# Compile into object files
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -c $< -o $@

# Clean executable, object files and temporary files
clean:
	$(RM) $(OBJS) $(DEPS) *~ $(MAIN)

