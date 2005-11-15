/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#ifndef INCLUDE_qposition_h
#define INCLUDE_qposition_h

#include "qtypes.h"

/* We'd really like to turn off alignment so positions can be packed into
 * a tight array!!!
 */
class qPosition {
 public:

  qPosition
    (const guint8 *row_wall_positions,
     const guint8 *col_wall_positions,
     const qSquare white_pawn_location,
     const qSquare black_pawn_location,
     const guint8 white_walls_left,
     const guint8 black_walls_left)
    : white_pawn_pos(white_pawn_location), black_pawn_pos(black_pawn_location)
    {
      int i;
      for (i=7; i>=0; i--)
	{
	  row_walls[i] = row_wall_positions[i];
	  col_walls[i] = col_wall_positions[i];
	}

      g_assert(white_walls_left <= 15);
      g_assert(black_walls_left <= 15);
      numwalls = black_walls_left<<4 + white_walls_left;
    };

  // Note that we rely on default memberwise copy for qPosition assignment a=b:
  // qPosition(qPosition copy) :
  //  white_pawn_pos(copy.white_pawn_pos), black_pawn_pos(black_pawn_pos);

  inline guint8 numWhiteWallsLeft() { return (numwalls & 0x15); };
  inline guint8 numBlackWallsLeft() { return (numwalls >> 4); };
  inline guint8 numWallsLeft(qPlayer p)
    { return p.isWhite() ? numWhiteWallsLeft() : numBlackWallsLeft(); };
  inline bool  isBlockedByWall(qSquare fromSq, qDirection dir)
    { return isBlockedByWall(fromSq.x(), fromSq.y(), dir); };
  inline bool  isBlockedByWall(guint8 x, guint8 y, qDirection dir)
    {
      switch(dir) {
      case UP:
	return( (y>=8) ? FALSE : (row_walls[y] & (x?((1<<x)|(1<<(x-1))):1) ));
      case DOWN:
	return( (y==0) ? FALSE : (row_walls[y] & (x?((1<<x)|(1<<(x-1))):1) ));
      case LEFT:
	return( (x==0) ? FALSE : (col_walls[x] & (y?((1<<y)|(1<<(y-1))):1) ));
      case RIGHT:
	return( (x>=8) ? FALSE : (col_walls[x] & (y?((1<<y)|(1<<(y-1))):1) ));
      }
      g_assert(0);
      return FALSE;
    };
  inline qSquare getPawn(qPlayer p)
    { return (p.isWhite()?white_pawn_pos:black_pawn_pos); };
  inline qSquare getWhitePawn() { return white_pawn_pos; };
  inline qSquare getBlackPawn() { return black_pawn_pos; };
  inline void setPawn(qPlayer p, qSquare s)
    { if (p.isWhite()) white_pawn_pos=s; else black_pawn_pos=s; };
  inline void setWhitePawn(qSquare s) { white_pawn_pos=s; };
  inline void setBlackPawn(qSquare s) { black_pawn_pos=s; };

  // This is slow, but we use it to initialize a fast cached lookup of whether
  // any wall move is possible and also a linked list of all possible wall
  // moves in the qMoveStack class.
  // Use fast lookups of cached data from there instead.
  bool canPutWall(bool, guint8, guint8);

  void applyMove(qPlayer, qMove);

  /* Hashing Function to provide to qPositionHash
   * Since a qPosition knows the layout of bits in its own privates,
   * we can write a decent hash func making use of the most active
   * bits that will be faster than a generic hash algorithm.  Maybe.
   * We'll try our own hash func and compare it to a generic one and
   * see which works better, if I ever happen to get to performance
   * testing.  I'll leave this comment triple-hooked until someone
   * does the performance testing and indicates the results here.  ???
   */
  guint16 hashFunc();

#if 0 /* Don't need any of these (yet) */
  /* None of these would validate the legality of a move */
  inline void applyPawnMove(qPlayer, qDirection);
  inline void putWall(bool, guint8, guint8);
  inline void putWall(qMove);

  bool qPosition::wallAt(bool HorV, guint8 row, guint8 x)
  { return (HorV == ROW) ? };???
  bool qPosition::wallAt(qMove)
    {};???
#endif

 private:
  guint8 row_walls[8];
  guint8 col_walls[8];
  qSquare /* guint8 */ white_pawn_pos;
  qSquare /* guint8 */ black_pawn_pos;
  guint8 numwalls; /* low 4 bits white, high 4 bits black */
};

/* Position at the beginning of the game */
extern const qPosition initialPosition;

#endif // INCLUDE_qposition_h
