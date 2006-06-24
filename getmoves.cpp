/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qmovstack.h"

IDSTR("$Id: getmoves.cpp,v 1.2 2006/06/24 00:24:05 bmiller Exp $");


/****/

qMoveList *getPossiblePawnMoves
(qPosition pos,
 qPlayer player2move,
 qMoveList *returnList)
{
  /* Try to put first, the move most likely to cause a short search.
   * These arrays are constructed so that if we hit an opponent's pawn we
   * can find the appropriate (jumping move) using idx+5.  From there, if
   * we need to "deflect" beacause of a wall, we can find the appropriate
   * move using 2*(idx+5) & 2*(idx+5)+1.
   */
  static const qDirection dirList_white[] =
    {
      UP, DOWN, LEFT, RIGHT, 0,
      UP, DOWN, LEFT, RIGHT, 0,
      LEFT, RIGHT, LEFT, RIGHT, UP, DOWN, UP, DOWN, 0
    };
  static const qMove moveList_white[] =
    {
      moveUp, moveDown, moveLeft, moveRight,
      moveUpUp, moveDownDown, moveLeftLeft, moveRightRight, 0
      moveUL, moveUR, moveDL, moveDR, moveUL, moveDL, moveUR, moveDR, 0
    };

  static const qDirection dirList_black[] =
    {
      DOWN, UP, LEFT, RIGHT, 0,
      DOWN, UP, LEFT, RIGHT, 0,
      LEFT, RIGHT, LEFT, RIGHT, DOWN, UP, DOWN, UP, 0
    };
  static const qMoveList moveList_black[] =
    {
      moveDown, moveUp, moveLeft, moveRight,
      moveDownDown, moveUpUp, moveLeftLeft, moveRightRight, 0
      moveDL, moveDR, moveUL, moveUR, moveDL, moveUL, moveDR, moveUR, 0
    };

  static const qDirection *const moveDirections[2];

  moveDirections[qPlayer.WhitePlayer] = qDirectionList_white;
  moveDirections[qPlayer.BlackPlayer] = qDirectionList_black;

  static const qMove      *const moveMoves[2];
  moveMoves[qPlayer.WhitePlayer] = moveList_white;
  moveMoves[qPlayer.BlackPlayer] = moveList_black;

  qDirection dir;
  qSquare myPawn = pos->getPawn(player2move);
  qSquare otherPawn = pos->getPawn(player2move.otherPlayer());
  qSquare dest;
  for (qDirection *moveDirectionList = moveDirections[player2move];
       dir = moveDirectionList[i];
       ++i) {
    // Is this move allowed?

    if (isBlockedByWall(myPawn, dir))
      continue;

    dest = myPawn->applyDirection(dir);

    if (dest.getSquareId() != otherPawn.getSquareId())
      {
	// dest is vacant--mark it as a legal move
	moveList.push_back(moveMoves[player2move][i]);
	continue;
      }

    // ...else other pawn is obstructing us.
    if (!isBlockedByWall(dest, dir))
      // We can jump & land on other side
      {
	moveList.push_back(moveMoves[player2move][i+5]);
	continue;
      }
    else
      {
	/* Other side is blocked, try deflecting in each direction */
	int j=2*(i+5);
	if (!isBlockedByWall(dest, j))
	  moveList.push_back(moveMoves[player2move][j]);
	j++;
	if (!isBlockedByWall(dest, j))
	  moveList.push_back(moveMoves[player2move][j]);
	continue;
      }
  }
  return moveList;
}

bool getPlayableMoves(qPosition *pos,
		      qMoveStack movStack,
		      qMoveList *moveList)
{
  if (!pos || !movStack || !moveList)
    return NULL;

  qPlayer player2move = movStack.getPlayer2Move();

  // Push legal player moves onto the beginning of the list;
  if (!getPossiblePawnMoves(pos, player2move, moveList))
    return NULL;

  // Insert all possible wall moves
  if (pos->numWallsLeft(player2move)) {
    qMoveList tmpList = qMoveList();
    qMove *mv = NULL;

    movStack->getPossibleWallMoves(&tmpList);

    // Now iterate from previous back of list to new back, checking for
    // legality of moves and removing any illegal moves
    while (mv = tmpList->pop_front()) {
      qPosition testpos = pos;
      testpos->apply(mv);
      /* Optimization???:  create some kind of graph or structure that can be
       * used to quickly test each wall position for legality?
       * One way or another, this inner section of loop is going to need to
       * be optimized.  We can view the board as a directed graph, snip some
       * nodes, and then determine if it's still connected.  Examine
       * http://en.wikipedia.org/wiki/Connected_graph for ideas.  We should
       * probably create graphs paralleling the board, one for each player,
       * with the end squares all coalesced into one combined node with 9
       * edges.
       */
      pathFound = qDijkstra(testpos, player2move);  // From eval.cpp
      if (pathFound)
	moveList->push_back(mv);
    }

  };
  return true;
}
