/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qsearcher.h"
#include "getmoves.h"
#include <memory>
#include <sys/time.h>

IDSTR("$Id: qsearcher.cpp,v 1.7 2006/07/15 05:16:38 bmiller Exp $");

// Convenience utility
guint32 milliseconds_since2000(void);


/* class qCompTreeChildEdgeEvalIterator - used internally
 */
class qCompTreeChildEdgeEvalIterator:public qEvalIterator {
private:
  qComputationTree *compTree;
  qComputationTree::qComputationTreeNodeId nodeId;
  guint8 edgeIdx;

public:
  // Walks through position in a qCompTree node
  qCompTreeChildEdgeEvalIterator(qComputationTree *tree,
				qComputationTree::qComputationTreeNodeId node)
  {
    compTree = tree;
    nodeId   = node;
    edgeIdx  = 0;
  };
  ~qCompTreeChildEdgeEvalIterator() {;};

  void next() {
    ++edgeIdx;
  };

  qPositionEvaluation const *val() {
    qComputationTree::qComputationTreeNodeId childId =
      compTree->getNthChild(nodeId, edgeIdx);
    if (childId == qComputationTree::qComputationTreeNode_invalid)
      return NULL;
    return compTree->getNodeEval(childId);
  };
  bool atEnd() {
    return (compTree->getNthChild(nodeId, edgeIdx)==qComputationTree::qComputationTreeNode_invalid) ? FALSE : TRUE;
  };
};

/*******************
 * class qSearcher *
 ******************/
void
qSearcher::think
(qPlayer player2move,
 gint32 thinkAmount)
{
  qMove move;

#if 0  // Keep a move count, and if enough moves have passed, do this???
  if (wallMovesSinceTableUpdate > 5)
    {
      moveStack.initWallMoveTable();
      wallMovesSinceTableUpdate = 0;
      return;
    }
#endif

  // call iSearch
  move = iSearch(player2move,
		 qComplexity_max,
		 thinkAmount,
		 0,
		 0,
		 MAXTIME_PER_THINK_SERVICE,  // Just to avoid endless hangs
		 SUGTIME_PER_THINK_SERVICE);


  return;
}

qMove
qSearcher::search
(qPlayer player2move,
 guint8 max_complexity,
 guint8 min_depth,
 guint8 min_breadth,
 guint8 slop,
 gint32 max_time,
 gint32 suggested_time)
{
  qMove move;

  // No longer relevant code; this class uses a service func for bg thinking:
  // Stop bg searcher
  // bgStop();

  // call iSearch
  move = iSearch(player2move,
		 max_complexity,
		 min_depth,
		 min_breadth,
		 slop,
		 max_time,
		 suggested_time);

  // Update position/bgPlayerToMove with latest move
  //  bgPos.applyMove(player2move, move);
  //
  // Restart bg searcher
  // bgPlayerToMove = qPlayer(player2move.getOtherPlayerId);
  //
  // bgGo();

  return move;
}

void
qSearcher::applyMove
(qMove mv,
 qPlayer p)
{
  moveStack.pushMove(p, mv);

  // Keep a move count???
  if (mv.isWallMove())
    wallMovesSinceTableUpdate++;
}


