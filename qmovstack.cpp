/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qmovstack.h"

IDSTR("$Id: qmovstack.cpp,v 1.7 2006/07/09 06:37:38 bmiller Exp $");


/***********************
 * class qWallMoveList *
 ***********************/

qMoveStack::qMoveStack
(const qPosition *pos, qPlayer player2move)
{
  sp = 0;
  moveStack[sp].resultingPos = *pos;
  // moveStack[sp].move = qMove();  Unnecessary
  moveStack[sp].wallMovesBlockedByMove.clearList();
  moveStack[sp].playerMoved = qPlayer(player2move.getOtherPlayerId());
  initWallMoveTable();
}

qMoveStack::~qMoveStack() {
  return;
};


/* Good optimizations:
 * !!! Prune "dead space" from the list of possible wall moves (well,
 *     everything after the first "dead" move).
 * !!! Keep separate lists for white and black???
 */
void qMoveStack::initWallMoveTable()
{
  qPosition pos = moveStack[0].resultingPos;
  int rowColNo, posNo;
  int rowOrCol;
  qMove mv;
  qWallMoveInfo *thisMove;

  // Can't revise the wallMoveTable with a bunch of state in the stack
  if (sp != 0) {
    g_assert(sp==0);
    return;
  }

  // 1st pass:  construct list of all possible wall moves.
  for (rowOrCol=1; rowOrCol >= 0; rowOrCol--)
    for (rowColNo=7; rowColNo >= 0; rowColNo--)
      for (posNo=7; posNo >= 0; posNo--)
	{
	  mv = qMove(rowOrCol, rowColNo, posNo); // how about mv.qMove(...)???
	  thisMove = &allWallMoveArry[mv.getEncoding()];

	  thisMove->move     = mv;
	  thisMove->possible = pos.canPutWall(rowOrCol, rowColNo, posNo);
	  thisMove->eliminates.clearList();
	  if (thisMove->possible)
	    possibleWallMoves.push(thisMove);
	  else
	    thisMove->next = thisMove->prev = NULL;
	}

  // 2nd pass: note for each possible move which future wall moves it blocks
#define MAYBE_ELIMINATE(mv) if((mv)->possible){thisMove->eliminates.push(mv);}
  for (thisMove=possibleWallMoves.getHead();
       thisMove;
       thisMove = thisMove->next)
    {
      mv = thisMove->move;

      if (mv.wallPosition()>0)
	MAYBE_ELIMINATE(&allWallMoveArry[qMove(mv.wallMoveIsRow(),
					       mv.wallRowOrColNo(),
					       mv.wallPosition()-1).getEncoding()]);
      if (mv.wallPosition()<7)
	MAYBE_ELIMINATE(&allWallMoveArry[qMove(mv.wallMoveIsRow(),
					       mv.wallRowOrColNo(),
					       mv.wallPosition()+1).getEncoding()]);
      MAYBE_ELIMINATE(&allWallMoveArry[qMove(!mv.wallMoveIsRow(),
					     mv.wallPosition(),
					     mv.wallRowOrColNo()).getEncoding()]);
    }
}

void qMoveStack::pushMove(qPlayer playerMoving, qMove mv, qPosition *endPos)
{
#define ARRAYSIZE(ARR) (sizeof(ARR)/sizeof((ARR)[0]))

  g_assert(sp < ARRAYSIZE(allWallMoveArry));
  qMoveStackFrame *frame = &moveStack[++sp];

  // Record the move
  frame->move = mv;
  frame->playerMoved = playerMoving;

  // Record the new position
  if (endPos)
    frame->resultingPos = *endPos;
  else
    (frame->resultingPos = moveStack[sp-1].resultingPos).applyMove(playerMoving, mv);

  if (mv.isPawnMove())
    frame->wallMovesBlockedByMove.clearList();
  else {
    qWallMoveInfo *thisMove, *blockedMove, *next;

    thisMove = &allWallMoveArry[mv.getEncoding()];
    g_assert(thisMove->possible == TRUE);

    frame->wallMovesBlockedByMove.clearList();

    // Of course, remove the wall placement from possible moves
    thisMove->possible = FALSE;
    frame->wallMovesBlockedByMove.push(thisMove);

    // Remove any other wall move options that are now blocked
    for (blockedMove=thisMove->eliminates.getHead();
	 blockedMove;
	 blockedMove = next)
    {
      next = blockedMove->next; // Copy this cuz we mess with blockedMove
      if (blockedMove->possible) {
	blockedMove->possible = FALSE;
	possibleWallMoves.pop(blockedMove);
        frame->wallMovesBlockedByMove.push(blockedMove);
      }
    }
  }

  return;
}

void qMoveStack::popMove
(void)
{
  qWallMoveInfo *blockedMove, *next;
  qMoveStackFrame *frame = &moveStack[sp--];

  // Replace any wall moves that had been blocked by the popped move
  while (blockedMove = frame->wallMovesBlockedByMove.pop())
    {
      blockedMove->possible = TRUE;
      // !!! Optimization: we could insert the entire wallMovesBlockedByMove
      // list into possibleWallMoves in one segment.
      possibleWallMoves.push(blockedMove);
    }

  return;
}


