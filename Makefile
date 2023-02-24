#CC = @CC@
CC = gcc
CXX = g++
#VERSION = @VERSION@
#CFLAGS = @CFLAGS@

DEBUGFLAGS = -g -O0 -m32 -DDEBUG

# Note:  -m32 is to allow using Valgrind (which doesn't work on 64-bit exes)
CXXFLAGS = -fpic $(DEBUGFLAGS)

#CXXFLAGS = $(CXXFLAGS) -DMALLOC_CHECK_=1
#CXXFLAGS = -g -mcmodel=medium

NAME = libdeepquor.so

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<


SRC = getmoves.cpp qdijkstra.cpp qmovstack.cpp qposhash.cpp qposinfo.cpp \
	qposition.cpp qsearcher.cpp eval.cpp qcomptree.cpp qtypes.cpp
OBJ = $(addsuffix .o, $(basename $(SRC)))

# And now we begin...
all: deepquor-lib connect

connect.o: connect.c

eval.o:	eval.cpp qtypes.h qposition.h qposinfo.h qposhash.h qmovstack.h parameters.h

getmoves.o: getmoves.cpp getmoves.h qmovstack.h qdijkstra.h

qdijkstra.o: qdijkstra.cpp qdijkstra.h

qmovstack.o: qmovstack.cpp qmovstack.h

qposhash.o: qposhash.cpp qposhash.h parameters.h

qposinfo.o: qposinfo.cpp qposinfo.h

qposition.o: qposition.cpp qposition.h parameters.h

qsearcher.o: qsearcher.cpp qsearcher.h

qtypes.o: qtypes.cpp qtypes.h

qcomptree.o: qcomptree.cpp qcomptree.h

# Header interdependencies
getmoves.h: qtypes.h qposition.h qmovstack.h

qcomptree.h: qtypes.h qposinfo.h parameters.h

qdijkstra.h: qtypes.h qposition.h

qmovstack.h: qtypes.h qposition.h qposhash.h parameters.h

qposhash.h: qtypes.h qposinfo.h qposition.h

qposinfo.h: qtypes.h

qposition.h: qtypes.h

qsearcher.h: qtypes.h qposition.h qposinfo.h qposhash.h qmovstack.h qcomptree.h parameters.h getmoves.h

qposition.h: qtypes.h

#parameters.h:
#
#qtypes.h:
#

connect: connect.o
	$(CC) $< -o connect


deepquor-lib: $(OBJ)
	 $(CXX) -shared $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ) $(NAME)

distclean:
	#rm -f 
