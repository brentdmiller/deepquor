/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */



#include "qtypes.h"
#include "qposition.h"
#include "posinfo.h"
#include "poshash.h"
#include "movstack.h"
#include "parameters.h"

IDSTR("$Id: qsearcher.cpp,v 1.1 2005/11/09 20:27:25 bmiller Exp $");


// Criteria:
// minimum depth
// maximum complexity
// maximum time spent
search
()
{
  // 1. set up alarm for max time to spend searching

  // 2. 

  // 3. If winning, favor high move with greatest "low point" (i.e. score
  minus complexity)
  //    If losing, favor move with highest peak (i.e. score plus complexity)
  //    If near even, favor move with highest score

  while (!done) {
    // Get list of possible moves

    // Get current evaluation stored for each possible move
    // Override evaluation score with 0 (even for both players) for any
    // position with matching player to move already in this line of
    // thinking. i.e. check each position in the move stack.
    

    // Throw out any moves whose score + complexity < the highest score
    // score - corresponding complexity

    // Find best score
    // Find best move according to:
    // sortscore = score + B(score, complexity)
    // B(score, complexity) meets these criteria:
    // for score>0, B is negative and related to 1/complexity
    // for score==0, B is 0
    // for score<0, B is positive and related to complexity*log(depth)
    //     or sqrt(depth)

    // Are criteria for making a move met?

    // Has time expired?
    // Yes: return best chosen move

    // Is best move complexity 0 forced win?
    // Yes: make best move

    // Is best move complexity 0 forced loss?
    // Yes: make move with most computations (if we have computations avail)

    // Have we met minimum depth yet?
    // No: Evaluate "best" move deeper

    // Among best moves in contention, is bestmove only move left?
    // Yes: return bestmove

    // Is complexity of all moves in contention below some minimum threshold?
    // (threshhold function of time, score, etc.)
    // Yes: return bestmove

    // Among moves in contention, evaluate most complex position deeper
  }
}