/***************************
 * class qWallMoveInfoList *
 ***************************/
qWallMoveInfoList::qWallMoveInfoList
(void)
{
  head = tail = NULL;
  return;
}

void qWallMoveInfoList::push
(qWallMoveInfo *mv)
{
  mv->prev = NULL;
  mv->next = head;
  if (head) {
    head->prev = mv;
  } else {
    tail = mv;
  }
  head = mv;
}
void qWallMoveInfoList::pop
(qWallMoveInfo *mv)
{
  if (mv->next) {
    mv->next->prev = mv->prev;
  } else {
    tail = mv->prev;
  }
  if (mv->prev) {
    mv->prev->next = mv->next;
  } else {
    head = mv->next;
  }
}
qWallMoveInfo *qWallMoveInfoList::pop
(void)
{
  qWallMoveInfo *rval=head;
  if (head) {
    head = head->next;
    head->prev = NULL;
    if (!head)
      tail = NULL;
  }
  return rval;
};


// Helper funcs for flagging positions in the thought sequence
//

/*********************
 * Private utilities *
 *********************/

// These funcs use the positionHash to flag which moves are under evaluation
// These funcs flag which moves are under evaluation
inline static void setMoveInEval
(qPositionInfo *posInfo, qPlayer playerToMove)
{
  if (playerToMove.isWhite())
    posInfo->setPositionFlagBits(qPositionInfo::flag_WhiteToMove);
  else
    posInfo->setPositionFlagBits(qPositionInfo::flag_BlackToMove);
}

inline static void clearMoveInEval
(qPositionInfo *posInfo, qPlayer playerToMove)
{
  if (playerToMove.isWhite())
    posInfo->clearPositionFlagBits(qPositionInfo::flag_WhiteToMove);
  else
    posInfo->clearPositionFlagBits(qPositionInfo::flag_BlackToMove);
}

// This is a fast check--since revisiting positions is rare, it should
// be used before checking for which playerToMove is in the stack
inline static bool isInMoveStack
(qPositionInfo *posInfo)
{ return (posInfo->isPosExceptional() && (posInfo->getPositionFlag() > 0));};


/***********************
 * Public helper funcs *
 ***********************/

// These funcs set the positionHash in the qPosHash to track which moves are
// currently under evaluation.
qPositionInfo *pushMove(qMoveStack        *movStack,
			qPositionInfo     *start_posInfo, // optimizer
			qPositionInfoHash *posHash,  // reqd if posInfo==NULL
			qPlayer            whoMoved, //   From here down same
			qMove              move,     //   as qMoveStack
			qPosition         *endPos)
{
  g_assert(movStack);
  g_assert(start_posInfo || posHash);
  if (!start_posInfo && !posHash)
    return NULL;

  // 1. Mark position as under evaluation in posInfo w/setMoveInEval()
  if (!start_posInfo)
    start_posInfo = posHash->getOrAddElt(movStack->getPos());
  setMoveInEval(start_posInfo, whoMoved);
  g_assert(whoMoved.playerId() == movStack->getPlayer2Move().playerId);

  // 2. call movStack->pushMove(mv)
  movStack->pushMove(whoMoved, move, endPos);
  return start_posInfo;
}

void popMove(qMoveStack *movStack)
{
  // 1. Pop off to previous moveStack frame
  movStack->popMove();

  // 2. Mark the position we were examining as no longer under evaluation
  clearMoveInEval(movStack->getPosInfo(), movStack->getPlayer2Move());
}

/* These "private" versions of funcs don't need the movStack arg in our
 * current implementation.  Future implementations, though, will probably
 * require the movStack arg.  We've used inline functions in qmovstack.h
 * to discard the first arg and call a private_* version of these funcs
 * with one fewer arg.  When these funcs are changed to use a movStack
 * arg then we can remove "private_" from their names and get rid of the
 * inline versions in the header???
 */
bool private_isInMoveStack
(qPositionInfo *posInfo, qPlayer p)
{ return (isInMoveStack(posInfo) &&
	  (p.isWhite() ? isWhiteMoveInStack(NULL, posInfo) :
	   isBlackMoveInStack(NULL, posInfo)));};

bool private_isWhiteMoveInStack
(qPositionInfo *posInfo)
{ return (isInMoveStack(posInfo) &&
          (posInfo->getPositionFlag() & flag_WhiteToMove));};

bool private_isBlackMoveInStack
(qPositionInfo *posInfo)
{ return (isInMoveStack(posInfo) &&
	  (posInfo->getPositionFlag() & flag_BlackToMove));};


bool qMoveStack::getPossibleWallMoves(qMoveList *moveList) const
{
  if (!moveList)
    return FALSE;

  qWallMoveInfo *c = possibleWallMoves.getHead();

  while (c) {
    g_assert(c->possible == TRUE);
    moveList->push_back(c->move);
    c = c->next;
  }
  return TRUE;
}
