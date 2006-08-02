/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qtypes.h"
#include "qposition.h"
#include "qposinfo.h"
#include "qposhash.h"
#include "qmovstack.h"
#include "qdijkstra.h"
#include "qsearcher.h"
#include "getmoves.h"
#include "parameters.h"

IDSTR("$Id: eval.cpp,v 1.11 2006/08/02 04:08:07 bmiller Exp $");


/****/

#define qNO_PATH -1

/* Score bonuses for various things */
static const gint16 qScore_PLY=PLY_SCORE;  // Bonus for player whose turn it is
static const gint16 qScore_TURN=TURN_SCORE;// Bonus for a whole move ahead

/* Our strategy for evaluating a position is based primarily on the current
 * player's number of moves required to reach the end minus the opponent's
 * number of moves to reach his end.  We use the dijkstra algorithm to
 * quickly find these values.
 * We then modify these scores, giving or removing points based on:
 *   number of walls left,
 *   largely varying available paths to the finish.
 */

#ifdef WALL_SCORE_FUDGE
static gint16 wallScoreFudge[][11] = WALL_SCORE_FUDGE;
inline static gint16 WALL_SCORE(guint8 p, guint8 o)
  { return wallScoreFudge[p][o]; };
#else
inline static gint16 WALL_SCORE(guint8 p, guint8 o)
  { return (TURN_SCORE*(p-o)); };
#endif

#ifdef WALL_COMPLEXITY_FUDGE
static guint8 wallComplexityFudge[][11] = WALL_COMPLEXITY_FUDGE;
inline static gint16 WALL_COMPLEXITY(guint8 p, guint8 o)
  { return wallComplexityFudge[p][o]; };
#else
#define WALL_COMPLEXITY(p, o) (PLY_SCORE*((p)+(o)))
inline static gint16 WALL_COMPLEXITY(guint8 p, guint8 o)
  { return PLY_SCORE*(p+o); };
#endif


#if 0 /* Now using default hashFunc defined in qposhash.cpp */
// Passed to qPositionHash
guint16 CBhashFunc(qPosition p)
{
  return p.hashFunc();
}
void CBinitFunc(qPositionInfo posInfo, qPosition p)
{
  posInfo->initEval();
  /* posInfo->pos = *p; position not stored in posInfo */
}
#endif

inline guint16 scoreSpread(gint8 *sortedFinishDistances)
{
  g_assert(sortedFinishDistances && *sortedFinishDistances);
  guint16 spread = 0;
  gint8 prevdist = *sortedFinishDistances;
  ++sortedFinishDistances;
  while (*sortedFinishDistances) {
    spread += square(*sortedFinishDistances - prevdist);
    prevdist = *(sortedFinishDistances++);
  }
  return spread;
}


