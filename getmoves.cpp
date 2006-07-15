/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "getmoves.h"
#include "qmovstack.h"
#include "qdijkstra.h"

IDSTR("$Id: getmoves.cpp,v 1.4 2006/07/15 05:16:38 bmiller Exp $");


/****/

qMoveList *getPossiblePawnMoves
(const qPosition *pos,
 qPlayer          player2move,
 qMoveList       *returnList)
{
  if (!returnList)
    return NULL;

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
      moveUpUp, moveDownDown, moveLeftLeft, moveRightRight, moveNull,
      moveUL, moveUR, moveDL, moveDR, moveUL, moveDL, moveUR, moveDR, moveNull
    };

  static const qDirection dirList_black[] =
    {
      DOWN, UP, LEFT, RIGHT, 0,
      DOWN, UP, LEFT, RIGHT, 0,
      LEFT, RIGHT, LEFT, RIGHT, DOWN, UP, DOWN, UP, 0
    };
  static const qMove moveList_black[] =
    {
      moveDown, moveUp, moveLeft, moveRight,
      moveDownDown, moveUpUp, moveLeftLeft, moveRightRight, moveNull,
      moveDL, moveDR, moveUL, moveUR, moveDL, moveUL, moveDR, moveUR, moveNull
    };

  static const qDirection *const moveDirections[2] = {
	dirList_white, dirList_black
  };
  g_assert((qPlayer::WhitePlayer==0) && (qPlayer::BlackPlayer==1));
  // I'd prefer the following, but a const must be initialized in the
  // declaration.
  // moveDirections[qPlayer::WhitePlayer] = dirList_white;
  // moveDirections[qPlayer::BlackPlayer] = dirList_black;

  static const qMove *const moveMoves[2] = {
	moveList_white, moveList_black
  };
  // See comment above regarding const intialization.
  // static const qMove      *const moveMoves[2];
  // moveMoves[qPlayer.WhitePlayer] = moveList_white;
  // moveMoves[qPlayer.BlackPlayer] = moveList_black;

  qDirection dir;
  qSquare myPawn = pos->getPawn(player2move);
  qSquare otherPawn = pos->getPawn(player2move.otherPlayer());
  qSquare dest;

  const qDirection *moveDirectionList = moveDirections[player2move.getPlayerId()];
  int i;
  for (i=0;
       dir = moveDirectionList[i];
       ++i) {
    // Is this move allowed?

    if (pos->isBlockedByWall(myPawn, dir))
      continue;

    dest = myPawn.applyDirection(dir);

    if (dest.squareNum != otherPawn.squareNum)
      {
	// dest is vacant--mark it as a legal move
	returnList->push_back(moveMoves[player2move.getPlayerId()][i]);
	continue;
      }

    // ...else other pawn is obstructing us.
    if (!pos->isBlockedByWall(dest, dir))
      // We can jump & land on other side
      {
	returnList->push_back(moveMoves[player2move.getPlayerId()][i+5]);
	continue;
      }
    else
      {
	/* Other side is blocked, try deflecting in each direction */
	int j=2*(i+5);
	if (!pos->isBlockedByWall(dest, j))
	  returnList->push_back(moveMoves[player2move.getPlayerId()][j]);
	j++;
	if (!pos->isBlockedByWall(dest, j))
	  returnList->push_back(moveMoves[player2move.getPlayerId()][j]);
	continue;
      }
  }
  return returnList;
}

qMoveList *getPlayableMoves(const qPosition  *pos,
		            qMoveStack       *movStack,
		            qMoveList        *moveList)
{
  if (!pos || !movStack || !moveList)
    return NULL;

  qPlayer player2move = movStack->getPlayer2Move();

  // Push legal player moves onto the beginning of the list;
  if (!getPossiblePawnMoves(pos, player2move, moveList))
    return NULL;

  // Insert all possible wall moves
  if (pos->numWallsLeft(player2move)) {
    qMoveList tmpList;  // Creates empty list
    qMove mv;
    qDijkstraArg dijArg;
    dijArg.pos = NULL;
    dijArg.player = player2move;
    dijArg.getAllRoutes = FALSE;

    movStack->getPossibleWallMoves(&tmpList);

    // Now iterate from previous back of list to new back, checking for
    // legality of moves and removing any illegal moves
    while (!tmpList.empty()) {
      mv = tmpList.front();
      tmpList.pop_front();

      qPosition testpos = *pos;
      testpos.applyMove(player2move, mv);
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
      dijArg.pos = &testpos;
      if ( qDijkstra(&dijArg) )
	moveList->push_back(mv);
    }

  };
  return moveList;
}

bool pruneUselessMoves(const qPosition  *pos,
                       qMoveList        *moveList)
{return true;/*???*/};

