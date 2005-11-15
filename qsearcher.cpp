/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "searcher.h"

IDSTR("$Id: qsearcher.cpp,v 1.2 2005/11/15 18:49:42 bmiller Exp $");


/*******************
 * class qSearcher *
 ******************/


// Criteria:
// minimum depth
// complexity threshold
// maximum time spent
qMove
qSearcher::search
(qPosition pos,
 qPlayer player2move,
 gint32 maxtime,
 gint32 suggested_time)
{
  // Stop bg searcher
  bgStop();

  // call iSearch
  move = iSearch(pos, player2move, maxtime, suggested_time);

  // Update position/bgPlayerToMove with latest move
  bgPos.applyMove(player2move, move);

  // Restart bg searcher
  bgPlayerToMove = qPlayer(player2move.getOtherPlayerId);

  bgGo();

  return move;
}


qMove
qSearcher::iSearch
(qPlayer player2move,
 guint8  max_complexity,
 guint8  minbreadth,
 guint8  mindepth,
 gint32  maxtime,
 gint32  suggested_time)
{
  gint8 current_depth = 0;
  time_type end_time;

  // Figure out how long to think
  guint8 think_time = suggested_time;
  if ((!think_time) || (maxtime < end_time))
    end_time = maxtime;
  end_time = gettime() + maxtime;

  // Start off with a breadth first search up through some minimum number of
  // plies???
  // i.e. for some depth n, dive down and evaluate every possible move
  // at depth n and percolate the scores back up. (make this an arg such
  // as comprehensive_depth???)  This could actually be a mode in the while
  // loop:  while we're in breadth mode, call a depth routine for every
  // possible move with the depth set to some number.
  // (depth < 0 could mean just go to a leaf, and then extend the depth
  // by "depth" plies); depth>0 could mean simply evaluate all options
  // (breadth first) through depth "depth")

  while () {
    // Are criteria for making a move met?

    // Has time expired?
    // Yes: return best chosen move
    if (gettime() >= maxtime)
      return best_move;

    // Is best move complexity 0 forced win?
    // Yes: make best move
    if (best_move().score == qScore_won) {
      return best_move;
    }

    // Is best move complexity 0 forced loss?
    // Yes: make move with most computations (if we have computations avail;
    // otherwise just make any move)
    if (best_move().score == qScore_lost) {
#if WE_HAVE_NUM_COMPUTATIONS
      // Find move with most computations
      return move_with_most_computations;
#endif
      return best_move;
    }

    // Have we met minimum depth yet?
    // No: Evaluate "best" move deeper

    // Among best moves in contention, is bestmove only move left?
    // Yes: return bestmove

    // Is complexity of all moves in contention below some minimum threshold?
    // (threshhold function of time, score, etc.)
    // Yes: return bestmove

    // If there is still no clear best move, of those moves in contention
    // pick the one that results in the most complex position and evaluate it
    // deeper.
    // ??? Perhaps there should be an "evaluate deeper" flag to this
    // routine, or a separate evaluate deeper routine, to skip constraint
    // evaluation and force a walk down to the deepest leaf and then evaluate
    // all possible moves from that position and trickle the score back up.
  }

}


qPositionEval qSearch::scanDeeper
(qPosition pos,
 qPlayer player2move,
 gint8 depth
)
{
  qPositionInfo *posInfo;

  // Get this position's evaluation
  posInfo = posHash.getElt(pos);

  if (!posInfo)
    {
      posInfo = addElt(pos);
      g_assert(posInfo);
    }
  else
    {
      // Check if this position is already in the move stack
      if (moveStack.isInMoveStack(pos, player2move))
	return even_evaluation;
    }

  moveStack.pushMove(pos, player2move);

    // If we don't have an evaluation for the current position
    // do a shallow check if it's already a won position
	myeval = lookupEvaluation();
	if (!myeval) {
	  if (GAME_IS_WON()) {
	    myeval.set(player2move, score=qScore_won, complexity=0);
	    return 1or0evaluation;
	  } else {
	    myeval.set(player_white, score=0, complexity=);
	  }

  while (!done) {
    // Get list of possible moves

    if (depth > 0)
      {
    // Get current evaluation stored for each possible move
    // For any position already in this line of thinking
    // with matching player to move, override evaluation score
    // with 0 (even for both players).
    // i.e. check each position in the move stack.


    // Throw out any moves whose score + complexity < the highest score
    // score - corresponding complexity

    // Find best score
    // If winning, favor conservative positions; if losing, favor complex
    // positions.
    // Thus, find best scoring move according to:
    // sortscore = score + B(score, complexity)
    // B(score, complexity) meets these criteria:
    // for score>0, B is negative and related to 1/complexity
    // for score==0, B is 0
    // for score<0, B is positive and related to complexity*log(depth)
    //     or sqrt(depth)


  }
}




void setBackgroundThinking(bool thinking)
{
  g_mutex_lock(bgMutex);
  bgThinking = thinking;
  if (bgThinking) {
    // Turn on bg thread if currently opponent's move???
  }
  g_mutex_unlock(bgMutex);
}

bool getBackgroundThinking()
{
  return bgThinking;
}

void bgStop()
{
  g_mutex_lock(bgMutex);
  if (bgThinking) {
    // Use cond var and/or semaphor to signal thread & put it into sleep state?
  }
  g_mutex_unlock(bgMutex);
}

void bgGo(qPosition pos, qPlayer playerToMove)
{
  g_mutex_lock(bgMutex);
  if (bgThinking) {
    if (bgThread) {
      awaken bg Thread???;
      increment semaphor and signal cond var???;
    }
  }
  g_mutex_unlock(bgMutex);
}
