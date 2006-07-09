/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qposinfo.h"

IDSTR("$Id: qposinfo.cpp,v 1.4 2006/07/09 06:37:38 bmiller Exp $");


// Used, for example, in a line of thinking that repeats
const qPositionEvaluation positionEval_even_rec =
  {
    0,
    0, // What should the complexity be?
  };
const qPositionEvaluation *positionEval_even = &positionEval_even_rec;
/* I've decided to have even_evaluation's complexity be 0 because, suppose
 * every move led to a repeated position--then we'd be stuck with zero
 * complexity.  Surely, the score of a position in this case should be
 * decided by the non-repeating moves.
 */

