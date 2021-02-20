#!/bin/sh

#CFLAGS="-g -mcmodel=medium"
#CFLAGS="-g -fpic -DMALLOC_CHECK_=1"
DEBUGFLAGS="-g -O0 -m32 -DDEBUG"
CFLAGS="-fpic $DEBUGFLAGS"


#g++ -I.. ./testmovstack.cpp -L.. -Bdynamic -ldeepquor -o movstack
#g++ -g -c ../qmovstack.cpp
#g++ -g -c ../qposhash.cpp
#g++ -g -c ../qposinfo.cpp
#g++ -g -c ../qposition.cpp
#g++ -g -c ../qtypes.cpp
g++ $CFLAGS -c -I.. testmovstack.cpp
g++ $CFLAGS -o movstack testmovstack.o ../qposinfo.o ../qmovstack.o ../qposhash.o ../qposition.o ../qtypes.o

g++ $CFLAGS -c -I.. testthink.cpp
g++ $CFLAGS -o think testthink.o ../qcomptree.o ../qsearcher.o ../eval.o ../qdijkstra.o ../getmoves.o ../qposinfo.o ../qmovstack.o ../qposhash.o ../qposition.o ../qtypes.o

g++ $CFLAGS -c -I.. quorshell.cpp
g++ $CFLAGS -o quorshell quorshell.o ../qcomptree.o ../qsearcher.o ../eval.o ../qdijkstra.o ../getmoves.o ../qposinfo.o ../qmovstack.o ../qposhash.o ../qposition.o ../qtypes.o
# -Wl,--stack,128000000

#g++ $CFLAGS -c -I.. t.cpp
#g++ $CFLAGS -o a.out t.o ../qcomptree.o ../qsearcher.o ../eval.o ../qdijkstra.o ../getmoves.o ../qposinfo.o ../qmovstack.o ../qposhash.o ../qposition.o ../qtypes.o
