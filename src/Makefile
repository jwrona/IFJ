################################################################################
# File:					Makefile
# Description:			Super Makefile
# Author:				Fabry Marko		-	xfabry01
#
# Projekt:				IFJ-2012
# Team 34:
#						Duban Michal	-	xduban01
#						Hanselka Vaclav	-	xhanse00
#						Heczkova Petra	-	xheczk04
#						Fabry Marko		-	xfabry01
#						Wrona Jan		-	xwrona00
#
################################################################################


# Source files to be linked together:
SOURCES=ifj12.c

# Header files:
HEADERS=ifj12.h expressions.h htable.h ial.h ilist.h interpret.h parser.h scaner.h stack.h

# Application name:
EXECUTABLE=ifj12

# Application arguments:
args=in

# Valgrind arguments:
#CHECK_ARGS=-q --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes

CHECK_ARGS=-v --tool=memcheck --leak-check=full --track-origins=yes --show-reachable=yes

# Zip archive name:
ZIP_NAME=xfabry01.zip

# Other files to zip:
# (All *.c and *.h files will be added automatically)
ZIP_FILES=Makefile dokumentace.pdf rozdeleni expressions.c htable.c ial.c ilist.c interpret.c parser.c scaner.c stack.c

################################################################################
# DO NOT EDIT BELOW THIS LINE - UNLESS YOU KNOW WHAT YOU ARE DOING !!!
################################################################################

#Compiler:
CC=gcc

# Compiler options:
C_FLAGS=-c -std=c99 -pedantic -W -Wall -Wextra

# Linker options:
LD_FLAGS=-lrt -lm

# Object files from source files:
OBJECTS=$(SOURCES:.c=.o)

################################################################################
# DO NOT EDIT BELOW THIS LINE - UNDER NO CIRCUMSTANCES !!!
################################################################################
all: clean $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LD_FLAGS)

$(OBJECTS): $(SOURCES) $(HEADERS)
	$(CC) $(C_FLAGS) $(SOURCES)

$(SOURCES): $(HEADERS)

run: all
	./$(EXECUTABLE) $(args)

check: all
	valgrind $(CHECK_ARGS) ./$(EXECUTABLE) $(args)

zip: all
	rm -rf $(ZIP_NAME)
	zip $(ZIP_NAME) $(ZIP_FILES) $(SOURCES) $(HEADERS)
	./is_it_ok.sh $(ZIP_NAME) testdir

clean:
	rm -rf $(EXECUTABLE)
	rm -rf *o
