/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qsearcher.h"

IDSTR("$Id: qsearcher.cpp,v 1.4 2005/11/19 08:22:33 bmiller Exp $");


/*******************
 * class qSearcher *
 ******************/

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

  computationTree.initializeTree();
  currentTreeNode = computationTree.getRootNode();

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
      break;

    // Find current top-scoring move
    ???

    // Is top move complexity 0 forced win?
    // Yes: make best move
    if (best_move().score == qScore_won) {
      return best_move;
    }

    // Is top move complexity 0 forced loss?
    // Yes: make move with most computations (if we have computations avail;
    // otherwise just make any move)
    if (best_move().score == qScore_lost) {
#if WE_HAVE_NUM_COMPUTATIONS
      // Find move with most computations
      return move_with_most_computations;
#endif
      return best_move;
    }

    // Among moves in contention, is bestmove only move left?
    // Yes:
    //   Have we met minimum depth yet?
    //   Yes: best_move
    //   No: Evaluate "best" move deeper

    // Is complexity of all moves in contention below some minimum threshold?
    // (threshhold function of time, score, etc.)
    //   Have we met minimum depth yet?
    //   Yes: return bestmove
    //   No: Evaluate "best" move deeper

    // If there is still no clear best move, of those moves in contention
    // pick the one that results in the most complex position and evaluate it
    // deeper.
    // ??? Perhaps there should be an "evaluate deeper" flag to this
    // routine, or a separate evaluate deeper routine, to skip constraint
    // evaluation and force a walk down to the deepest leaf and then evaluate
    // all possible moves from that position and trickle the score back up.
  }

  // Time to return our best move
  // Choose "best" move
  // If winning, favor conservative positions; if losing, favor complex
  // positions.
  // Thus, find best scoring move according to:
  // sortscore = score + B(score, complexity)
  // B(score, complexity) meets these criteria:
  // for score>0, B is negative and related to 1/complexity
  // for score==0, B is 0
  // for score<0, B is positive and related to complexity*log(depth)
  //     or sqrt(depth)   or maybe complexity*log(computations)
  return best_move;
}

/* scanDeeper
 */  
