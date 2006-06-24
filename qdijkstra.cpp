/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

#include "qdijkstra.h"

IDSTR("$Id: qdijkstra.cpp,v 1.1 2006/06/24 00:24:05 bmiller Exp $");

int qDijkstra
(qDijkstraArg* arg)
{
  g_assert(arg &&
	   arg->pos &&
	   (arg->player->isWhite() || arg->player->isWhite));

  if (arg->pos->isWon(arg->player)) {
    arg->dist[0] = 0;
    arg->dist[1] = -1;
    return 1;
  }

  int rval = 0;
  vector<qSquare> frontier;
  int dist[qSquare.maxSquareNum] = {0};
  /* 0 will represent infinite distance to squares (i.e. unvisited squares)
   * >0 will represent distance+1 from the pawn to that square.
   * When we reach a finishing square, subtract 1 to get moves to that square.
   */

  qSquare currSquare = pos->getPawn(arg->player);
  qSquare tmpSquare;
  frontier->push_back(currSquare);
  dist[currSquare.squareNum] = 1;

  if (arg->player->isWhite())
    { // Only check if done after UP moves
      while (currSquare = frontier.pop_front()) {
	curr_dist = dist[currSquare.squareNum]+1;

	if (!isBlockedByWall(currSquare, UP) &&
	    !dist[(tmpSquare=currSquare).applyDirection(UP)])
	  {
	    if (arg->pos->isWhiteWon()) {
	      arg->dist[rval++] = curr_dist - 1;
	      if (!getAllRoutes) 
		return rval;
	    } else {
	      dist[tmpSquare] = curr_dist;
	      frontier->push_back(tmpSquare);
	    }
	  }
	if (!isBlockedByWall(currSquare, DOWN) &&
	    !dist[(tmpSquare=currSquare).applyDirection(DOWN)]) {
	  dist[tmpSquare] = curr_dist;
	  frontier->push_back(tmpSquare);
	}
	if (!isBlockedByWall(currSquare, LEFT) &&
	    !dist[(tmpSquare=currSquare).applyDirection(LEFT)]) {
	  dist[tmpSquare] = curr_dist;
	  frontier->push_back(tmpSquare);
	}
	if (!isBlockedByWall(currSquare, RIGHT) &&
	    !dist[(tmpSquare=currSquare).applyDirection(RIGHT)]) {
	  dist[tmpSquare] = curr_dist;
	  frontier->push_back(tmpSquare);
	}
      }
    }
  else
    { // player is black; only check for completion after DOWN moves
      while (currSquare = frontier.pop_front()) {
	curr_dist = dist[currSquare.squareNum]+1;

	if (!isBlockedByWall(currSquare, DOWN) &&
	    !dist[(tmpSquare=currSquare).applyDirection(DOWN)])
	  {
	    if (arg->pos->isBlackWon()) {
	      arg->dist[rval++] = curr_dist - 1;
	      if (!getAllRoutes) 
		return rval;
	    } else {
	      dist[tmpSquare] = curr_dist;
	      frontier->push_back(tmpSquare);
	    }
	  }
	if (!isBlockedByWall(currSquare, UP) &&
	    !dist[(tmpSquare=currSquare).applyDirection(UP)]) {
	  dist[tmpSquare] = curr_dist;
	  frontier->push_back(tmpSquare);
	}
	if (!isBlockedByWall(currSquare, LEFT) &&
	    !dist[(tmpSquare=currSquare).applyDirection(LEFT)]) {
	  dist[tmpSquare] = curr_dist;
	  frontier->push_back(tmpSquare);
	}
	if (!isBlockedByWall(currSquare, RIGHT) &&
	    !dist[(tmpSquare=currSquare).applyDirection(RIGHT)]) {
	  dist[tmpSquare] = curr_dist;
	  frontier->push_back(tmpSquare);
	}
      }
    }

  return rval;
}