qPositionEvaluation const *ratePositionByComputation
(qPosition pos, qPlayer player2move, qPositionInfo *posInfo)
/****************************************************************************
 *
 * Examine position and populate *posInfo with score/complexity/computations
 *  of position's evaluation.
 * Fills in posInfo->evaluation[player2move] (and possibly other player too)
 * Returns:
 *  NULL - illegal position
 *  non-NULL - legal position & evaluation completed
 *
 ****************************************************************************/
{
  int     distance[2];
  qPlayer opponent(player2move.getOtherPlayerId());
  qDijkstraArg darg;

#ifdef USE_FINISH_SPREAD
  guint16 spread[2];
  darg.getAllRoutes = TRUE;
#else
  darg.getAllRoutes = FALSE;
#endif

  for (gint8 playerNum=0; playerNum <= qPlayer::BlackPlayer; ++playerNum) {
    qPlayer player(playerNum);
#ifdef HAVE_NUM_COMPUTATIONS
    posInfo->setComputations(player, 1).computations = 1;
#endif
    posInfo->setScore(player, qScore_PLY + 
		      WALL_SCORE(pos.numWallsLeft(player), pos.numWallsLeft(player.otherPlayer())));
    posInfo->setComplexity(player,
      BASE_COMPLEXITY +
      WALL_COMPLEXITY(pos.numWallsLeft(player),
                      pos.numWallsLeft(player.otherPlayer())));

    /* Use Dijkstra algorithm to find shortest path for each player */
    qDarg_CLEAR_CACHE(darg);
    darg.player = player;
    darg.pos    = &pos;

    if (qDijkstra(&darg))
      {
	distance[player.getPlayerId()] = darg.dist[0];
#ifdef USE_FINISH_SPREAD
	spread[player]   = scoreSpread(darg.dist);
#endif
      }
    else
      {
	/* If path to finish could not be found, confirm no path exists by
	 * searching again with opposing pawn removed from the board (maybe
	 * a path exists but is temporarily blocked by opponent)
	 */

	// Snip opponent from position; we'll put the opponent "under" our pawn
	qSquare oldWhite(pos.getWhitePawn());
        qSquare oldBlack(pos.getBlackPawn());
	if (player.isWhite()) {
	  pos.setBlackPawn(oldWhite);
	} else {
	  pos.setWhitePawn(oldBlack);
	}

	// I was tempted to try recycling previous graph used to compute
	// Dijkstra of neighboring positions, and find a way to just compute
	// the modifications to the graph, but decided it was too hard.  This
	// is described in a comment in qcomptree.h

	bool pathFound = qDijkstra(&darg);

	// Put the opposing pawn back
	pos.setWhitePawn(oldWhite);
	pos.setBlackPawn(oldBlack);

	if (!pathFound)
	  {
	    /* Still no path exists; this is not a legal position */
	    posInfo->setPositionIsIllegal();
	    //evaluation[0] = evaluation[1] = {0, 0, 1};
            posInfo->setScore(qPlayer_white, 0);
            posInfo->setScore(qPlayer_black, 0);
            posInfo->setComplexity(qPlayer_white, 0);
            posInfo->setComplexity(qPlayer_black, 0);
#ifdef HAVE_NUM_COMPUTATIONS
            posInfo->setComputations(qPlayer_white, 1);
            posInfo->setComputations(qPlayer_black, 1);
#endif
	    return NULL;
	  }
	// Path exists but is blocked, so our score isn't as accurate.
	//evaluation[player].complexity += BLOCKED_POSITION_FUDGE;
        posInfo->setComplexity(player,
                               posInfo->getComplexity(player) + BLOCKED_POSITION_FUDGE);
      }
  }

  {
    gint16 tmp = qScore_TURN * (distance[1] - distance[0]);

#ifdef USE_FINISH_SPREAD_SCORE
    // Add in a factor for the spread in moves to all avail. end squares,
    // This is only a factor if the opponent actually has walls.
    // Use some multiplier for this???
    // This factor should roughly be the difference between the shortest
    // path the the finish plus the longest path to the finish, plus some
    // multiplier <= 1 times the length of all other paths to the finish.
    //
    // Whatever factor is arrived at, the score should be decreased by
    // approximately half that factor, and the complexity increased by
    // the factor.
    goal_line_spread_factor[0] =
      numWallsLeft(qPlayer_black) ? spread[0] : 0;
    goal_line_spread_factor[1] =
      numWallsLeft(qPlayer_white) ? spread[1] : 0;

    gint16 score_adjust_white =
      (goal_line_spread_factor[0] - goal_line_spread_factor[1])/2;
    gint16 score_adjust_black =
      (goal_line_spread_factor[1] - goal_line_spread_factor[0])/2;
    gint16 complexity_adjust =
      goal_line_spread_factor[0] + goal_line_spread_factor[1];
#else
    const gint16 score_adjust_white = 0;
    const gint16 score_adjust_black = 0;
    const gint16 complexity_adjust = 0;
#endif

    // !!! This spread (modified according to # walls opponent has) should
    // be a large factor in the complexity of a position.  Other factors
    // adding to complexity should be existence of "pawn collisions" along
    // path, and perhaps number of moves required to reach finish and
    // number of possible finishing squares(?)???

    // Store the scores for each player
    if (distance[0] <= 1) {
      //evaluation[0].score        = qScore_won;
      //evaluation[0].complexity   = 0;
      posInfo->setScore(qPlayer_white, qScore_won);
      posInfo->setComplexity(qPlayer_white, 0);
#ifdef HAVE_NUM_COMPUTATIONS
      //evaluation[player].computations = 1;
      posInfo->setComputations(player2move, 1);
#endif
    } else {
      posInfo->setScore(     qPlayer_white,
                             posInfo->getScore(qPlayer_white) + tmp);
      posInfo->setComplexity(qPlayer_white,
                             posInfo->getComplexity(qPlayer_white) + complexity_adjust);
    }

    if (distance[1] <= 1) {
      posInfo->setScore     (qPlayer_black, qScore_won);
      posInfo->setComplexity(qPlayer_black, 0);
#ifdef HAVE_NUM_COMPUTATIONS
      posInfo->setComputations(qPlayer_black, 1);
#endif
    } else {
      posInfo->setScore     (qPlayer_black,
                             posInfo->getScore(qPlayer_black) - tmp);
      posInfo->setComplexity(qPlayer_black,
                             posInfo->getComplexity(qPlayer_black) + complexity_adjust);
    }
  }
  return posInfo->get(player2move);
}