qMove
qSearcher::iSearch
(qPlayer player2move,
 guint8  max_complexity,
 guint8  min_depth,
 guint8  min_breadth,
 guint8  slop,
 gint32  max_time,
 gint32  suggested_time)
{
  gint8 current_depth = 0;
  guint32 positionsEvaluated = 0;
  qMove   bestMove;
  guint32 stop_time;     // an "alarm" for when to stop & check conditions
  guint32 current_time;

  // Figure out how long to think
  {
    gint32 think_time = suggested_time;
    current_time = milliseconds_since2000();

    if ((!think_time) || (max_time < think_time))
      think_time = max_time;
    stop_time = current_time + think_time;
  }

  computationTree.initializeTree();
  currentTreeNode = computationTree.getRootNode();

  // Start off with a breadth first search up through some minimum number of
  // plies
  // i.e. for some depth n, dive down and evaluate every possible move
  // at depth n and percolate the scores back up. (make this an arg such
  // as comprehensive_depth???)  This could actually be a mode in the while
  // loop:  while we're in breadth mode, call a depth routine for every
  // possible move with the depth set to some number.
  // (depth < 0 could mean just go to a leaf, and then extend the depth
  // by "depth" plies); depth>0 could mean simply evaluate all options
  // (breadth first) through depth "depth")
  if (min_breadth) {
    scanDeeper(moveStack.getPos(),
	       player2move,
	       -min_breadth,
	       positionsEvaluated);
  }


  // Go into a loop refining our evaluation until it's good enough

  while (1) {
    // Check criteria for if we've done enough to decide on a move

    // 1. Has time expired?
    // If hard limit has expired, return best chosen move
    // If soft limit has expired, loosen criteria & continue
    {
      current_time = milliseconds_since2000();

      if (current_time >= stop_time) {
	stop_time = current_time + max_time;

	// If we're beyond the hard max time, just return
	if (current_time >= stop_time)
	  break;

	// We weren't beyond the max time; soften criteria
	max_complexity = 255;
	min_breadth    = 0;
	min_depth      = 1;
	slop          += 10; // No basis for choosing this???
      }
    }

    // Find current top-scoring move
    qPositionEvaluation const *bestEval;
    {
      qComputationTree::qComputationTreeNodeId bestPosId;

      bestPosId = computationTree.getTopScoringChild(currentTreeNode);

      bestMove = computationTree.getNodePrecedingMove(bestPosId);
      bestEval = computationTree.getNodeEval(bestPosId);
    }

    // 2. Is top move complexity 0 forced win?
    // Yes: make best move
    if (bestEval->score == qScore_won) {
      return bestMove;
    }

    // 3. Is top move complexity 0 forced loss?
    // Yes: make move with most computations (if we have computations avail;
    // otherwise just make any move)
    if (bestEval->score == qScore_lost) {
#if HAVE_NUM_COMPUTATIONS
      // Find move with most computations
      return move_with_most_computations;
#endif
      return bestMove;
    }

    // 4. Keep thinking if we haven't achieved minimum complexity
    if (bestEval->complexity > max_complexity)
      goto analyzeMore;

    // 5. Keep thinking if we haven't achieved minimum depth
    /* How do we test depth???
     * if (bestEval->depthOfAnalysis < min_depth)
     *   goto analyzeMore;
     */

    // 6. Is complexity of all contending moves below some minimum threshold?
    //   Yes: return bestmove; not worth further evaluation
    {
      gint16 minScore;
      qComputationTree::qComputationTreeNodeId curPosId;
      qPositionEvaluation const *curEval;
      bool worthRefining = FALSE;

      minScore = (qScore_lost + bestEval->complexity >= bestEval->score) ?
	qScore_lost : (bestEval->score - bestEval->complexity);

      guint8 n = 0;

      if (slop) {
	for (;
	     curPosId = computationTree.getNthChild(currentTreeNode, n);
	     n++)
	  {
	    // if (curPosId == bestPosId)
	    //   continue;

	    curEval = computationTree.getNodeEval(curPosId);

	    if ((curEval->score + curEval->complexity <= minScore) &&
		(curEval->score < qScore_won - curEval->complexity))
	      break; // No more contendors

	    if (curEval->complexity > slop) {
	      worthRefining = TRUE;
	      break; // 
	    }
	  }

	if (!worthRefining) {
	  // All the moves are so even that we may as well pick any
	  break;  // Break out and return a move
	}
      }

      // 5. Among moves in contention, is bestMove only move left?
      // Yes:
      //   Have we met minimum depth yet? (How do we know???)
      //   Yes: return bestMove
      //   No: Further evaluate best move (How do we implement this???)
      if (n <= 1) {
	// If there's only one move left in contention, then this will
	// give us a move out of the score range:
	curPosId = computationTree.getNthChild(currentTreeNode, 1);
	curEval = computationTree.getNodeEval(curPosId);
	if ((curEval->score + curEval->complexity <= minScore) &&
	    (curEval->score < qScore_won - curEval->complexity))
	  {
	    // Not even child 1 is a contendor--there's only one (child 0).
	    break;  // Break out and return a move
	  }
      }
    }

    // If we still didn't find a move to return, do some more thinkin'
  analyzeMore:

    /* Make this intelligently determined??? See comment at the
     * scanDeeper decl in qsearcher.h
     */
#define POSITIONS_PER_DIVE 200

    scanDeeper(moveStack.getPos(),
	       player2move,
	       POSITIONS_PER_DIVE,
	       positionsEvaluated);

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
  g_assert(bestMove.exists());
  return bestMove;
}

/* scanDeeper
 */  
const qPositionEvaluation *qSearcher::scanDeeper
(const qPosition *pos,
 qPlayer          player2move,
 gint32           depth,
 // qPositionEval *evalToBeat,
 guint32         &r_positionsEvaluated)
{
  qPositionInfo *posInfo;
  guint32        childPositionsComputed;
  qPlayer otherPlayer=qPlayer(player2move.getOtherPlayerId());

  r_positionsEvaluated = 0;

  // See if there's existing position info stored
  if (!(posInfo = computationTree.getNodePosInfo(currentTreeNode))) {
    if (posInfo = posHash.getElt(pos))
      computationTree.setNodePosInfo(currentTreeNode, posInfo);
  }

  // Check that we have a posInfo, and that it's not a repeat
  if (!posInfo)
    {
      posInfo = posHash.addElt(pos);
      g_assert(posInfo);
      computationTree.setNodePosInfo(currentTreeNode, posInfo);
    }
  else
    {
      // Check if this position is already in the move stack
      if (moveStack.isInEvalStack(posInfo, player2move))
	return positionEval_even;
    }

  // If we're at the end of a search, return the position's existing
  // evalutation (or make one if necessary)
  if (depth == 0) {
    if (!posInfo->evalExists(player2move)) {
      // Check return val???
      ratePositionByComputation(*pos, player2move, posInfo);
      // Let the caller do this:
      // computationTree.setNodeEval(currentTreeNode, posInfo->get(player2mv));
      ++r_positionsEvaluated;
    }
    return posInfo->get(player2move);;
  }

  // If we don't have an evaluation for the current position
  // do a shallow check if it's already a won position
  if (!posInfo->evalExists(player2move))
    {
      if (pos->isWon(player2move)) {
	posInfo->set(player2move, positionEval_won);
	return positionEval_won;
      } else if (pos->isLost(player2move)) {
	posInfo->set(player2move, positionEval_lost);
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
      if (posInfo->getComplexity(player2move) == 0)
	return posInfo->get(player2move);
    }

  if (depth < 0)
    {
      guint8 move_idx;
      qComputationTree::qComputationTreeNodeId next_position_id;

      // Make sure we've stored the list of moves in computationTree
      if (!computationTree.getNthChild(currentTreeNode, 0)) {
	qMoveList possible_moves; // Initially empty
	qMoveListIterator i;

	getPlayableMoves(pos, &moveStack, &possible_moves);

	// Should we prune when doing brute force search???
	// Doing so blocks us from being 100% thorough for "analysis modes"
	// We probably won't be using brute force for analysis anyway.
	pruneUselessMoves(pos, &possible_moves);

	for (i  = possible_moves.begin();
	     i != possible_moves.end();
	     i++)
	  computationTree.addNodeChild(currentTreeNode,
				       *i,
				       positionEval_none);
      }

      for (move_idx=0;
	   next_position_id = computationTree.getNthChild(currentTreeNode,
							   move_idx);
	   move_idx++) {
	qMove possible_move = computationTree.getNodePrecedingMove(next_position_id);

	moveStack.pushEval(posInfo, &posHash, player2move, possible_move, NULL);
	currentTreeNode = next_position_id;
	scanDeeper(moveStack.getPos(), otherPlayer, depth++, childPositionsComputed);
	r_positionsEvaluated += childPositionsComputed;
	depth -= childPositionsComputed;
	currentTreeNode = computationTree.getNodeParent(currentTreeNode);
	moveStack.popEval();
      }
      {
	qCompTreeChildEdgeEvalIterator itor(&computationTree, currentTreeNode);
	ratePositionFromNeighbors(pos, player2move, posInfo, &itor);
      }
      return posInfo->get(player2move);
    }

  // Else we are extending existing analysis (depth > 0)
  do {

    // Make sure we have list of possible moves, with an evaluation of
    // each possible move (so that we can compute score from neighbors).
    if (!computationTree.getNthChild(currentTreeNode, 0))
      {
	qMoveList possible_moves; // Initially empty
	qMoveListIterator i;

	getPlayableMoves(pos, &moveStack, &possible_moves);

	// There should there be a way to bypass pruning for analysis mode
	pruneUselessMoves(pos, &possible_moves);

	for (i  = possible_moves.begin();
	     i != possible_moves.end();
	     i++)
	  {
	    qMove possible_move = *i;
	    qPosition newPos = *pos;
	    newPos.applyMove(player2move, possible_move);

	    qPositionEvaluation const *newposEval;
	    qPositionInfo *newposInfo = posHash.getElt(&newPos);

	    // See if we need to compute an evaluation.
	    // Use existing evaluations; use even_evaluation for
	    // cycling back to repeated positions (i.e. positions with
	    // corresponding player-to-move already in the move stack);
	    // or compute an evaluation.
	    if (((newposInfo==NULL) && (newposInfo=posHash.addElt(&newPos))) ||
		(!newposInfo->evalExists(otherPlayer)))
	      {
		ratePositionByComputation(newPos, otherPlayer, newposInfo);
		depth--;
		++r_positionsEvaluated;
		newposEval = newposInfo->get(otherPlayer);
	      }
	    else if (moveStack.isInEvalStack(newposInfo, otherPlayer))
	      {
		newposEval = positionEval_even;
	      }
	    else
	      newposEval = newposInfo->get(otherPlayer);

	    qComputationTree::qComputationTreeNodeId new_node;
	    new_node = computationTree.addNodeChild(currentTreeNode,
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
	qComputationTree::qComputationTreeNodeId contendingMoveId, bestMoveId, curMoveId;
	qPositionEvaluation const *bestEval;
	qPositionEvaluation const *curEval;
	gint16 minScore;

	bestMoveId = contendingMoveId =
	  computationTree.getTopScoringChild(currentTreeNode);
	g_assert(bestMoveId); // How'd we get in a position with no moves?

	bestEval = computationTree.getNodeEval(bestMoveId);
	guint16 maxComplexity = bestEval->complexity;
	minScore = (qScore_lost + bestEval->complexity >= bestEval->score) ?
	  qScore_lost : (bestEval->score - bestEval->complexity);

	// Skim through contendors to pick the best one to refine,
	guint8 n;
	for (n=0;
	     curMoveId = computationTree.getNthChild(currentTreeNode, n);
	     n++)
	  {
	    if (curMoveId == bestMoveId)
	      continue;

	    curEval = computationTree.getNodeEval(curMoveId);
	    if ((curEval->score + curEval->complexity <= minScore) &&
		(curEval->score < qScore_won - curEval->complexity))
	      break; // No more contendors--we've fallen below the threshold

	    // Pick whichever contending move has highest complexity???
	    // Maybe give preference to higher scoring moves,
	    // since they're more likely to be valuable--multply complexity
	    // by delta score/10???
	    if (curEval->complexity > maxComplexity) {
	      contendingMoveId = curMoveId;
	      maxComplexity = curEval->complexity;
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
	qMove possible_move =
	  computationTree.getNodePrecedingMove(contendingMoveId);
	moveStack.pushEval(posInfo, NULL, player2move, possible_move, NULL);
	currentTreeNode = contendingMoveId;
	scanDeeper(moveStack.getPos(), otherPlayer, scan_depth, childPositionsComputed);
	r_positionsEvaluated += childPositionsComputed;
	depth -= childPositionsComputed;
	currentTreeNode = computationTree.getNodeParent(currentTreeNode);
	moveStack.popEval();

	// Now loop back and re-evaluate the current position's options
      }
  } while (depth >= 0);

  // Now combine the scores we've found and return
  {
    qCompTreeChildEdgeEvalIterator itor(&computationTree, currentTreeNode);
    posInfo = ratePositionFromNeighbors(pos, player2move, posInfo,
			      &itor);
  }

  return posInfo->get(player2move);
}

/* Secs since a fixed point in time near 1/1/2000 */
guint32 milliseconds_since2000()
{
  struct timeval t;

  if (gettimeofday(&t, NULL))
    return 0;

#define SECS_FROM_EPOCH_TO_2000 946706400

  return 1000*(t.tv_sec - SECS_FROM_EPOCH_TO_2000) + (t.tv_usec/1000);
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
