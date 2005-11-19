/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qposinfo.h,v 1.3 2005/11/19 08:22:33 bmiller Exp $

#ifndef INCLUDE_posinfo_h
#define INCLUDE_posinfo_h

#include "qtypes.h"

/* qPositionEvaluation
 * Struct containing all relevant information stored regarding the score
 * of a position for a given player.
 * ??? Items Marked "!" are possible future enhancement ideas
 */

typedef struct _qPositionEvaluation {
  gint16 score;         // Rating of how good position is
  guint16 complexity;   // Score's uncertainty: 0=sure, +/- range of score
  //! guint32 computations; // Number of direct calculations that contributed
  //! guint8  demand;       // Rating of how significant this position is
} qPositionEvaluation;

// Maximum complexity
#define qComplexity_max  (guint16)0xffff

#define qScore_won   (gint16)0x7fff
#define qScore_lost  (gint16)-0x7fff

// Useful, for example, in a line of thinking that repeats
extern const qPositionEvaluation *positionEval_even;
extern const qPositionEvaluation *positionEval_won;
extern const qPositionEvaluation *positionEval_lost;

// A position that has not been evaluated beyond checked for game over
extern const qPositionEvaluation *positionEval_none;



/* Note that in general we want to evalatute positions with increasing
 * depth until we've found an adequate move with sufficiently low
 * complexity.  If each position's score is viewed as a "quantum cloud"
 * centered at the score, but with a possible range described by the
 * complexity, we would like to keep evaluating until we've found a best
 * move with no overlapping "quantum clouds" from other moves.
 * This is achieved by adding depth to whichever relevant positions have
 * the largest quantum cloud (i.e. highest complexity--by adding more
 * depth, we can decrease the complexity).
 *
 * However, maybe "demand" should be used to help prioritize the order in
 * which we evaluation positions.  For example, if a given position is
 * relevant to many lines of thinking, then it's probably worth evaluating
 * that position well.
 * Similarly, if we have a choice between deeper analysis of several
 * positions, perhaps adding analysis to the one with the highest score
 * should be prefered slightly, since that analysis is more likely to be
 * useful on future moves.
 */


/************************************************************************
 * class qPositionInfo                                                  *
 * This will be the datum in our qPositionHash                          *
 * It's important not to use a type with constructors/destructors       *
 * because we allocate large arrays of these elts, and do not want to   *
 * have to perform initialization on all the elts.                      *
 ************************************************************************/
/* qPositionInfo class
 * This class holds everything we will remember about any pariticular
 * position.
 */
class qPositionInfo {
 public:
  bool          evalExists(qPlayer p);  // Return if a score/cmplxty are set
  bool          initEval(qPlayer p);    // Initialize eval to !exists for p
  bool          initEval();             // Initialize to !exists for both

  inline qPositionEvaluation const *get(qPlayer p);

  void set(qPlayer p, qPositionEvaluation const *neweval);

  inline gint16 const getScore(qPlayer p)
    { return evaluation[p.getPlayerId()].score; };

  inline void         setScore(qPlayer p, gint16 val)
    { evaluation[p.getPlayerId()].score=val; };

  inline guint8 const getComplexity(qPlayer p)
    { return evaluation[p.getPlayerId()].complexity;};

  inline void         setComplexity(qPlayer p, guint8 val)
    { evaluation[p.getPlayerId()].complexity=val;};

  /* inline guint8 getComputations(qPlayer p)
     { return evaluation[p.getPlayerId()].computations;};
     inline void   setComputations(qPlayer p, guint32 val)
     { evaluation[p.getPlayerId()].computations=val;};
  */

  /* Flags for noting stuff about position.
   * Values <= are reserved as follows:
   * 0  - nothing of interest
   * <0 - position is not legal
   * Positive bitmask values can be set and cleared by subordinate packages
   * Bits are reserved by the following packages:
   * bits 1-2: movstack
   */
  typedef gint8 qPositionFlag;
  enum {
    flag_PosIllegal  = 0xf0, // This should be a big negative number

    // Bits reserved by movstack pkg:
    flag_WhiteToMove = 0x01, // Position w/white to move under evaluation
    flag_BlackToMove = 0x02, // Position w/black to move under evaluation

    flag_unused4     = 0x04,
    flag_unavail     = 0x08  // Sign bit, signifies illegal position
  };

  // isPosExceptional - return if position is illegal or has flags set
  // Use isPosExceptional 1st for fast comparison; then use other tests
  inline bool isPosExceptional()     { return (flagPosException != 0); };

  inline bool isPosLegal()           { return (flagPosException >= 0); };
  inline void setPositionIsIllegal() { flagPosException = flag_PosIllegal; };

  gint8 getPositionFlag()            { return(flagPosException); }
  gint8 setPositionFlagBits(guint8 bitmask)
    { assert(flagPosException >= 0);
      return (flagPosException |= bitmask);
    }
  gint8 clearPositionFlagBits(guint8 bitmask)
    { assert(flagPosException >= 0);
      return (flagPosException &= ~bitmask);
    }

 private:
  /* Stats for each player if his turn.   [0]=white, [1]=black */
  qPositionEvaluation evaluation[2];

  qPositionFlag flagPosException;
};

#endif // INCLUDE_posinfo_h
