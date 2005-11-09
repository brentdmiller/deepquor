 ************************************************************
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE_DOCS file for terms.
 ************************************************************


This source code implements a quoridor-playing machine.

There are several primitive types from qtype.h:
  qSquare - a square (where a pawn can land) on the quoridor board
  qDirection - a vector representing the relative movement between squares
  qMove   - a move (qDirection for pawn more or location of a dropped wall)

The larger data structures and inerfaces are as follows:
  qPosition - qposition.[h,cpp]
  * An encoded position intended to use as few bytes as possible

  qPositionInfo - posinfo.[h,cpp]
  * Holds all cached info known about a position.
  * positionHashes point to these

  qPositionHash - poshash.[h,cpp]
  * Structure for looking up/storing positions we've seen, & associated data

  qMoveStack - movstack.[h,cpp]
  * structure for storing sequences of moves under evaluation
  * For each frame, allows push, pop & peek of (playerId, qMove, qPosition)
  * In addition to storing pushed & popped info, this class also provides
    O(1) access to the list of possible (unblocked by other walls) wall
    placements moves from the current frame.
  * Possible pawn moves are not provides, since this can be calculated
    inexpensives from the qPosition.
  * Can provide fast query ability (if enabled) to determine if given
    positions occur in the move stack.  Flagging moves that are part of the
    game history or part of the current move sequence under evaluation can
    prevent repeated moves or endless cycles during evaluation.

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
