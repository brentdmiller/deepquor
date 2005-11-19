/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qmovstack.h"

IDSTR("$Id: qmovstack.cpp,v 1.5 2005/11/19 08:22:33 bmiller Exp $");


/***********************
 * class qWallMoveList *
 ***********************/

qMoveStack::qMoveStack
(qPosition *pos)
{
  sp = 0;
  moveStack[sp].pos = *pos;
  moveStack[sp].move = -1;
  moveStack[sp].wallMovesBlockedByMove = NULL;
  initWallMoveTable(pos);
}

qMoveStack::~qMoveStack() {
  return;
};

void qMoveStack::initWallMoveTable(qPosition *pos)
{
  int rowColNo, posNo;
  int rowOrCol;
  qMove mv;
  qWallMoveInfo *thisMove;

  for (rowOrCol=1; rowOrCol >= 0; rowOrCol++)
    for (rowNo=7; rowNo >= 0; rowNo++)
      for (posNo=7; posNo >= 0; posNo++)
	{
	  mv.qMove(rowOrCol, rowNo, posNo);
	  thisMove = &allWallMoveArry[mv.getEncoding()];

	  thisMove->move     = mv;
	  thisMove->possible = pos.canPutWall(rowOrCol, rowNo, posNo);
	  thisMove->eliminates.clearList;
	  if (thisMove->possible)
	    possibleWallMoves.push(thisMove);
	  else
	    thisMove->next = thisMove->prev = NULL;
	}

  // 2nd pass, noting for each possible move which future wall moves it blocks
#define MAYBE_ELIMINATE(mv) if((mv)->possible){thisMove->eliminates.push(mv);}
  for (thisMove=possibleWallMoves.getHead();
       thisMove;
       thisMove = thisMove->next)
    {
      mv = thisMove->move;

      if (mv.wallPosition>0)
	MAYBE_ELIMINATE(&allWallMoveArry[qMove(mv.wallMoveIsRow,
					       mv.wallRowOrColNo,
					       mv.wallPosition-1).getEncoding()]);
      if (mv.wallPosition<7)
	MAYBE_ELIMINATE(&allWallMoveArry[qMove(mv.wallMoveIsRow,
					       mv.wallRowOrColNo,
					       mv.wallPosition+1).getEncoding()]);
      MAYBE_ELIMINATE(&allWallMoveArry[qMove(!mv.wallMoveIsRow,
					     mv.wallPosition,
					     mv.wallRowOrColNo).getEncoding()]);
    }
}

void qMoveStack::pushMove(qPlayer playerMoving, qMove mv, qPosition *endPos)
{
#define ARRAYSIZE(ARR) (sizeof(ARR)/sizeof((ARR)[0]))

  q_assert(sp < ARRAYSIZE(allWallMoveArry));
  qMoveStackFrame *frame = &moveStack[++sp];

  // Record the move
  frame->move = mv;
  frame->playerMoved = playerMoving;

  // Record the new position
  if (endPos)
    frame->pos = *endPos;
  else
    (frame->pos=moveStack[sp-1].pos).applyMove.applyMove(playerMoving, mv);

  if (mv.isPawnMove())
    frame->wallMovesBlockedByMove.clearList();
  else {
    qWallMove *thisMove, *blockedMove, *next;

    thisMove = allWallMoveArry[mv.getEncoding()];
    g_assert(thisMove->possible == True);

    frame->wallMovesBlockedByMove.clearList();

    // Of course, remove the wall placement from possible moves
    thisMove->possible = False;
    frame->wallMovesBlockedByMove.push(thisMove);

    // Remove any other wall move options that are now blocked
    for (blockedMove=thisMove.eliminates.getHead();
	 blockedMove;
	 blockedMove = next)
    {
      next = blockedMove->next; // Copy this cuz we mess with blockedMove
      if (blockedMove->possible) {
	blockedMove->possible = False;
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
  qWallMove *blockedMove, *next;
  qMoveStackFrame *frame = &moveStack[sp--];

  // Replace any wall moves that had been blocked by the popped move
  while (blockedMove = frame->wallMovesBlockedByMove.pop())
    {
      blockedMove->possible = True;
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
  head = next = NULL;
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
  if (qWallMove->next) {
    qWallMove->next->prev = qWallMove->prev;
  } else {
    tail = qWallMove->prev;
  }
  if (qWallMove->prev) {
    qWallMove->prev->next = qWallMove->next;
  } else {
    head = qWallMove->next;
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
(qPositionInfo posInfo)
{ return (posInfo.isPosExceptional() && (posInfo.getPositionFlag() > 0))};


/***********************
 * Public helper funcs *
 ***********************/

// These funcs set the positionHash in the qPosHash to track which moves are
// currently under evaluation.
qPositionInfo *pushMove(qMoveStack    *movStack,
			qPositionInfo *posInfo,  // optimizer
			qPosInfoHash  *posHash,  // reqd if posInfo==NULL
			qPlayer        whoMoved, //   From here down same
			qMove          move,     //   as qMoveStack
			qPosition     *pos);

inline void popMove(qMoveStack *movStack)
{
  // Mark the position as no longer under evaluation
  movStack->popMove();

  clearMoveInEval(positionEvalsHash->getElt(frame->pos), frame->playerMoved);
}

inline bool isInMoveStack
(qPositionInfo posInfo, qPlayer p)
{ return (isInMoveStack(posInfo) &&
	  (p.isWhite() ? isWhiteMoveInStack(posInfo) :
	   isBlackMoveInStack(posInfo)))};

inline bool isWhiteMoveInStack
(qPositionInfo posInfo)
{ return (isInMoveStack(posInfo) &&
          (posInfo.getPositionFlag() & flag_WhiteToMove))};

inline bool isBlackMoveInStack
(qPositionInfo posInfo)
{ return (isInMoveStack(posInfo) &&
	  (posInfo.getPositionFlag() & flag_BlackToMove))};

