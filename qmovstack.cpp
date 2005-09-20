/***********************
 * class qWallMoveList *
 ***********************/

static qPosition qInitialPosition =
{
  {0,0,0,0,0,0,0,0}, // No walls in any row
  {0,0,0,0,0,0,0,0}, // No walls in any col
  SQUARE(4,0),       // white pawn location
  SQUARE(4,8),       // black pawn location
  0xaa;              // 10 walls each
}

qMoveStack::qMoveStack() { qMoveStack(&qInitialPosition); };

qMoveStack::qMoveStack
(qPosition *pos)
{
  sp = 0;
  moveStack[sp].pos = *pos;
  moveStack[sp].move = -1;
  moveStack[sp].wallMovesBlockedByMove = NULL;
  initWallMoveTable(pos);
}
qMoveStack::~qMoveStack() { return; };

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

void qMoveStack::pushMove(qPlayer p, qMove mv, qPosition *endPos)
{
  qMoveStackFrame *frame = &moveStack[++sp];

  // Record the move
  frame->move = mv;

  // Record the new position
  if (endPos)
    frame->pos = *endPos;
  else
    (frame->pos=moveStack[sp-1].pos).applyMove.applyMove(p, mv);

  if (mv.isPawnMove())
    frame->wallMovesBlockedByMove.clearList();
  else {
    qWallMove *thisMove, *blockedMove, *next;
    thisMove = allWallMoveArry[mv.getEncoding()];
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
