/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

#include "qdijkstra.h"
#include "qposition.h"
#include <deque>

IDSTR("$Id: qdijkstra.cpp,v 1.5 2006/08/02 04:08:07 bmiller Exp $");


/****/

int qDijkstra
(qDijkstraArg* arg)
{
  g_assert(arg &&
	   arg->pos &&
	   (arg->pos->numWhiteWallsLeft() <= 0x0f) &&
	   (arg->pos->numBlackWallsLeft() <= 0x0f) &&
	   (arg->pos->getWhitePawn().squareNum <= qSquare::maxSquareNum) &&
	   (arg->pos->getBlackPawn().squareNum <= qSquare::maxSquareNum) &&
	   (arg->player.isWhite() || arg->player.isBlack()));

  if (arg->pos->isWon(arg->player)) {
    arg->dist[0] = 0;
    arg->dist[1] = -1;
    return 1;
  }

  int rval = 0;
  std::deque<qSquare> frontier;
  int dist[qSquare::maxSquareNum+1] = {0};
  /* 0 will represent infinite distance to squares (i.e. unvisited squares)
   * >0 will represent distance+1 from the pawn to that square.
   * When we reach a finishing square, subtract 1 to get moves to that square.
   */

  qSquare currSquare = arg->pos->getPawn(arg->player);
  qSquare tmpSquare(qSquare::undefSquareNum);
  frontier.push_back(currSquare);
  dist[currSquare.squareNum] = 1;
  int curr_dist;

  if (arg->player.isWhite())
    { // Only check if done after UP moves
      while (!frontier.empty()) {
	currSquare = frontier.front();
        frontier.pop_front();
	curr_dist = dist[currSquare.squareNum]+1;

	if (!arg->pos->isBlockedByWall(currSquare, UP) &&
	    !dist[((tmpSquare=currSquare).applyDirection(UP)).squareNum])
	  {
	    if (tmpSquare.isWhiteWon()) {
	      arg->dist[rval++] = curr_dist - 1;
	      if (!arg->getAllRoutes) 
		return rval;
	    } else {
	      dist[tmpSquare.squareNum] = curr_dist;
	      frontier.push_back(tmpSquare);
	    }
	  }
	if (!arg->pos->isBlockedByWall(currSquare, DOWN) &&
	    !dist[((tmpSquare=currSquare).applyDirection(DOWN)).squareNum]) {
	  dist[tmpSquare.squareNum] = curr_dist;
	  frontier.push_back(tmpSquare);
	}
	if (!arg->pos->isBlockedByWall(currSquare, LEFT) &&
	    !dist[((tmpSquare=currSquare).applyDirection(LEFT)).squareNum]) {
	  dist[tmpSquare.squareNum] = curr_dist;
	  frontier.push_back(tmpSquare);
	}
	if (!arg->pos->isBlockedByWall(currSquare, RIGHT) &&
	    !dist[((tmpSquare=currSquare).applyDirection(RIGHT)).squareNum]) {
	  dist[tmpSquare.squareNum] = curr_dist;
	  frontier.push_back(tmpSquare);
	}
      }
    }
  else
    { // player is black; only check for completion after DOWN moves
      while (!(frontier.empty())) {
	currSquare = frontier.front();
        frontier.pop_front();
	curr_dist = dist[currSquare.squareNum]+1;

	if (!arg->pos->isBlockedByWall(currSquare, DOWN) &&
	    !dist[((tmpSquare=currSquare).applyDirection(DOWN)).squareNum])
	  {
	    if (tmpSquare.isBlackWon()) {
	      arg->dist[rval++] = curr_dist - 1;
	      if (!arg->getAllRoutes) 
		return rval;
	    } else {
	      dist[tmpSquare.squareNum] = curr_dist;
	      frontier.push_back(tmpSquare);
	    }
	  }
	if (!arg->pos->isBlockedByWall(currSquare, UP) &&
	    !dist[((tmpSquare=currSquare).applyDirection(UP)).squareNum]) {
	  dist[tmpSquare.squareNum] = curr_dist;
	  frontier.push_back(tmpSquare);
	}
	if (!arg->pos->isBlockedByWall(currSquare, LEFT) &&
	    !dist[((tmpSquare=currSquare).applyDirection(LEFT)).squareNum]) {
	  dist[tmpSquare.squareNum] = curr_dist;
	  frontier.push_back(tmpSquare);
	}
	if (!arg->pos->isBlockedByWall(currSquare, RIGHT) &&
	    !dist[((tmpSquare=currSquare).applyDirection(RIGHT)).squareNum]) {
	  dist[tmpSquare.squareNum] = curr_dist;
	  frontier.push_back(tmpSquare);
	}
      }
    }

  return rval;
}
