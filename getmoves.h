/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: getmoves.h,v 1.2 2006/06/24 00:24:05 bmiller Exp $

#ifndef INCLUDE_getmoves_h
#define INCLUDE_getmoves_h

#include "qtypes.h"
#include <deque>

typedef deque<qMove> qMoveList; 
typedef deque<qMove>::iterator qMoveListIterator;

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
(qPosition pos,
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

static const qMove moveUp    = qMove(UP);
static const qMove moveDown  = qMove(DOWN);
static const qMove moveLeft  = qMove(LEFT);
static const qMove moveRight = qMove(RIGHT);

static const qMove moveUpUp       = qMove(UP+UP);
static const qMove moveDownDown   = qMove(DOWN+DOWN);
static const qMove moveLeftLeft   = qMove(LEFT+LEFT);
static const qMove moveRightRight = qMove(RIGHT+RIGHT);

static const qMove moveUL = qMove(UP+LEFT);
static const qMove moveUR = qMove(UP+RIGHT);
static const qMove moveDL = qMove(DOWN+LEFT);
static const qMove moveDR = qMove(DOWN+RIGHT);

#endif // INCLUDE_getmoves_h
