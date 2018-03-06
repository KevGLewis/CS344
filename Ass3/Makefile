#########################################################
# File: Makefile
# Description: A more robust makefile for CS162.
# You need to edit the variables under "USER SECTION".
# To execute the makefile on unix-y systems, type 'make'.
# If that doesn't work, 'make -f makefile' might be useful.
#########################################################

# #'s are line-comments 

# CXX is a standard variable name used for the compiler. 
CXX = gcc

# CXXFLAGS is a standard variable name for compile flags.
# -std=c++0x specifies to use a certain language version.
CXXFLAGS = -std=c99

# -Wall turns on all warnings
CXXFLAGS += -Wall

# -pedantic-errors strictly enforces the standard
CXXFLAGS += -pedantic-errors

# -g turns on debug information 
CXXFLAGS += -g

####################
### USER SECTION ###
####################

# SRCS is a standard name for the source files. 
# Edit as needed.
SRC1 = smallsh.c
SRC2 = processArgs.c
SRC3 = 
SRC4 = 
SRC5 = 
SRC6 = 
SRCS = ${SRC1} ${SRC2} ${SRC3} ${SRC4} ${SRC5} ${SRC6}

# HEADERS is a standard name for the header files. 
# Edit these as needed.
HEADER1 = processArgs.h
HEADER2 = 
HEADER3 = 
HEADER5 = 
HEADER6 = 
HEADERS = ${HEADER1} ${HEADER2} ${HEADER3} ${HEADER4} ${HEADER5}

# These will be your executable names. 
# Edit as needed.
PROG1 = smallsh
PROG2 = 
PROG3 = 
PROGS = ${PROG1} ${PROG2} ${PROG3}


#####################
### BUILD SECTION ###
#####################

# Typing 'make' in terminal calls the first build availible.
# In this case, default.
default:
	${CXX} ${SRCS} ${HEADERS} -o ${PROG1}

# Typing 'make all' in terminal calls this build.
all:
	${CXX} ${CXXFLAGS} ${SRCS} ${HEADERS} -o ${PROG1}


# Typing 'make clean' calls this build.
# It's designed to clean up the folder.
# Be careful with this, edit as needed.
clean: 
	rm -f ${PROGS} *.o *~

