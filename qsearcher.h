/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

#ifndef INCLUDE_searcher_h
#define INCLUDE_searcher_h


#include "qtypes.h"
#include "qposition.h"
#include "qposinfo.h"
#include "qposhash.h"
#include "qmovstack.h"
#include "parameters.h"

IDSTR("$Id: qsearcher.h,v 1.3 2005/11/19 08:22:33 bmiller Exp $");

/* Given a position, searches, within specified constraints, for the
 * best possible move.
 */
class qSearcher {
public:

  // Initiates a searcher object with bg thinking on (but bg thinking does
  // not start until search is called the first time)
  qSearcher();          // Normal initial position
  qSearcher(qPosition pos, qPlayer player2move); // Initial position specified
  ~qSearcher();

  // Tell me what the best move is
  // This routing does not apply the move to the stored position
  // Criteria:
  //   max_complexity:  255 for no max, 0 for "solve position mode"
  //   min_depth:  Examine at least this # of plies for any returned move
  //   min_breadth:  force breadth-1st search through this # plies from start
  //                 0 is equivalent to 1 (we always search neighbors)
  //   max_time: thinking time, in seconds.  0 = no time limit
  //   suggested_time: thinking time, in seconds.  0 = no limit
  qMove search(qPlayer player2move,     // Which player to find a move for
	       guint8  max_complexity,  // keep thinking until below
	       guint8  min_depth,       // keep thinking until beyond
	       guint8  min_breadth,     // brute force search this many plies
               gint32  max_time,        // Hard limit on our avail. time
	       gint32  suggested_time); // Start relaxing criteria after this

  // Adjust qSearcher's stored position with this move
  applyMove(qMove mv, qPlayer p);

  // Do a "unit" of thinking (used for bg thinking); use as a service call
  void think(qPlayer player2move);

private:
  typedef qPositionHash<qPosition, qPositionInfo> qPosInfoHash;

  qPosInfoHash posHash;     // Where we store everything we've thought about

   // Where we store what we're thinking about
  qMoveStack   moveStack;
  qComputationTree computationTree;
  qComputationTree::qComputationTreeNodeId currentTreeNode;

  // In cases of repeated positions or draws, this is useful for returning
  // a score evaluation that is not in the positionHash.
  static const qPositionEval even_evaluation;

  // Internal search routine used by both search() and background searches
  qMove iSearch(qPosition pos,
		qPlayer player2move,     // Which player to find a move for
		guint8  max_complexity,  // keep thinking until below
		guint8  minbreadth,      // breadth-1st through this many plies
		guint8  mindepth,        // keep thinking until beyond
		gint32  maxtime,         // Hard limit on our avail. time
		gint32  suggested_time); // Ignore constraints after this

  /* scanDeeper
   * If depth < 0, brute-force examine every position w/in abs(depth) plies.
   * If depth > 0, then depth limits approximately how many new positions
   *   to add to the current analysis, before returning for re-evaluation of
   *   criteria.  Examining larger numbers of positions avoids going up and
   *   down the stack a lot, but adding lots of evaluation to a losing line is
   *   fruitless.  The more analysis we've done on a position, the higher
   *   this number should probably become (maybe starting at 200 and going
   *   up to 300 or 400(?) eventually, as we begin to evaluate
   *   positions with larger number of contibuting computations.
   */
  qPositionEval scanDeeper(qPosition     *pos,
			   qPlayer        player2move,
			   gint32         depth,
			   // qPositionEval *evalToBeat,
			   // ??? (carry both alpha & beta?)
			   guint32       &r_positionsEvaluated);

  /* Routines for computing & setting this position's score/depth */
  // Defined in eval.cpp:
  gint16 ratePositionByComputation(qPlayer player2move);


};

#endif // INCLUDE_searcher_h
