 ************************************************************
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE_DOCS file for terms.
 ************************************************************


This source code implements a quoridor-playing machine.

There are several primitive types from qtype.h:
  qSquare - a square (where a pawn can land) on the quoridor board
  qDirection - a vector representing the relative movement between squares
  qMove   - a move (qDirection for pawn more or location of a dropped wall)
In addition, qtype.cpp defines a few constants that are available for use
  by other packages

The larger data structures and interfaces are as follows:
  qPosition - qposition.[h,cpp]
  * An encoded position intended to use as few bytes as possible

  qPositionInfo - posinfo.[h,cpp]
  * Holds all cached info known about a position.
  * positionHashes point to these

  qPositionHash - poshash.[h,cpp]
  * Structure for looking up/storing positions we've seen & associated data.
    This holds evaluations of positions for reference when searching for
    not just the current move but for future moves as well.

  qComputationTree - qcomptree.[h,cpp]
  * Holds state, used in computing qPositionInfos, which is not kept longer
    than a single move search.  A qComputationTree is populated and used
    while searching for a move, then discarded and rebuilt when searching
    for the next move.

  qMoveStack - movstack.[h,cpp]
  * structure for storing sequences of moves under evaluation
  * For each frame, allows push, pop & peek of (playerId, qMove, qPosition)
  * In addition to storing pushed & popped info, this class also provides
    O(1) access to the list of possible (unblocked by other walls) wall
    placement moves from the current frame.
  * Possible pawn moves are not provided, since they can be calculated
    inexpensively from the qPosition.
  * Can provide fast query ability to determine if given positions occur in
    the move stack.  Flagging moves that are part of the
    game history or part of the current move sequence under evaluation can
    prevent repeated moves or endless cycles during evaluation.
  qMoveStack functionality could probably be folded into the qComputationTree,
  but I'd already designed the qMoveStack by the time I came up with the
  qComputationTree.

  eval.cpp 
  * contains a procedure for rating positions from evaluating the board
    position and a procedure for rating positions from their neighbors'
    evaluations.
  * Note that the eval procedures return both a score and an estimated
    complexity of the position.  This allows us to use a refined alpha-beta
    algorithm when searching for best moves.  While the
    difference between alpha & beta are a factor in rating the accuracy
    of a position's score, other factors can also contribute, and are
    stored in an evaluation field called complexity.  (The lower the
    complexity, the more accurate the score is deemed to be.)
  * The evaluation of board positioning is based using the Dijkstra
    algorithm and comparing the number of moves required for the two
    players to reach their goal, plus or minus adjustments for the number
    of walls each player has, etc.

  search.cpp (maybe put this in eval.cpp???)
  * Uses a variation on alpha-beta search to find the best move in a
    given position.  Takes various criteria into consideration for deciding
    when to stop thinking:  time taken, depth searched, how well the
    contendors for "best line" are understood.

Perhaps search.cpp & eval.cpp should be combined into a moveSearcher object.



DATA TYPES LISTED BY SOURCE FILE, AND THEIR DEPENDENCIES

parameters.h:
	square (the mathematical operation)

qtypes.h:
	g*int*
	qDirection
	RowOrCol	
	qSquare	
	qPlayer	
	qMove

qposition.h:
	qPosition: (gint, qSquare, qDirection, qPlayer, qMove) = qtypes.h

qposinfo.h:
	qPositionEvaluation: (gint) = qtypes.h
	qPositionFlag: (gint) = qtypes.h
	qPositionInfo:
		(qPlayer, gint) = qtypes.h
		(qPositionEvaluation) = self

qposhash.h:
	qGrowHash: (gint) = qtypes.h
	qPositionInfoHash:
		(qPosition) = qposition.h
		(qPositionInfo) = qposinfo.h

qmovstack.h:
	qWallMoveInfo (qMove, qWallMoveInfoList) = qtypes.h
	qWallMoveInfoList (qWallMoveInfo) = self
	qMoveList
	qMoveStackFrame:
		(qPosition) = qposition.h
		(qMove, qPlayer) = qtypes.h
		(qWallMoveInfoList) = self
		(qPositionInfo) = qposinfo.h
	qMoveStack:
		(qPlayer, qMove) = qtypes.h
		(qPosition) = qposition.h
		(qPositionInfo) = qposinfo.h
		(qMoveList) = self
		(qMoveStackFrame, qWallMoveInfo, qWallMoveInfoList) = self
	flag_WhiteToMove, flag_BlackToMove
	pushMove(): (qPositionInfoHash) = qposhash.h

getmoves.h
	func getPlayableMoves:
		(qMoveList, qMoveStack) = qmovstack.h
		(qPosition) = qposition.h
	func getPossiblePawnMoves:
		(qMoveList) = qmovstack.h
		(qPosition) = qposition.h
		(qPlayer) = qtypes.h
	func pruneUselessMoves:
		(qMoveList) = qmovstack.h
		(qPosition) = qposition.h

qcomptree.h
	qComputationTree:
		(qMove, gint) = qtypes.h
		(qPositionEvaluation, qPositionInfo) = qposinfo.h
		

qdijkstra.h
	qDijkstraArg:
		(qPosition) = qposition.h
		(qPlayer, gint) = qtypes.h

qsearcher.h
	qSearcher:
		(qPosition) = qposition.h
		(qPlayer, qMove, gint) = qtypes.h
		(qPositionInfoHash) = qposhash.h
		(qMoveStack) = qmovstack.h
		(qComputationTree) = qcomptree.h
		(qPositionEvaluation) = qposinfo.h
	func ratePositionByComputation:
		(qPositionEvaluation) = qposinfo.h
		(qPosition) = qposition.h
		(qPlayer) = qtypes.h
		(qPositionInfo) = qposinfo.h
	qEvalIterator:
		(qPositionEvaluation) = qposinfo.h
	qEvalItorFromMvContainer:qEvalIterator:
		(qPositionInfoHash) = qposhash.h
		(qPlayer) = qtypes.h
		(qPosition) = qposition.h
		(qPositionEvaluation) = qposinfo.h
	func ratePositionFromNeighbors:
		(qPositionInfo) = qposinfo.h
		(qPosition) = qposition.h
		(qPlayer) = qtypes.h
		(qEvalIterator) = self
		
		
TODO:
1. Improve coalesceScores() in eval.cpp
2. Turn on USE_FINISH_SPREAD_SCORE in parameters.h  It should be used.
3. Try turning on HAVE_NUM_COMPUTATIONS
4. Evaluate uses of deque and see if any can/should be replaced with list.
   deque has contant random-access (like vector).  A list is a doubly-linked
   list, which is what I really neede most places that I used deque.

OPTIMIZATIONS:
5. scanDeeper (depth <0 and maybe >0 also) walks nodes, eventually calling
   scanDeeper(depth==0) for leaf nodes.  It disregards the rvals of these
   leaf nodes, and then calls reatePositionFromNeighbors.  Perhaps it could
   get the rating directly from the rvals of previous calls.

