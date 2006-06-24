/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qmovstack.h,v 1.6 2006/06/24 00:24:05 bmiller Exp $


#ifndef INCLUDE_movstack_h
#define INCLUDE_movstack_h

#include "qtypes.h"
#include "qposition.h"
#include "qposhash.h"
#include "qposinfo.h"
#include "parameters.h"
#include <vector>


/* Idea:
 * We'll construct a large array enumerating every possible wall location.
 * Then we'll use prev/next pointers in the elts of that array to link all
 * elts arbitrarily into a large doubly-linked list.
 * During move evaluation, we'll have a stack of moves, and each stack
 * frame will contain the position at that frame, a move, and list of
 * other possible wall moves that were eliminated by that move.
 * When we add a new move to the stack, we look up the wall positions
 * eliminated by that move, snip them from the possible wall move list
 * and insert them into that frame's "eliminated wall move list."
 * When we work our way back up the move stack and undo that move, we
 * insert the frame's eliminated wall move list" back into the possible wall
 * move list.
 * Note that as the game progresses, we can boost performance slightly by
 * regenerating the list of each wall move's "blocked" moves.  This can be
 * done by re-initializing the move stack from the current position.
 *
 * Further optimization: ???
 * after we've created an initial lookahead tree (qcomptree), maintaining
 * the list of possible wall moves might not be worthwhile as we walk up and
 * down the lookahead tree.  The lookahead tree already has possible moves
 * cached for previously visited positions.  Perhaps we should have the
 * ability to "bind" and "unbind" calculation of wall moves, so we can
 * turn off wall move list maintenance while walking up and down the
 * lookahead tree.  When we reach and end node of the tree and want to
 * start evaluating new territory, we'd call initWallMoveTable() for the
 * current position, turn on the wall list functionality, and proceed.
 *
 * I suspect that, except for very deep positions, it's faster to just
 * maintain the wall list.  Ballpark estimate figures:
 *
 * operations required to get list of possible wall moves for a position:
 *   128 (maybe this can be optimized)
 * 
 * operations required to update wall list when a move is made:
 *   4 x 2 (x2 because we have to remove the moves and then replace them later)
 *
 * So a back-of the envelope estimate is that we'd have to walk about 16
 * plies from a pre-computed lookahead tree before turning off wall
 * maintenance was worthwhile.  16 plies is a lot!
 */
typedef struct _wallMoveInfo {
  qMove move;            // Which wall move is this? (0x0-0x7f)
  bool possible;         // Is this move currently possible?

  // List of which moves get eliminated by this move
  class qWallMoveInfoList *eliminates;

  struct _wallMoveInfo *prev;
  struct _wallMoveInfo *next;
} qWallMoveInfo;

class qWallMoveInfoList {
 public:
  qWallMoveInfoList(); // Constructor
  void           push(qWallMoveInfo*);
  void           pop(qWallMoveInfo*);

  struct _wallMoveInfo *pop();

  qWallMoveInfo *getHead(void)  { return head; };
  qWallMoveInfo *getTail(void)  { return tail; };
  void           clearList(void){ head = tail = NULL; };

 private:
  qWallMoveInfo *head;
  qWallMoveInfo *tail;
};



typedef struct _qMoveStackFrame {
  qPosition         resultingPos;
  qMove             move;
  qPlayer           playerMoved;

  qWallMoveInfoList wallMovesBlockedByMove;

  // !!! See comment in class qMoveStack:
  qPositionInfo    *posInfo;    // Remember this to avoid extra lookups
} qMoveStackFrame;

class qMoveStack {
 public:
  // Initialize from an existing position
  // Calls initWallMoveTable() for you.
  qMoveStack(const qPosition* pos = &qInitialPosition,
	     qPlayer player2move  = WhitePlayer);

  ~qMoveStack();

  /* This func generates each wall move's list of possible wall moves
   * that it blocks.  It can optionally be called after any move (real move,
   * not evaluated move) to generate updated (and hopefully shorter) lists.
   * A small performance boost might come from doing this during the opponent's
   * thinking time.  However, note that any time a move is "taken back," the
   * move table would become useless if it was newer than move being taken
   * back.  In that event, the wallMoveTable would have to be regenerated
   * from the "old" position.
   */
  void initWallMoveTable(void);