const qPositionEvaluation *qSearch::scanDeeper
(qPosition *pos,
 qPlayer    player2move,
 gint16     depth,
 // qPositionEval *evalToBeat,
 guint32       &r_positionsEvaluated)
{
  qPositionInfo *posInfo;
  guint32        childPositionsComputed;
  qPlayer otherPlayer=qPlayer(player2move.getOtherPlayerId());

  r_positionsEvaluated = 0;

  // See if there's existing position info stored
  if (!(posInfo = getNodePosInfo(currentTreeNode))) {
    if (posInfo = posHash.getElt(pos))
      setNodePosInfo(currentTreeNode, posInfo);
  }

  // Check that we have a posInfo, and that it's not a repeat
  if (!posInfo)
    {
      posInfo = addElt(pos);
      g_assert(posInfo);
      setNodePosInfo(currentTreeNode, posInfo);
    }
  else
    {
      // Check if this position is already in the move stack
      if (moveStack.isInMoveStack(posInfo, player2move))
	return positionEval_even;
    }

  // If we're at the end of a search, return the position's existing
  // evalutation (or make one if necessary)
  if (depth == 0) {
    if (!posInfo.evalExists(player2move)) {
      // Check return val???
      ratePositionByComputation(player2move, posInfo);
      // Let the caller do this:
      // setNodeEval(currentTreeNode, posInfo.get(player2move));
      ++r_positionsEvaluated;
    }
    return posInfo->get(player2move);;
  }

  // If we don't have an evaluation for the current position
  // do a shallow check if it's already a won position
  if (!posInfo.evalExists(player2move))
    {
      if (pos.isWon(player2move)) {
	posInfo.set(player2move, positionEval_won);
	return positionEval_won;
      } else if (pos.isLost(player2move)) {
	posInfo.set(player2move, positionEval_lost);
	return positionEval_lost;
      }

      // Since this is a new position, count it
      r_positionsEvaluated++;
      if (depth > 0) {
	depth--;

#if 0  /* Skip this for now; it breaks the depth counting mechanism */
	// optimization:  we're into new territory, so most adjacent moves
	// will also probably probably be new.  Do a 1-ply depth-first
	// to get the evaluations ready.
	if (depth > 0) {
	  scanDeeper(pos, player2move, -1, NULL);
	  // Don't bother to record the evaluation returned for the current
	  // position...it'll get recorded by the caller when we evenutally
	  // return.
	}
#endif
      }
    }
  else
    {
      // Skip further analyzing forced positions
      if (posInfo.getComplexity(player2move) == 0)
	return posInfo.get(player2move);
    }

  if (depth < 0)
    {
      guint8 move_idx;
      qComputationTreeNodeId next_position_id;

      // Get list of possible moves
      if (!computationTree.getNthChild(currentTreeNode, 0)) {
	qMoveList possible_moves; // Initially empty
	qMoveListIterator i;

	getPlayableMoves(pos, movStack, possible_moves);

	// Should we prune when doing brute force search???
	// Doing so blocks us from being 100% thorough for "analysis modes"
	// We probably won't be using brute force for analysis anyway.
	possible_moves = pruneUselessMoves(pos, possible_moves);

	for (i  = possible_moves.begin();
	     i != possible_moves.end();
	     i++)
	    computationTree.addChildNode(currentTreeNode,
					 *i,
					 positionEval_none);
      }
      
      for (n=0;
	   next_position_id = computationTree.getNthChild(n);
	   n++) {
	possible_move = computationTree.getPrecedingMove(next_position_id);

	moveStack.pushMove(player2move, possible_move, posInfo);
	currentTreeNode = next_position_id;
	scanDeeper(moveStack.getPos(), otherPlayer, depth++, NULL, childPositionsComputed);
	r_positionsEvaluated += childPositionsComputed;
	depth -= positionsComputed;
	currentTreeNode = computationTree.getParent(currentTreeNode);
	moveStack.popMove();
      }
      ratePositionFromNeighbors(pos, player2move, posInfo,
				possible_moves,
				p_eval, p_mvlist, p_evalList);
      /*posInfo = ratePositionFromNeighbors(pos, player2move, posInfo,
					  p_eval, p_mvlist, p_evalList);
      */
      return posInfo;
    }

  // Else we are extending existing analysis (depth > 0)
  do {

    // Make sure we have list of possible moves, with an evaluation
    // each possible move (so that we can compute score from neighbors).
    if (!computationTree.getNthChild(currentTreeNode, 0))
      {
	qMoveList possible_moves; // Initially empty
	qMoveListIterator i;

	getPlayableMoves(pos, movStack, possible_moves);

	// There should there be a way to bypass pruning for analysis mode
	possible_moves = pruneUselessMoves(pos, possible_moves);

	for (i  = possible_moves.begin();
	     i != possible_moves.end();
	     i++)
	  {
	    qMove possible_move = *i;
	    qPosition newPos = *pos;
	    newPos.applyMove(player2move, possible_move);

	    qPositionInfo *newposInfo = posHash.getElt(&newPos);

	    // See if we need to compute an evaluation.
	    // Use existing evaluations; use even_evaluation for
	    // cycling back to repeated positions (i.e. positions with
	    // corresponding player-to-move already in the move stack);
	    // or compute an evaluation.
	    if (((newposInfo==NULL) && (newposInfo=posHash.newElt(&newPos))) ||
		(!newposInfo.exists(other_player)))
	      {
		ratePositionByComputation(&newPos, other_player, newposInfo);
		depth--;
		++r_positionsEvaluated;
		newposEval = newposInfo.get(other_player);
	      }
	    else if (moveStack.isInMoveStack(newposInfo, other_player))
	      {
		newposEval = positionEval_even;
	      }
	    else
	      newposEval = newposInfo.get(other_player);

	    qComputationTree::qComputationTreeNodeId new_node;
	    new_node = computationTree.addChildNode(currentTreeNode,
						    possible_move,
						    newposEval);
	    computationTree.setNodePosInfo(new_node, newposInfo);
	  }

      }
    else
      {
	// We now know we have a list of possible moves in the computationTree

	// Throwing out any moves that are obviously not "contendors"
	// (i.e. score + complexity < highest score - corresponding complexity)
	// pick which move to refine.
	qComputationTreeNodeId contendingMoveId, bestMoveId, curMoveId;
	qPositionEval *bestEval;
	qPositionEval *curEval;

	bestMoveId = contendingMoveId =
	  computationTree.getTopScoringChild(currentTreeNode);
	g_assert(bestMoveId); // How'd we get in a position with no moves?

	bestEval = computationTree.getNodeEval(bestMoveId);
	maxComplexity = bestEval.complexity;
	minScore = (qScore_lost + bestEval.complexity >= bestEval.score) ?
	  qScore_lost : (bestEval.score - bestEval.complexity);

	// Skim through contendors to pick the best one to refine,
	guint8 n;
	for (n=0;
	     curMoveId = computationTree.getNthChild(currentTreeNode, n);
	     n++)
	  {
	    if (curMoveId == bestMoveId)
	      continue;

	    curEval = computationTree.getNodeEval(curMoveId);
	    if ((curEval.score + curEval.complexity <= minScore) &&
		(curEval.score < qScore_won - curEval.complexity))
	      break; // No more contendors--we've fallen below the threshold

	    // Pick whichever contending move has highest complexity???
	    // Maybe give preference to higher scoring moves,
	    // since they're more likely to be valuable--multply complexity
	    // by delta score/10???
	    if (curEval.complexity > maxComplexity) {
	      contendingMoveId = curMoveId;
	      maxComplexity = curEval.complexity;
	    }
	  }
	g_assert(n>=1);  // We should always get past at least the best move

	// Devote a depth effort proportional to the avail. depth divided
	// by the number of contending moves.
	gint16 scan_depth = depth / n;

	// scan_depth cannot be 0 or we could loop forever.
#if MIN_POSITIONS_EXAMINED_PER_PLY > 0
	if (scan_depth < MIN_POSITIONS_EXAMINED_PER_PLY)
	  scan_depth = MIN_POSITIONS_EXAMINED_PER_PLY;
#else
#ERROR(MIN_POSITIONS_EXAMINED_PER_PLY must be at least 1)
#endif
	possible_move = computationTree.getNodePrecedingMove(contendingMoveId);
	moveStack.pushMove(player2move, possible_move, posInfo);
	currentTreeNode = contendingMoveId;
	scanDeeper(moveStack.getPos(), otherPlayer, scan_depth, NULL, childPositionsComputed);
	r_positionsEvaluated += childPositionsComputed;
	depth -= positionsComputed;
	currentTreeNode = computationTree.getParent(currentTreeNode);
	moveStack.popMove();

	// Now loop back and re-evaluate the current position's options
      }
  } while (depth >= 0);

  // Now combine the scores we've found and return
  posInfo = ratePositionFromNeighbors(pos, player2move, posInfo,
				      p_eval, p_mvlist, p_evalList);
  return posInfo;
}


#if 0
// This code was originally when I was going to have a bg thinker
// thread spawned by the qsearcher object, but I think it's move flexible
// to have a think service func so this code is unused for now.
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
#endif
