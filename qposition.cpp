/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */



#include "qposition.h"
#include "parameters.h"

IDSTR("$Id: qposition.cpp,v 1.6 2006/07/18 06:55:33 bmiller Exp $");


/****/

// Is this how to have a global that is initialized by the compiler???
const guint8  blankRows[8] = {0,0,0,0,0,0,0,0};
const guint8 *blankCols = blankRows;
const qSquare initialWhitePawnLocation = qSquare(4,0);
const qSquare initialBlackPawnLocation = qSquare(4,8);

const qPosition qInitialPosition =
qPosition(blankRows,                // No walls in any row
	  blankCols,                // No walls in any col
	  initialWhitePawnLocation, // white pawn location
	  initialBlackPawnLocation, // black pawn location
	  10,                       // white walls
	  10);                      // black walls

void qPosition::applyMove
(qPlayer player,
 qMove   move)
{
  if (move.isWallMove())
    {
      if (player.isWhite()) {
	int n = numWhiteWallsLeft();
	if (n == 0)
	  return;
	setWhiteWallsLeft(n-1);
      } else {
	g_assert(p.isBlack());
	int n = numBlackWallsLeft();
	if (n == 0)
	  return;
	setBlackWallsLeft(n-1);
      }
      if (move.wallMoveIsRow())
	row_walls[move.wallRowOrColNo()] |= 1<<(move.wallPosition());
      else
	col_walls[move.wallRowOrColNo()] |= 1<<(move.wallPosition());

      return;
    }
  else // isPawnMove
    {
      if (player.isWhite())
	setWhitePawn(getWhitePawn().applyDirection(move.pawnMoveDirection()));
      else
	setBlackPawn(getBlackPawn().applyDirection(move.pawnMoveDirection()));
      return;
    }
}

bool qPosition::canPutWall
(bool rowOrCol,
guint8 RCNum,
guint8 pos) const
{
  // Is there a wall already in place here?
  guint8 mask;
  if (pos == 0)
    mask = 3;  // 11000000
  else
    { // pos > 0
      if (pos <= 6)
        mask = 7<<(pos-1); // 11100000, 01110000, 00111000, etc.
      else
        {  // pos >= 7
          if (pos > 7)
            { g_assert(pos <= 7); return FALSE; }
          mask = 192; // 00000011
        }
    }
  if (rowOrCol == ROW) {
    if (row_walls[RCNum] & mask)
      return FALSE;

    // Is there a wall across our path?
    if (col_walls[pos] & (1<<RCNum))
      return FALSE;
  } else {
    if (col_walls[RCNum] & mask)
      return FALSE;

    // Is there a wall across our path?
    if (row_walls[pos] & (1<<RCNum))
      return FALSE;
  }
  return TRUE;
}

// There is no theoretical basis for this hashFunc...the idea was to make
// something fast that had a good chance of working sort of well.
// I tried to mix up the bits, shifting the various rows & cols of wall
// positions psuedo-randomly so that the positions that get used most
// frequently would get distributed evenly.
// We should probably do some testing to detect if there are lots of
// collisions!!!
guint16 qPosition::hashFunc
(void) const
{
  guint32 mixer;

  mixer = white_pawn_pos.squareNum | (black_pawn_pos.squareNum<<9);

#define mixedRowNCol(n) ((((guint32)(row_walls[(3*(n))%8]))<<(2*(n))) ^ (((guint32)(col_walls[(5*(n)+2)%8]))<<(2*(n)+7)))
  mixer ^= mixedRowNCol(0) ^
    mixedRowNCol(1) ^
    mixedRowNCol(2) ^
    mixedRowNCol(3) ^
    mixedRowNCol(4) ^
    mixedRowNCol(5) ^
    mixedRowNCol(6) ^
    mixedRowNCol(7);
  mixer ^= (guint32)numwalls;

  // Now superimpose the top 16 bytes on the lower 16 bytes, so the lower
  // 16 bytes are well mixed.
  mixer ^= mixer>>16;
  return (guint16)(mixer%POSITION_HASH_BUCKETS);
}