  qPlayer getPlayer2Move(void);
  void  pushMove(qPlayer        whoMoved,
		 qMove          move,
		 qPosition     *endPos=NULL); // Optional optimizer

  void  popMove(void);
  qMove peekLastMove(void)   {return moveStack[sp].move;};
  qPosition* getPos(void)    {return &(moveStack[sp].resultingPos);};
  qPosition* getPrevPos(void)
    {return (sp > 0) ? &(moveStack[sp-1].resultingPos) : NULL;};

  // By setting the posinfo as we build a stack, we can avoid needing to
  // perform hash lookups again on the way back down the stack.
  // We can assume no posInfo records that were in the stack got harvested
  // to free memory because anything under evaluation cannot be cleaned
  // or we'll lose track of any possible cycles in our thought.
  qPositionInfo *getPosInfo(void) { return moveStack[sp].posInfo; }
  void           setPosInfo(qPositionInfo *p) { moveStack[sp].posInfo = p; }

  // Get list of possible wall moves from current location, appending them to
  // the back of the passed-in list.  Return true on success, false otherwise.
  bool getPossibleWallMoves(qMoveList *moveList);


 private:

  qMoveStackFrame moveStack[MOVESTACKSIZ];
  guint8          sp;

  qWallMoveInfo     allWallMoveArry[256];  // max possible encoding fr/qMove
  qWallMoveInfoList possibleWallMoves;
};



/***************************************************
 * Utility wrapper funcs                           *
 *                                                 *
 * These functions "wrap" the qMoveStack class and *
 * add useful functionality.                       *
 ***************************************************/

/* Note: because we want to use the "possible wall move" optimization,
 * we need to record all moves in our stack--we can't keep actual moves
 * in one stack and "contemplating" moves in another.
 * Thus, there is no point in creating a derived class of qMoveStack
 * that flags which moves are under consideration and allows querying
 * if a given move is in the thought sequence.  Rather, all parts of
 * the AI player will share one stack, and will use wrapper funcs when
 * contemplating moves and flagging them as in the thought sequence, and
 * will directly push moves in the qMoveStack (without the wrapper funcs)
 * when making a real move.
 */

enum { flag_WhiteToMove=0x01, flag_BlackToMove=0x02 };

// These funcs set the positionHash in the qPosHash to track which moves are
// currently under evaluation.
// This uses bits 1 & 2 in the qPositionInfo qPositionFlag.
// MT NOTE:  depending on the posHash (& posInfo) for tagging which
// positions are in the stack is convenient, but prevents multiple
// qMoveStacks from operating on one posHash.  We should implement
// a data type as part of the qMoveStack for checking if moves are in
// the stack.  ???
qPositionInfo *pushMove(qMoveStack        *movStack,
			qPositionInfo     *posInfo,  // optimizer
			qPositionInfoHash *posHash,  // reqd if posInfo==NULL
			qPlayer            whoMoved, //   From here down same
			qMove              move,     //   as qMoveStack
			qPosition         *pos);

void  popMove(qMoveStack *movStack);

// Returns if a given position is in the stack
bool private_isInMoveStack(qPositionInfo *posInfo,
			   qPlayer        player);
inline bool isInMoveStack
(qMoveStack    *movStack,
 qPositionInfo *posInfo,
 qPlayer        player)
{
  return private_isInMoveStack(posInfo, player);
};

// Returns if this position is in the move stack with white to move
bool private_isWhiteMoveInStack(qPositionInfo posInfo);
inline bool isWhiteMoveInStack
(qMoveStack *moveStack,
 qPositionInfo posInfo)
{
  return private_isWhiteMoveInStack(posInfo);
};

// Returns if this position is in the move stack with black to move
bool private_isBlackMoveInStack(qPositionInfo posInfo);
inline bool isBlackMoveInStack
(qMoveStack *moveStack,
 qPositionInfo posInfo)
{
  return private_isBlackMoveInStack(posInfo);
};

#endif // INCLUDE_movstack_h
