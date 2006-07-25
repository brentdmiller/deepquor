/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qmovstack.h,v 1.11 2006/07/25 22:29:33 bmiller Exp $


#ifndef INCLUDE_movstack_h
#define INCLUDE_movstack_h 1

#include "qtypes.h"
#include "qposition.h"
#include "qposhash.h"
#include "parameters.h"
#include <deque>
#include <list>


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
typedef struct _wallMoveInfo qWallMoveInfo;

class qWallMoveInfoList {
 public:
  qWallMoveInfoList(); // Constructor
  void           push(qWallMoveInfo*);
  void           pop(qWallMoveInfo*);

  struct _wallMoveInfo *pop();

  qWallMoveInfo *getHead(void)  const { return head; };
  qWallMoveInfo *getTail(void)  const { return tail; };
  void           clearList(void){ head = tail = NULL; DEBUG_CODE(numElts=0;) };

 private:
  qWallMoveInfo *head;
  qWallMoveInfo *tail;
  DEBUG_CODE(int numElts;)
  DEBUG_CODE(void verifyEltCount();)
};

struct _wallMoveInfo {
  qMove move;            // Which wall move is this? (0x0-0x7f)
  bool possible;         // Is this move currently possible?

  // List of which moves get eliminated by this move
  list<qWallMoveInfo*> eliminates;

  // data needed for holding this instance in qWallMoveInfoLists
  qWallMoveInfo *prev;
  qWallMoveInfo *next;
};


typedef std::deque<qMove> qMoveList; 
typedef std::deque<qMove>::iterator qMoveListIterator;
typedef std::deque<qMove>::reverse_iterator qMoveListReverseIterator;


typedef struct _qMoveStackFrame {
  _qMoveStackFrame() : resultingPos(NULL, NULL, qSquare(0), qSquare(0),0,0) { }
  qPosition         resultingPos;
  qMove             move;
  qPlayer           playerMoved;

  qWallMoveInfoList wallMovesBlockedByMove;

  // !!! See comment in class qMoveStack:
  qPositionInfo    *posInfo; // Store qPositionInfo to avoid extra lookups
} qMoveStackFrame;

class qMoveStack {
 public:
  // Initialize from an existing position
  // Calls initWallMoveTable() for you.
  qMoveStack(const qPosition* pos = &qInitialPosition,
	     qPlayer player2move  = qPlayer_white);

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

  void  pushMove(qPlayer        whoMoved,
		 qMove          move,
		 qPosition     *endPos =NULL); // Optional optimizer
  void  popMove(void);
  qMove peekLastMove(void) const   {return moveStack[sp].move;};

  const qPosition* getPos(void) const {return &(moveStack[sp].resultingPos);} ;
  const qPosition* getPrevPos(void) const
    {return (sp > 0) ? &(moveStack[sp-1].resultingPos) : NULL;};
  qPlayer getPlayer2Move(void) const
    { return moveStack[sp].playerMoved.otherPlayer(); };

  // By setting the posinfo as we build a stack, we can avoid needing to
  // perform hash lookups again on the way back down the stack.
  // We can assume no posInfo records that were in the stack got harvested
  // to free memory because anything under evaluation cannot be cleaned
  // (or we'd lose track of any possible cycles in our thought).
  qPositionInfo *getPosInfo(void) const       {return moveStack[sp].posInfo;};
  void           setPosInfo(qPositionInfo *p) {moveStack[sp].posInfo = p;};

  // Get list of possible wall moves from current location, appending them to
  // the back of the passed-in list.  Return true on success, false otherwise.
  bool getPossibleWallMoves(qMoveList *moveList) const;

  /***************************************************
   * Members for handling moves under evaluation     *
   ***************************************************/

  /* Note: because we want to use the "possible wall move" optimization,
   * we need to record all moves in our stack--we can't keep actual moves
   * in one stack and "contemplating" moves in another.
   * Thus, there is no point in creating a derived class of qMoveStack
   * that flags which moves are under consideration and allows querying
   * if a given move is in the thought sequence.  Rather, all parts of
   * the AI player will share one stack, and will use the following distinct
   * funcs for pushing/popping moves under consideration, flagging them as
   * in the thought sequence.  Use the direct push() pop() member functions
   * when making real moves.
   */

  // These functions mark positions under evaluation so we can check what
  // is in the stack.
  // They set bits in records looked up in qPosHash to mark moves as
  // "currently under evaluation."
  // This uses bits 1 & 2 in the qPositionInfo qPositionFlag.
  // MT NOTE:  depending on the posHash (& posInfo) for tagging which
  // positions are in the stack is convenient, but prevents multiple
  // qMoveStacks from operating on one posHash.  We should implement
  // a data type as part of the qMoveStack for checking if moves are in
  // the stack.  ???
  // ENCAPSULATION NOTE: The dependence on the posHash means these should
  // probably be helper funcs defined outside the qMoveStack class. (???)

  qPositionInfo *pushEval(qPositionInfo     *startPosInfo, // optimizer
                          qPositionInfo     *endPosInfo,   // optimizer
			  qPositionInfoHash *posHash,  // reqd if posInfo==NULL
			  qPlayer            whoMoved, //   From here down same
			  qMove              move,     //   as pushMove
			  qPosition         *endPos);

  void  popEval();

  // Returns if position is in the move stack with specified player to move
  bool isInEvalStack (qPositionInfo *posInfo,
		      qPlayer        player) const;
  inline bool isWhiteMoveInEvalStack(qPositionInfo *posInfo) const;
  inline bool isBlackMoveInEvalStack(qPositionInfo *posInfo) const;

 private:
  qMoveStackFrame moveStack[MOVESTACKSIZ];
  guint8          sp;

  qWallMoveInfo     allWallMoveArry[256];  // max possible encoding fr/qMove
  qWallMoveInfoList possibleWallMoves;
};


#endif // INCLUDE_movstack_h
