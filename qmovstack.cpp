/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qmovstack.h"

IDSTR("$Id: qmovstack.cpp,v 1.10 2006/07/23 04:29:56 bmiller Exp $");


/****/

/***********************
 * class qWallMoveList *
 ***********************/

qMoveStack::qMoveStack
(const qPosition *pos, qPlayer player2move)
  :sp(0)
{
  moveStack[sp].resultingPos = *pos;
  // moveStack[sp].move = qMove();  Unnecessary
  moveStack[sp].wallMovesBlockedByMove.clearList();
  moveStack[sp].playerMoved = qPlayer(player2move.getOtherPlayerId());
  moveStack[sp].posInfo = NULL;
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
	  thisMove->eliminates.clear();
	  if (thisMove->possible)
	    possibleWallMoves.push(thisMove);
	  else
	    thisMove->next = thisMove->prev = NULL;
	}

  // 2nd pass: for each possible move, note which future wall moves it blocks
#define MAYBE_ELIMINATE(m) if((m)->possible){thisMove->eliminates.push_back(m);}
  for (thisMove=possibleWallMoves.getHead();
       thisMove;
       thisMove = thisMove->next)
    {
      mv = thisMove->move;
      g_assert(mv.isWallMove());

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

void qMoveStack::pushMove
(qPlayer        playerMoving,
 qMove          mv,
 qPosition     *endPos)
{
#define ARRAYSIZE(ARR) (sizeof(ARR)/sizeof((ARR)[0]))

  g_assert(sp < ARRAYSIZE(moveStack));
  qMoveStackFrame *frame = &moveStack[++sp];

  // Record the move
  frame->move = mv;
  frame->playerMoved = playerMoving;
  frame->posInfo     = NULL;

  // Record the new position
  if (endPos)
    frame->resultingPos = *endPos;
  else
    (frame->resultingPos = moveStack[sp-1].resultingPos).applyMove(playerMoving, mv);

  if (mv.isPawnMove())
    frame->wallMovesBlockedByMove.clearList();
  else {
    qWallMoveInfo *thisMove, *next;
    list<qWallMoveInfo*>::iterator blockedMove;

    thisMove = &allWallMoveArry[mv.getEncoding()];
    g_assert(thisMove->possible == TRUE);

    frame->wallMovesBlockedByMove.clearList();

    // Of course, remove the wall placement from possible moves
    thisMove->possible = FALSE;
    possibleWallMoves.pop(thisMove);
    frame->wallMovesBlockedByMove.push(thisMove);

    // Remove any other wall move options that are now blocked
    for (blockedMove=thisMove->eliminates.begin();
	 blockedMove != thisMove->eliminates.end();
	 ++blockedMove)
    {
      if ((*blockedMove)->possible) {
	(*blockedMove)->possible = FALSE;
	possibleWallMoves.pop(*blockedMove);
        frame->wallMovesBlockedByMove.push(*blockedMove);
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
(void):
  head(NULL), tail(NULL)
{
  DEBUG_CODE(numElts = 0;)
  return;
}

#ifdef DEBUG
void qWallMoveInfoList::verifyEltCount()
{ // Make sure this mv isn't already in the info list (which will corrupt us)
  qWallMoveInfo *tmp = head;
  int countedElts = 0;
  while (tmp) {
    tmp = tmp->next;
    countedElts++;
  }
  g_assert(countedElts == numElts);
}
#endif


void qWallMoveInfoList::push
(qWallMoveInfo *mv)
{
#ifdef DEBUG
  { // Make sure this mv isn't already in the info list (which will corrupt us)
    qWallMoveInfo *tmp = head;
    while (tmp) {
      g_assert(tmp!=mv);
      tmp = tmp->next;
    }
  }
#endif

  DEBUG_CODE(verifyEltCount();)

  mv->prev = NULL;
  mv->next = head;
  if (head) {
    head->prev = mv;
  } else {
    tail = mv;
  }
  head = mv;

  DEBUG_CODE(++numElts;)
  DEBUG_CODE(verifyEltCount();) //Make sure we didn't break anything
}
void qWallMoveInfoList::pop
(qWallMoveInfo *mv)
{
#ifdef DEBUG
  { // Make sure the mv is actually in the list
    qWallMoveInfo *tmp = head;
    while (tmp) {
      if (tmp==mv)
	break;
      tmp = tmp->next;
    }
    g_assert(tmp);
  }
#endif

  DEBUG_CODE(verifyEltCount();)

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

  DEBUG_CODE(--numElts;)
  DEBUG_CODE(verifyEltCount();) //Make sure we didn't break anything
}
qWallMoveInfo *qWallMoveInfoList::pop
(void)
{
  DEBUG_CODE(verifyEltCount();)

  qWallMoveInfo *rval=head;
  if (head) {
    head = head->next;
    if (head)
      head->prev = NULL;
    else
      tail = NULL;
  }

  DEBUG_CODE(if (rval) --numElts;)
  DEBUG_CODE(verifyEltCount();) //Make sure we didn't break anything
  return rval;
};

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


// Funcs that flag positions in the thought sequence (i.e. for evaluating
// moves temporarily rather than actually making them.
// These funcs use the positionHash to flag which moves are under evaluation
//
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


qPositionInfo *qMoveStack::pushEval
(qPositionInfo     *startPosInfo, // optimizer
 qPositionInfo     *endPosInfo,   // optimizer
 qPositionInfoHash *posHash,  // reqd if endPosInfo==NULL or startPosInfo==NULL
 qPlayer            whoMoved, //   From here down same
 qMove              move,     //   as pushMove()
 qPosition         *endPos)
{
  g_assert(endPosInfo || posHash);
  if (!endPosInfo && !posHash)
    return NULL;

  g_assert(whoMoved.getPlayerId() == this->getPlayer2Move().getPlayerId());

  // 1. Mark position as under evaluation in posInfo w/setMoveInEval()
  if (!this->moveStack[sp].posInfo) {
    if (startPosInfo)
      this->moveStack[sp].posInfo = startPosInfo;
    else
      this->moveStack[sp].posInfo = posHash->getOrAddElt(this->getPos());
  }
  g_assert(this->moveStack[sp].posInfo);
  setMoveInEval(this->moveStack[sp].posInfo, this->moveStack[sp].playerMoved);

  // 2. Push move onto stack with posInfo
  if (!endPosInfo) {
    if (!endPos) {
      g_assert(this->getPos());
      qPosition myPos = *this->getPos();
      myPos.applyMove(whoMoved, move);
      endPosInfo = posHash->getOrAddElt(&myPos);

      this->pushMove(whoMoved, move, &myPos);
      return endPosInfo;
    }
    endPosInfo = posHash->getOrAddElt(endPos);
  }

  this->pushMove(whoMoved, move, endPos);
  return endPosInfo;
}

void qMoveStack::popEval(void)
{
  // 1. Pop off to previous moveStack frame
  this->popMove();

  // Can't pop a move that's not in eval
  g_assert(this->getPosInfo());

  // 2. Mark the position we were examining as no longer under evaluation
  clearMoveInEval(this->getPosInfo(), this->getPlayer2Move());
}

// This is a fast check--since revisiting positions is rare, it should
// be used before checking for which playerToMove is in the stack
inline static bool private_isInEvalStack
(qPositionInfo *posInfo)
{
  return (posInfo->isPosExceptional() && (posInfo->getPositionFlag() > 0));
}


bool qMoveStack::isInEvalStack
(qPositionInfo *posInfo, qPlayer p) const
{
  return (p.isWhite() ?
	  isWhiteMoveInEvalStack(posInfo) : isBlackMoveInEvalStack(posInfo));
}

bool qMoveStack::isWhiteMoveInEvalStack
(qPositionInfo *posInfo) const
{
  return (private_isInEvalStack(posInfo) &&
          (posInfo->getPositionFlag() & qPositionInfo::flag_WhiteToMove));
}

bool qMoveStack::isBlackMoveInEvalStack
(qPositionInfo *posInfo) const
{
  return (private_isInEvalStack(posInfo) &&
	  (posInfo->getPositionFlag() & qPositionInfo::flag_BlackToMove));
}
