/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qposinfo.h"

IDSTR("$Id: qposinfo.cpp,v 1.5 2006/07/18 06:55:33 bmiller Exp $");


/****/

const qPositionEvaluation positionEval_won_rec =
  { qScore_won, 0, };
const qPositionEvaluation *positionEval_won = &positionEval_won_rec;

const qPositionEvaluation positionEval_lost_rec =
  { qScore_lost, 0, };
const qPositionEvaluation *positionEval_lost = &positionEval_lost_rec;

// Used, for example, in a line of thinking that repeats
const qPositionEvaluation positionEval_even_rec =
  { 0, 0 /* What should the complexity be? */, };
const qPositionEvaluation *positionEval_even = &positionEval_even_rec;

/* I've decided to have even_evaluation's complexity be 0 because, suppose
 * every move led to a repeated position--then we'd be stuck with zero
 * complexity.  Surely, the score of a position in this case should be
 * decided by the non-repeating moves.
 */

const qPositionEvaluation positionEval_none_rec =
  { 0, qComplexity_max, };
const qPositionEvaluation *positionEval_none = &positionEval_none_rec;


/*************************
 * qPositionInfo members * 
 *************************/
bool qPositionInfo::evalExists(qPlayer p) const
{
  return ((this->getComplexity(p) != qComplexity_max) ||
          (this->getScore(p)      != 0));
}

bool qPositionInfo::initEval(qPlayer p)
{
  this->evaluation[p.getPlayerId()] = positionEval_none_rec;
  return TRUE;
}

bool qPositionInfo::initEval()
{
  this->evaluation[0] = this->evaluation[1] = positionEval_none_rec;
  return TRUE;
}

