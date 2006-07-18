/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qtypes.h,v 1.7 2006/07/18 06:55:33 bmiller Exp $

#ifndef INCLUDE_qtypes_h
#define INCLUDE_qtypes_h 1

#define IDSTR(msg) \
static /**/const char *const rcsid[] = { (const char *)rcsid, "\100(#)" msg }


// Hack since my glib headers didn't seem to be installed correctly!!!
#define STDINT_GLIB_HACK 1
#ifdef STDINT_GLIB_HACK

#include <stdint.h>
typedef int8_t  gint8;
typedef uint8_t guint8;
typedef int16_t gint16;
typedef uint16_t guint16;
typedef int32_t gint32;
typedef uint32_t guint32;
#define g_assert assert
#ifndef DEBUG
#define NDEBUG
#endif
#include <assert.h>

#else
#include <glib.h>
#endif

#ifndef NULL
#define NULL 0
#endif



/* Basic global types and values for Brent's quoridor prog */

/* The idea here is to choose values that can be added together to form
 * relative moves from any square to any other
 */
typedef gint8 qDirection;
#define  LEFT  (qDirection(-1))
#define  RIGHT (qDirection(1))
#define  DOWN  (qDirection(-9))
#define  UP    (qDirection(9))

#undef  FALSE
#define FALSE 0
#undef  TRUE
#define TRUE  1

/* Identify a particular square on the board
 * a qSquare is a guint8 value with a few possible operators.
 */
class qSquare {
 public:
  guint8 squareNum;
  enum { maxSquareNum = 80, undefSquareNum };

  // SQUARE_VAL macro included to assist defining qInitialPosition in qpos.cpp
#define SQUARE_VAL(x,y) ((x)+9*(y))
  qSquare(guint8 x, guint8 y)
    {
      assert((x<=8) && (y<=8));
      squareNum = SQUARE_VAL(x,y);
    }

  qSquare() {
      squareNum = qSquare::undefSquareNum;
  }

  qSquare(guint8 squareId)
    {
      assert(squareId <= maxSquareNum);
      squareNum = squareId;
    }

  // No need for destructor???

  // OPERATORS:
  guint8 x() const { return squareNum%9; }
  guint8 y() const { return squareNum/9; }

  /* Rather than do this, we'll make the squareNum public & modifiable.
  guint8 getSquareId() const
    {
      assert(squareNum <= 80);
      return(squareNum);
    }
  */

  qSquare applyDirection(qDirection vector)
    { return qSquare(squareNum + vector); }
};

/* Allow bool parameters to specify cols versus rows */
typedef enum { COL=0, ROW=1 } RowOrCol;

/* Identify a particular player */
class qPlayer {
 private:
  gint8 playerId;

  /* First some basic types */
 public:
  enum { WhitePlayer=0, BlackPlayer=1, NoPlayer=-1, OtherNoPlayer=2 };

  // Construct with qPlayer::WhitePlayer or qPlayer::BlackPlayer
  qPlayer(gint8 id = NoPlayer) { playerId = id; };

  qPlayer    otherPlayer() const { return qPlayer(1-playerId); };

  // Change the current instance
  void    changePlayer()  { playerId = 1-playerId; }

  bool isWhite()      const { return(playerId == WhitePlayer); } 
  bool isBlack()      const { return(playerId == BlackPlayer); }
  gint8 getPlayerId() const { return playerId;  }
  gint8 getOtherPlayerId() const { return 1-playerId; }
};


/* Identify a possible move (either pawn move or a wall placement) */
class qMove {
 private:
  /* The move is encoded as follows.
   *
   * Setting the low bit indicates a wall drop; unset indicates a pawn move
   *
   * For a pawn move, (signed)move>>1 gives the qDirection of the move
   *
   * For a wall move, the following indicates the drop position:
   *  (0x02 & mv)==TRUE  - place wall at row
   *  (0x02 & mv)==FALSE - place wall at column
   *  (0x1f & mv)>>2     - indicates row/col in which to lay wall
   *  (mv>>5) - indicates at which position within row/col to lay wall
   */
  guint8 move;

 public:
  // qMove -  Constructor for a wall placement
  qMove(bool RowOrCol, guint8 rowColNo, guint8 posNo)
    { move = (posNo<<5)|(rowColNo<<3)|(RowOrCol); };

  // Constructor for a pawn move
  qMove(gint8 deltaX, gint8 deltaY)
    {
      assert((-3 < deltaX) && (deltaX < 3) && (-3 < deltaY) && (deltaY < 3));
      move = (deltaX+9*deltaY)<<1;
    };

  qMove(qDirection d)    { move = (d<<1);     };

  // Constructor using a previously encoded move
  qMove(guint8 mv) { move = mv; };

  // Constructor for a "NULL" move;
  qMove() { move = 0; }

  // Members for accessing wall moves
  inline bool isWallMove()      const { return   move&0x01;      };
  inline bool   wallMoveIsRow() const { return   move&0x02;      };
  inline bool   wallMoveIsCol() const { return !(move&0x02);     };
  inline guint8 wallRowOrColNo()const { return   (move&0x1f)>>2; };
  inline guint8 wallPosition()  const { return   move>>5;;       };

  // Members for accessing pawn moves
  inline bool     isPawnMove()  const { return !(move&0x01);             };
  inline qDirection pawnMoveDirection()
                                const { return  (((qDirection)move)>>1); };

  // Gets the binary representation of a move (in one byte)
  inline guint8 getEncoding(void) const { return move; };

  // False for moves that were constructed but not initialized, else true
  bool   exists(void) const { return move; };
};


/********************
 * USEFUL CONSTANTS *
 ********************/
extern const qPlayer qPlayer_white;
extern const qPlayer qPlayer_black;

// Here's basically an enumeration of all possible pawn moves
extern const qMove moveUp;
extern const qMove moveDown;
extern const qMove moveLeft;
extern const qMove moveRight;

extern const qMove moveUpUp;
extern const qMove moveDownDown;
extern const qMove moveLeftLeft;
extern const qMove moveRightRight;

extern const qMove moveUL;
extern const qMove moveUR;
extern const qMove moveDL;
extern const qMove moveDR;

extern const qMove moveNull;

/* Notes:
 * For hashed positions, we should generally give preference to keeping
 * positions that required greater computation to derive.
 * Perhaps it is a good idea to count the number of computed positions
 * that contributed.  For example, a directly computed position would score
 * one accumulated computed position.
 * A position that was computed from 82 neighboring positions, 81 of which
 * were directly computed and 1 of which was computed from 30 directly-computed
 * neighbors, would score 111 computations.
 */
#endif // INCLUDE_qtypes_h
