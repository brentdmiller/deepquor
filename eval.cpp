/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include <qtypes.h>
#include <qposition.h>
#include <qposinfo.h>
#include <qposhash.h>
#include <qmovstack.h>
#include <parameters.h>

IDSTR("$Id: eval.cpp,v 1.3 2005/11/19 08:22:33 bmiller Exp $");

#define qNO_PATH -1

/* Score bonuses for various things */
#define qScore_PLY   PLY_SCORE  /* Bonus for the player whose turn it is */
#define qScore_TURN  TURN_SCORE /* Bonus for a whole move ahead          */
#define qScore_WALL  TURN_SCORE /* Maybe this should be a function???    */

/* Our strategy for evaluating a position is based primarily on the current
 * player's number of moves required to reach the end minus the opponent's
 * number of moves to reach his end.  We use the dijkstra algorithm to
 * quickly find these values.
 * We then modify these scores, giving or removing points based on:
 *   number of walls left,
 *   largely varying available paths to the finish.
 */

#ifdef WALL_SCORE_FUDGE
static gint16 wallScoreFudge[][] = WALL_SCORE_FUDGE;
#define WALL_SCORE(p, o) (wallScoreFudge[(p)][(o)])
#else
#define WALL_SCORE(p, o) (qScore_WALL * ((p) - (o)))
#endif

#ifdef WALL_COMPLEXITY_FUDGE
static guint8 wallComplexityFudge[] = WALL_COMPLEXITY_FUDGE;
#define WALL_COMPLEXITY(p, o) (wallComplexityFudge[(p)])
#else
#define WALL_COMPLEXITY(p, o) (PLY_SCORE*((p)+(o)))
#endif


// Passed to qPositionHash
guint16 CBhashFunc(qPosition p)
{
  return p.hashFunc();
}
void CBinitFunc(qPositionInfo posInfo, qPosition p)
{
  posInfo->clearEval(p); ???
  posInfo->pos = *pos;
}


typedef struct _qDijkstra {
  int      moves;  // out: returned number of moves required
  guint16  spread; // out: rating of how spread apart possible finishes are

  /* Maybe allow the following
   * bool          useCachedGraph;
   * qCalcGraphRec graph;
   */
} qDijkstraArg;
#define qDarg_CLEAR_CACHE(d) ((d).useCachedGraph=False) (1)

QDecl(bool, qDijkstra, (qPlayer, qDijkstraOut*));

