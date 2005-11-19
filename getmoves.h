/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: getmoves.h,v 1.1 2005/11/19 08:22:33 bmiller Exp $

#ifndef INCLUDE_getmoves_h
#define INCLUDE_getmoves_h

#include "qtypes.h"

typedef deque<qMove> qMoveList; 
typedef deque<qMove>::iterator qMoveListIterator;

// Returns list of all legally playable moves in a given position
// Note: uses the moveStack to accelerate finding possible moves.
// See the moveStack class for more info.
qMoveList *getPlayableMoves(qPosition   *pos,
			    qMoveStack   movStack,
			    qMoveList   *return_list);

// !!! A very good optimization would be if we found a way to throw out all
// useless moves.  Actually we should keep the first useless wall move we see
// and throw out all others, thus preserving the ability to play a
// "throw-away" move.
// Examining the "offensive" and "defensive" wall strategies used by
// hardquor might be useful here.
qMoveList *pruneUselessMoves(qPosition  *pos,
			     qMoveList  *moveList);


#endif // INCLUDE_getmoves_h