inline void coalesceScores(const qPositionEvaluation &bestEval,
                           const qPositionEvaluation &currEval,
                           qPositionEvaluation       &newEval)
{
  // This protocol needs major tweaking...it should take into account
  // things like who is winning for how much to weigh complexity, etc.
  gint32 newVal =
    newEval.score - (min(static_cast<gint32>(currEval.score)+currEval.complexity, static_cast<gint32>(bestEval.score)+bestEval.complexity) - (currEval.score-currEval.complexity)) / (1+2*currEval.complexity + currEval.score - bestEval.score);
  if (newVal < qScore_lost)
    newEval.score = qScore_lost;
  else 
    newEval.score = static_cast<gint16>(newVal);

  newVal = newEval.complexity + (3*static_cast<gint32>(currEval.complexity)) / (10 + static_cast<gint32>(currEval.score) - bestEval.score);
  if (newVal > qComplexity_max)
    newEval.complexity = qComplexity_max;
  else 
    newEval.complexity = static_cast<guint16>(newVal);
}

qPositionInfo    *ratePositionFromNeighbors
(const qPosition *pos,
 qPlayer          player2move,
 qPositionInfo   *posInfo,
 qEvalIterator   *evalItor)
/****************************************************************************
 *
 * Examine position and populate this->evaluation[player2move] with
 *  score/complexity/computations of position's evaluation.
 * Fills in this->evaluation[player2move];
 *
 ****************************************************************************/
{
  //if (!posInfo)
  //   posInfo = posHash.getOrAddElt(pos));
  g_assert(posInfo);

  qPositionEvaluation *newEval = posInfo->get(player2move);

  // Look up eval of each legal move
  //   (or zero score if resulting position seen)
  //   rateByComputation for new positions

  // Can this be done in one pass??? */
  if (!evalItor)
    return NULL;

 list<const qPositionEvaluation *> scoreList;

 if (evalItor->atEnd())
   { // No neighbors--we have no legal moves!  Let's call it a draw
     newEval->score = 0;
     newEval->complexity = 0;
#ifdef HAVE_NUM_COMPUTATIONS
     newEval->computations = 0;
#endif
     g_assert(0); // This shouldn't have happened, so let's take a look.
     return posInfo;
   }
  
  // 1. Find move that gives opponent worst evaluation
  // Keep bestMove always at the front of the list
  const qPositionEvaluation *bestMove, *currMove;
  bestMove = evalItor->val();
  scoreList.push_front(bestMove), evalItor->next();
  for ( ;
       !evalItor->atEnd();
       evalItor->next())
    {
      currMove = evalItor->val();
      // Is score alone the way to find the best move?  We should probably
      // Take complexity and who is winning into account.  Especially because
      // a complexity-zero win is always better than a non-zero complexity
      if (currMove->score < bestMove->score) {
        bestMove = currMove;
        scoreList.push_front(currMove);
      } else {
        scoreList.push_back(currMove);
      }
    }

  // 2. Combine evals into current positions eval.
  //    Take into accont minmax of last two plies???
  *newEval = *bestMove;

  // If we have a winning move, we're done (do this comparison in above loop???)
  if (bestMove->score == qScore_lost) {
    newEval->score = qScore_won;
    return posInfo;
  }
  // If the best we could do was losing, then forget it; we've lost
  if (bestMove->score == qScore_won) {
    newEval->score = qScore_lost;
    return posInfo;
  }

  scoreList.pop_front();
  while(!scoreList.empty()) {
    currMove = scoreList.back();
    scoreList.pop_back();
    if (currMove->score == qScore_won)
      break; // This sure isn't going to improve things for us
    if ((currMove->score - currMove->complexity) <
        (bestMove->score + bestMove->complexity))
      continue;

    coalesceScores(*bestMove, *currMove, *newEval);
  }

  newEval->score = -newEval->score;
  /*
  Score should be the negation of opponent's best move.  Perhaps the score
    should be decreased slightly with the number of opponent's moves whose
    complexity allows a probability of exceeding the "best" move's score.
  Complexity should increase with the number of opponent's moves whose
    "range" gives them a chance of exceeding the "best" move's range.
    Opponent's moves whose range peeks into the current range without
    exceeding it should cause the opponent's score to increase slightly
    while decreasing complexity slightly.  (i.e. there is a lower chance
    of the opponent's moving being at the bottom end of his range.)
  Base these overlaps on empirical evidence of within what range around
    rated scores positions ended up in relative to the rated complexity???
  Instead of sorting based on score, sort based on the score "range's" max???
    This would tell us which moves have an effect.
  It would be nice if the process was cumative:
    eval(a, eval(b,c))==eval(eval(a,b), c)

  What we really want here is, if move x gives a score probability
    distribution with mean Mx and standard deviation SDx, and move y
    give My & SDy, what are Mz and SDz if z represents max(X,Y)?
  */

  /* Short-term solution:  pick move with highest score
   * new score = that move's score
   * new complexity = that move's complexity * .7 (+/- fudge factors)
   */

#if 0
  score = ???;
  computations = sum of neighbors computations;
  complexity = ???; /* set to 0 for solved win/loss/draw
		     * higher for significant scores with high complexity or
		     * lots of opponents options with high complexity
		     */
#endif

  // 5. Return all data so calling routine can decide if further
  //    evaluation is needed.
  // Note:  if moveList/evalList are populated when called (maybe with
  //        some evalList items NULLed out), leverage those values???
  return posInfo;
}

qPositionInfo *ratePositionFromNeighbors
(const qPosition   *pos,
 qPlayer            player2move,
 qPositionInfo     *posInfo,      // optional optimization
 qPositionInfoHash *posHash,
 qMoveStack        *moveStack)
{
  //std::auto_ptr<qMoveList> mvList(new mvList(NULL));
  qMoveList      possMoves;

  if (!posInfo)
     posInfo = posHash->getOrAddElt(pos);

  // Create an evalItor from pos
  getPlayableMoves(pos, moveStack, &possMoves);

  qEvalItorFromMvContainer<qMoveList>
     evalItor(&possMoves, pos, player2move, posHash);

  return ratePositionFromNeighbors(pos, player2move, posInfo, &evalItor);
}