bool qPositionHashElt::ratePositionByComputation
(qPlayer player2move, qPosition pos, qPositionInfo *posInfo)
/****************************************************************************
 *
 * Examine position and populate *out with score/complexity/computations
 *  of position's evaluation.
 * Fills in this->evaluation[player2move] (and possibly other player too)
 * Returns:
 *  True  - legal position & evaluation completed
 *  False - illegal position
 *
 ****************************************************************************/
{
  int     distance[2];
  guint16 spread[2];
  qDijkstraArg darg;
  qPlayer opponent(player2move.getOtherPlayerId);

  for (int player=0; player<=BLACK; ++player) {
    evaluation[player].computations = 1;
    evaluation[player].complexity = BASE_COMPLEXITY +
      WALL_COMPLEXITY(numWallsLeft(player), numWallsLeft(opponent)));

    /* Use Dijkstra algorithm to find shortest path for each player */
    qDarg_CLEAR_CACHE(darg);

    if (qDijkstra(player, &darg))
      {
	distance[player] = darg.score;
	spread[player]   = darg.spread;
      }
    else
      {
	/* If path to finish could not be found, confirm no path exists by
	 * searching again with opposing pawn removed from the board (maybe
	 * a path exists but is temporarily blocked by opponent)
	 */

	// Snip opponent from position; we'll put the opponent "under" our pawn
	guint8 oldWhite, oldBlad;
	oldWhite = pos.getWhitePos;
	oldBlack = pos.getBlackPos;
	if (player==WHITE) {
	  pos.setBlackPos(oldWhite);
	} else {
	  pos.setWhitePos(oldBlack);
	}

	// Use cached calculation grid???
	// Update cached calc graph to reflect removed opponent & recalc
	/*(???);*/

	bool pathFound;
	pathFound = qDijkstra(player, &darg);

	// Put the opposing pawn back
	pos.setWhitePos(oldWhite);
	pos.setBlackPos(oldBlack);

	if (!pathFound)
	  {
	    /* Still no path exists; this is not a legal position */
	    setPositionIsIllegal();
	    evaluation[0] = evaluation[1] = {0, 0, 1};
	    return False;
	  }
	// Path exists but is blocked, so our score isn't as accurate.
	evaluation[player].complexity += BLOCKED_POSITION_FUDGE;
    }
  }

  {
    int tmp = qScore_TURN * (distance[player] - distance[1-player]) +
      (player==WHITE ?
       WALL_SCORE(pos.numWhiteWallsLeft(), pos.numBlackWallsLeft()) :
       WALL_SCORE(pos.numBlackWallsLeft(), pos.numWhiteWallsLeft()));

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
    goal_line_spread_factor[player] =
      numWallsLeft(opponent) ? spread[player] : 0;
    goal_line_spread_factor[opponent] =
      numWallsLeft(player) ? spread[opponent] : 0;

    score_adjust =
      (goal_line_spread_factor[player] - goal_line_spread_factor[opponent])/2;
    complexity_adjust =
      goal_line_spread_factor[player] + goal_line_spread_factor[opponent];

    tmp = tmp - score_adjust;


    // !!! This spread (modified according to # walls opponent has) should
    // be a large factor in the complexity of a position.  Other factors
    // adding to complexity should be existence of "pawn collisions" along
    // path, and perhaps number of moves required to reach finish and
    // number of possible finishing squares(?)???

    // Store the scores for each player
    if (distance[player2move] <= 1) {
      evaluation[player].computations = 1;
      evaluation[player].complexity   = 0;
      evaluation[player].score        = qScore_won;
    } else {
      evaluation[player].score   = 32 + tmp;
      evaluation[player].complexity = base_complexity + complexity_adjust;
    }

    if (distance[opponent] <= 1) {
      evaluation[opponent].computations = 1;
      evaluation[opponent].complexity   = 0;
      evaluation[opponent].score        = qScore_won;
    } else {
      evaluation[opponent].score = 32 - tmp;
      evaluation[opponent].complexity = base_complexity + complexity_adjust;
    }
  }
  return True;
}

qPositionInfo *ratePositionFromNeighbors
(qPosition     *pos,
 qPlayer        player2move,
 qPositionInfo *posInfo,      // optional optimization
 qPositionEvaluation *peval,     // populated upon return
                      moveList,  // populated upon return
                      evalList)  // populated upon return
/****************************************************************************
 *
 * Examine position and populate *out with score/complexity/computations
 *  of position's evaluation.
 *  in the call to EvaluateDraw() in search()/quiesce().
 * Fills in this->evaluation[player2move];
 *
 ****************************************************************************/
{
  // 1. Calculate all legal moves
  // 2. Lookup eval of each legal move
  //    (or zero score if resulting position seen)
  //    rateByComputation for new positions
  // 3. Sort all evals (how?  score + complexity/20?)
  // 4. Combine evals into current positions eval.
  //    ??? Take into accont minmax of last two plies?

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

  score = ???;
  computations = sum of neighbors computations;
  complexity = ???; /* set to 0 for solved win/loss/draw
		     * higher for significant scores with high complexity or
		     * lots of opponents options with high complexity
		     */
  // 5. Return all data so calling routine can decide if further
  //    evaluation is needed.
  // Note:  if moveList/evalList are populated when called (maybe with
  //        some evalList items NULLed out), leverage those values???


  /*
   * 1. Get all possible moves
   */
  // Get all possible wall locations (some may be illegal moves)
  glist wallVacancies = moveStack.getPossibleWallLocations();
  // Examine possible pawn moves for player
  (???);

  /*
   * 2. evaluate each move
   */
  
}
