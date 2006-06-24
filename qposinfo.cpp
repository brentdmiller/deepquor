/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qposinfo.h"

IDSTR("$Id: qposinfo.cpp,v 1.3 2006/06/24 00:24:05 bmiller Exp $");


// Used, for example, in a line of thinking that repeats
const qPositionEvaluation qPositionInfo::even_evaluation =
  {
    0,
    0, // What should the complexity be?
  };
/* I've decided to have even_evaluation's complexity be 0 because, suppose
 * every move led to a repeated position--then we'd be stuck with zero
 * complexity.  Surely, the score of a position in this case should be
 * decided by the non-repeating moves.
 */

