/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: getmoves.h,v 1.3 2006/07/09 06:37:38 bmiller Exp $

#ifndef INCLUDE_getmoves_h
#define INCLUDE_getmoves_h 1

#include "qtypes.h"
#include "qposition.h"
#include "qmovstack.h"
#include <deque>

// Populates list of all legally playable moves in a given position,
// inserting moves at the end of the list.
// Returns listToPopulate on success, NULL on failure
// Note: uses the moveStack to accelerate finding possible moves.
// See the moveStack class for more info.
qMoveList *getPlayableMoves(qPosition   *pos,
			    qMoveStack  *movStack,
			    qMoveList   *listToPopulate);


// Populates list of all legally playable pawn moves for given player & pos
// Inserts new moves at end of returnList.
// Returns returnList on success, NULL on failure
qMoveList *getPossiblePawnMoves
(qPosition *pos,
 qPlayer player2move,
 qMoveList *returnList);


// !!! A very good optimization would be if we found a way to throw out all
// useless moves.  Actually we should keep the first useless wall move we see
// and throw out all others, thus preserving the ability to play a
// "throw-away" move.
// Examining the "offensive" and "defensive" wall strategies used by
// hardquor might be useful here.
bool pruneUselessMoves(qPosition  *pos,
		       qMoveList  *moveList);

#endif // INCLUDE_getmoves_h
