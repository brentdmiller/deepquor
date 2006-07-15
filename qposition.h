/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qposition.h,v 1.8 2006/07/15 05:16:38 bmiller Exp $

#ifndef INCLUDE_qposition_h
#define INCLUDE_qposition_h 1

#include <string.h>
#include "qtypes.h"

/* We'd really like to turn off alignment so positions can be packed into
 * a tight array!!!
 */
#ifdef __GNUC__
#define PACK_DECL(decl) decl __attribute__ ((__packed__))
#else
#error  Error_do_not_know_how_to_force_packed_structs_on_your_compilier
#endif
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
	  row_walls[i] = row_wall_positions ? row_wall_positions[i] : 0;
	  col_walls[i] = col_wall_positions ? col_wall_positions[i] : 0;
	}

      g_assert(white_walls_left <= 15);
      g_assert(black_walls_left <= 15);
      numwalls = (black_walls_left<<4) + white_walls_left;
    };
  qPosition(const qPosition *prev) { *this = *prev; };

  inline bool isWon(qPlayer p) const
    { return((p.isWhite() ? (white_pawn_pos.y()==8) : black_pawn_pos.y()==0)); }
  inline bool isLost(qPlayer p) const
    { return((p.isWhite() ? (black_pawn_pos.y()==0) : white_pawn_pos.y()==8)); }
  inline bool isWhiteWon() const { return (white_pawn_pos.y()==8); }
  inline bool isBlackWon() const { return (black_pawn_pos.y()==8); }

  // Note that we rely on default memberwise copy for qPosition assignment a=b:
  // qPosition(qPosition copy) :
  //  white_pawn_pos(copy.white_pawn_pos), black_pawn_pos(black_pawn_pos);

  inline guint8 numWhiteWallsLeft() const { return (numwalls & 0x0f); };
  inline guint8 numBlackWallsLeft() const { return (numwalls >> 4); };
  inline guint8 numWallsLeft(qPlayer p) const
    { return p.isWhite() ? numWhiteWallsLeft() : numBlackWallsLeft(); };
  inline void setWhiteWallsLeft(guint8 n)
    { g_assert(n < 0x0f); numwalls &= 0xf0; numwalls |= n; };
  inline void setBlackWallsLeft(guint8 n)
    { g_assert(n < 0x0f); numwalls &= 0x0f; numwalls |= n<<4; };
  inline void setNumWallsLeft(qPlayer p, guint8 n)
    { if (p.isWhite()) setWhiteWallsLeft(n); else setBlackWallsLeft(n); };

  inline bool  isBlockedByWall(qSquare fromSq, qDirection dir) const
    { return isBlockedByWall(fromSq.x(), fromSq.y(), dir); };
  inline bool  isBlockedByWall(guint8 x, guint8 y, qDirection dir) const
    {
      switch(dir) {
      case UP:
	return( (y>=8) ? TRUE : (row_walls[y] & (x ? (3<<(x-1)) : 1) ));
      case DOWN:
	return( (y==0) ? TRUE : (row_walls[y] & (x ? (3<<(x-1)) : 1) ));
      case LEFT:
	return( (x==0) ? TRUE : (col_walls[x] & (y ? (3<<(y-1)) : 1) ));
      case RIGHT:
	return( (x>=8) ? TRUE : (col_walls[x] & (y ? (3<<(y-1)) : 1) ));
      }
      g_assert(0);
      return TRUE;
    };
  inline qSquare getPawn(qPlayer p) const
    { return (p.isWhite()?white_pawn_pos:black_pawn_pos); };
  inline qSquare getWhitePawn() const { return white_pawn_pos; };
  inline qSquare getBlackPawn() const { return black_pawn_pos; };
  inline void setPawn(qPlayer p, qSquare s)
    { if (p.isWhite()) white_pawn_pos=s; else black_pawn_pos=s; };
  inline void setWhitePawn(qSquare s) { white_pawn_pos=s; };
  inline void setBlackPawn(qSquare s) { black_pawn_pos=s; };

  // This is slow, but we use it to initialize a fast cached lookup of whether
  // any wall move is possible and also a linked list of all possible wall
  // moves in the qMoveStack class.
  // Use fast lookups of cached data from there instead.
  // This routine does not test if a wall placement is legal (i.e. if it
  // blocks anyone from reaching the goal); it only tests if the wall location
  // is blocked by other walls.
  bool canPutWall(bool rowOrCol, guint8 RCNum, guint8 pos) const;

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
  guint16 hashFunc() const;

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

  // ??? This fails to account for alignment holes, & isn't safe
  inline bool operator== (const qPosition& other) const
  { return (memcmp(this, &other, sizeof(*this)) == 0); }

 private:
  PACK_DECL(guint8 row_walls[8]);
  PACK_DECL(guint8 col_walls[8]);
  PACK_DECL(qSquare white_pawn_pos); /* guint8 */
  PACK_DECL(qSquare black_pawn_pos); /* guint8 */
  PACK_DECL(guint8 numwalls); /* low 4 bits white, high 4 bits black */
};

/* Position at the beginning of the game */
extern const qPosition qInitialPosition;

/* Probably not useful to anyone... */
extern const qSquare initialWhitePawnLocation;
extern const qSquare initialBlackPawnLocation;


#endif // INCLUDE_qposition_h
