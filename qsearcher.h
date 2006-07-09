/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qsearcher.h,v 1.5 2006/07/09 06:37:38 bmiller Exp $

#ifndef INCLUDE_searcher_h
#define INCLUDE_searcher_h 1


#include "qtypes.h"
#include "qposition.h"
#include "qmovstack.h"
#include "qposinfo.h"
#include "qposhash.h"
#include "qcomptree.h"

/* Given a position, searches, within specified constraints, for the
 * best possible move.
 * MT note:  the qSearcher constructor should take a poshash as an arg
 * and bind to it.  This would allow many qSearchers contributing to a
 * single posHash.   ???
 */
class qSearcher {
public:

  // Initiates a searcher object with bg thinking on (but bg thinking does
  // not start until search is called the first time)
  qSearcher();          // Normal initial position
  qSearcher(qPosition pos, qPlayer player2move); // Initial position specified
  ~qSearcher();

  // Tell me what the best move is
  // This routine does not apply the move to the stored position
  // Criteria:
  //   max_complexity:  qComplexity_max for no max, 0 for "solve position mode"
  //   min_depth:  Examine at least this # of plies for any returned move
  //   min_breadth:  force breadth-1st search through this # plies from start.
  //                 Useful to front-load checking all moves if there's time.
  //   slop: Don't bother further refining evaluations if the range of
  //         possible scores are within 2*slop of the current best score.
  //         Calling func should base slop on time, score, etc.
  //   max_time: thinking time, in milliseconds.  0 = no time limit
  //   suggested_time: thinking time, in milliseconds.  0 = no limit
  qMove search(qPlayer player2move,     // Which player to find a move for
	       guint8  max_complexity,  // keep thinking until below
	       guint8  min_depth,       // keep thinking until beyond
	       guint8  min_breadth,     // brute force search this many plies
	       guint8  slop,            // don't need to refine beyond this
               gint32  max_time,        // Hard limit on our avail. time
	       gint32  suggested_time); // Start relaxing criteria after this

  // Adjust qSearcher's stored position with this move
  void applyMove(qMove mv, qPlayer p);

  // Do a "unit" of thinking (used for bg thinking); use as a service call
  // If specified, "thinkAmount" specifies approximately how many new positions
  // to evaluate.
  void think(qPlayer player2move, gint32 thinkAmount = 10);

private:
  qPositionInfoHash posHash; // Where we store everything we've thought about

   // Where we store what we're thinking about
  qMoveStack   moveStack;
  qComputationTree computationTree;
  qComputationTree::qComputationTreeNodeId currentTreeNode;
  guint8       wallMovesSinceTableUpdate;

  // Internal search routine used by both search() and background searches
  qMove iSearch(qPlayer player2move,     // Which player to find a move for
		guint8  max_complexity,  // keep thinking until below
		guint8  min_depth,       // keep thinking until beyond
		guint8  min_breadth,     // brute force search this many plies
		guint8  slop,            // don't need to refine beyond this
		gint32  max_time,        // Hard limit on our avail. time
		gint32  suggested_time); // Start relaxing criteria after this


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
  const qPositionEvaluation *scanDeeper(qPosition     *pos,
					qPlayer        player2move,
					gint32         depth,
					// qPositionEvaluation *evalToBeat,
					// ??? (carry both alpha & beta?)
					guint32       &r_positionsEvaluated);

};


/*****************************
 * Stuff defined in eval.cpp *
 *****************************/
// It is not an error that an actual pos is put on the stack.
// ratePositionByComputation() wants a copy on the stack that it can
// safely alter during its work.
qPositionEvaluation const *ratePositionByComputation
(qPosition pos, qPlayer player2move, qPositionInfo *posInfo);

// This is a simple virtual iterator useful for passing around eval iterators
// corresponding to container classes that hold various types.
class qEvalIterator {
 public:
  qEvalIterator()  {;};
  ~qEvalIterator() {;};

  // Advances to next eval in list
  virtual void next() = 0;

  // Returns an eval, or NULL if at end.
  virtual qPositionEvaluation const *val() = 0;

  // Returns true if at end; false otherwise
  virtual bool atEnd() = 0;
};


// Here's a template implementation for standard containers of qMoves
// C must be a container type holding qMoves.
template <class C> class qEvalItorFromMvContainer:public qEvalIterator
{
 private:
  C *container;
  C::const_iterator itor;
  C::const_iterator end;

  // Once we have the move, stuff for looking up the eval:
  qPositionInfoHash* posHash;
  qPosition          pos;
  qPlayer            player;

 public:
  // Constructor:  note that whoseEval is who's eval we get--opposite of
  // whose eval we might be computing.
  qEvalItorFromMvContainer(C*                 parentContainer,
			   qPosition*         parentPos,
			   qPlayer            whoseEval,
			   qPositionInfoHash* evalHash) {
    container = parentContainer;
    itor      = container.begin();
    end       = container.end();
    posHash   = evalHash;
    pos       = *parentPos;
    player    = whoseEval;
  };

  ~qEvalItorFromMvContainer() {;};

  void next() {
    ++itor;
  };

  qPositionEvaluation const *val() const {
    if (itor == end)
      return NULL;
    {
      qPositionInfo  info;
      qPosition newPos = pos;
      newPos.applyMove(*itor);
      info = evalHash.getElt(&newPos);
      return (qPositionEvaluation const*)info.get(player);
    }
  };
  bool atEnd() const {
    return (itor == end ? TRUE : FALSE);
  };
};


qPositionInfo *ratePositionFromNeighbors
(qPosition     *pos,
 qPlayer        player2move,
 qPositionInfo *posInfo,      // optional optimization
 qEvalIterator *evalItor = NULL);

#endif // INCLUDE_searcher_h
