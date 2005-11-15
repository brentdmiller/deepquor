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
#include "posinfo.h"
#include "poshash.h"
#include "movstack.h"
#include "parameters.h"

IDSTR("$Id: qsearcher.h,v 1.1 2005/11/15 18:49:42 bmiller Exp $");

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
  qMove search(qPlayer player2move,     // Which player to find a move for
	       guint8  max_complexity,  // keep thinking until below
	       guint8  mindepth,        // keep thinking until beyond
	       guint8  
               gint32  maxtime,         // Hard limit on our avail. time
	       gint32  suggested_time); // Go with any decent, understood moves

  // Adjust qSearcher's stored position with this move
  applyMove(qMove mv, qPlayer p);

  // Turn background thinking on (TRUE) or off (FALSE)
  void think(qPlayer player2move,
	     guint8  max_complexity,
	     guint8  mindepth);

  void setBackgroundThinking(bool thinking);

  // Tell me if the engine is set to think in the background
  bool getBackgroundThinking();

private:
  typedef qPositionHash<qPosition, qPositionInfo> qPosInfoHash;

  qPosInfoHash posHash;     // Where we store everything we've thought about
  qMoveStack   moveStack;   // Where we store what we're thinking about

  bool      bgThinking;     // Is background thinking on or off?
  qPosition bgPos;          // The position bg search should be pondering
  qPlayer   bgPlayerToMove; // Whose move it is in the bgpos
  gmutex    bgMutex;        // Control access to bgdata elements
  gthreadid bgThread;       // handle to bgThread

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

  qPositionEval scanDeeper(qPosition pos,
			   qPlayer player2move,
			   gint8 depth);
  void bgStop();
  void bgGo();


  /* Routines for computing & setting this position's score/depth */
  // Defined in eval.cpp:
  gint16 ratePositionByComputation(qPlayer player2move);


};

#endif // INCLUDE_searcher_h
