/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */



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
 * frame will contain the position at that frame, a move, and glist of
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



/* Keep a cache of possible moves and associated scores??? */
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
  typedef qPositionHash<qPosition, qPositionInfo> qPosInfoHash;

  // Initialize from the normal starting open board
  // Calls initWallMoveTable() for you.
  qMoveStack(qPosInfoHash *hash);

  // Initialize from an existing position
  // Binds to a position hash
  // Calls initWallMoveTable() for you.
  qMoveStack(qPosInfoHash *hash, qPosition *pos);

  // If bound to a position hash, all positions in the move stack will 
  // be flagged in the position hash, making it efficient to check if a
  // new position has already been reached in the stack.
  // This uses bits 1 & 2 in the 

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

  void  pushMove(qPlayer    whoMoved,
		 qMove      move,
		 qPosition *endPos); // Optional optimizer

  void  popMove(void);
  qMove peekLastMove(void)   {return moveStack[sp-1].move;};
  qPosition* getPos(void)    {return &(moveStack[sp].startPos);};
  qPosition* getPrevPos(void){return &(moveStack[sp-1].pos);};

  // Get list of possible wall moves from current location
  const vector<qMove> getPossibleWallMoves();

  // Returns if a given position is in the stack
  // This is a fast check--since revisiting positions is rare, it should
  // be used before checking for which playerToMove is in the stack
  bool isInMoveStack(qPositionInfo posInfo);

  bool isInMoveStack(qPositionInfo posInfo, qPlayer player);

  // Returns if this position is in the move stack with white to move
  bool isWhiteMoveInStack(qPositionInfo posInfo);

  // Returns if this position is in the move stack with black to move
  bool isBlackMoveInStack(qPositionInfo posInfo);


 private:

  enum { flag_WhiteToMove=0x01, flag_BlackToMove=0x02 };

  // These funcs flag which moves are under evaluation
  void setMoveStack(qPositionInfo posInfo, qPlayer playerToMove) {
    if (playerToMove.isWhite())
      posInfo.setPositionFlagBits(qPositionInfo::flag_WhiteToMove);
    else
      posInfo.setPositionFlagBits(qPositionInfo::flag_BlackToMove);
  };
  void clearMoveStack(qPositionInfo posInfo, qPlayer playerToMove) {
    if (playerToMove.isWhite())
      posInfo.clearPositionFlagBits(qPositionInfo::flag_WhiteToMove);
    else
      posInfo.clearPositionFlagBits(qPositionInfo::flag_BlackToMove);
  };

  qMoveStackFrame moveStack[MOVESTACKSIZ];
  guint8          sp;

  qWallMoveInfo     allWallMoveArry[128];  // Plently of moves
  qWallMoveInfoList possibleWallMoves;

  // ??? There's probably a more independent way to track which moves are
  // in the  in the stack, but we have evaluations available, so we'll
  // store some bits in the evals for looking up which moves are in the
  // stack of a depth search.  If there are problems, this can be replaced
  // with another data type.
  // !!! Assumes the evals for every move in the stack will remain cached
  // in the positionEvalsHash.  One safeguard would be to block removing
  // any position with "public" flags set in the posInfo from the hash;
  // another would be to just use our own data type here.
  qPosInfoHash *positionEvalsHash;
}

#endif // INCLUDE_movstack_h
