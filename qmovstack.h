/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qmovstack.h,v 1.5 2005/11/19 08:22:33 bmiller Exp $


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
  qMoveStack(qPosition *pos = &qInitialPosition);

  ~qMoveStack();

  /* This func generates each wall move's list of possible wall moves
   * that it blocks.  It can optionally be called after any move (real move,
   * not evaluated move) to generate updated (and hopefully shorter) lists.
   * A small performance boost might come from doing this during the opponent's
   * thinking time.  However, note that any time a move is "taken back," the
   * move table cannot be used if it is newer than the move being taken back.
   * If the move table is newer, then it will have to be regenerated from an
   * older position before stepping back.
   */
  void initWallMoveTable(qPosition*);

  void  pushMove(qPlayer        whoMoved,
		 qMove          move,
		 qPosition     *endPos=NULL); // Optional optimizer

  void  popMove(void);
  qMove peekLastMove(void)   {return moveStack[sp-1].move;};
  qPosition* getPos(void)    {return &(moveStack[sp].startPos);};
  qPosition* getPrevPos(void){return &(moveStack[sp-1].pos);};

  // By setting the posinfo as we build a stack, we can avoid needing to
  // perform hash lookups again on the way back down the stack.
  // We can assume no posInfo records that were in the stack got harvested
  // to free memory because anything under evaluation cannot be cleaned
  // or we'll lose track of any possible cycles in our thought.
  qPositionInfo *getPosInfo(void) { return moveStack[sp].posInfo; }
  void           setPosInfo(qPositionInfo *p) { moveStack[sp].posInfo = p; }

  // Get list of possible wall moves from current location
  const vector<qMove> getPossibleWallMoves();


 private:

  qMoveStackFrame moveStack[MOVESTACKSIZ];
  guint8          sp;

  qWallMoveInfo     allWallMoveArry[256];  // max possible encoding fr/qMove
  qWallMoveInfoList possibleWallMoves;
};


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

typedef qPositionHash<qPosition, qPositionInfo> qPosInfoHash;
enum { flag_WhiteToMove=0x01, flag_BlackToMove=0x02 };

// These funcs set the positionHash in the qPosHash to track which moves are
// currently under evaluation.
// This uses bits 1 & 2 in the qPositionInfo qPositionFlag.
qPositionInfo *pushMove(qMoveStack    *movStack,
			qPositionInfo *posInfo,  // optimizer
			qPosHash      *posHash,  // reqd if posInfo==NULL
			qPlayer        whoMoved, //   From here down same
			qMove          move,     //   as qMoveStack
			qPosition     *pos);

void  popMove(qMoveStack *movStack);

// Returns if a given position is in the stack
bool isInMoveStack(qPositionInfo *posInfo, qPlayer player);

// Returns if this position is in the move stack with white to move
bool isWhiteMoveInStack(qPositionInfo posInfo);

// Returns if this position is in the move stack with black to move
bool isBlackMoveInStack(qPositionInfo posInfo);


};

#endif // INCLUDE_movstack_h
