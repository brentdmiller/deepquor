/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: getmoves.h,v 1.5 2006/07/27 05:59:27 bmiller Exp $

#ifndef INCLUDE_getmoves_h
#define INCLUDE_getmoves_h 1

#include "qtypes.h"
#include "qposition.h"
#include "qmovstack.h"
#include <deque>

// Populates list of all legally playable moves in a given position,
// inserting moves at the end of the list.  Tries to append pawn moves closer
// to the beginning than wall drops.
// Returns listToPopulate on success, NULL on failure
// Note: uses the moveStack to accelerate finding possible moves.
// See the moveStack class for more info.
qMoveList *getPlayableMoves(const qPosition   *pos,
			    qMoveStack        *movStack,
			    qMoveList         *listToPopulate);

// Same as getPlayableMoves, but this doesn't bother to verify the legality
// of returned moves.  Thus, it's guaranteed to return a list including every
// playable move, and possibly some illegal moves.  Any move that is legal is
// is guaranteed to be playable.
qMoveList *getCandidateMoves(const qPosition  *pos,
                             qMoveStack       *movStack,
                             qMoveList        *moveList);



// Populates list of all legally playable pawn moves for given player & pos
// Inserts new moves at end of returnList.
// Returns returnList on success, NULL on failure
qMoveList *getPossiblePawnMoves
(const qPosition *pos,
 qPlayer          player2move,
 qMoveList       *returnList);


// !!! A very good optimization would be if we found a way to throw out all
// useless moves.  Actually we should keep the first useless wall move we see
// and throw out all others, thus preserving the ability to play a
// "throw-away" move.
// Examining the "offensive" and "defensive" wall strategies used by
// hardquor might be useful here.
bool pruneUselessMoves(const qPosition  *pos,
		       qMoveList        *moveList);

#endif // INCLUDE_getmoves_h
