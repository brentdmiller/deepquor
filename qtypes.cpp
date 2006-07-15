/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qtypes.h"

IDSTR("$Id: qtypes.cpp,v 1.1 2006/07/09 17:09:38 bmiller Exp $");


/****/

const qMove moveUp    = qMove(UP);
const qMove moveDown  = qMove(DOWN);
const qMove moveLeft  = qMove(LEFT);
const qMove moveRight = qMove(RIGHT);

const qMove moveUpUp       = qMove((qDirection)(UP+UP));
const qMove moveDownDown   = qMove((qDirection)(DOWN+DOWN));
const qMove moveLeftLeft   = qMove((qDirection)(LEFT+LEFT));
const qMove moveRightRight = qMove((qDirection)(RIGHT+RIGHT));

const qMove moveUL = qMove((qDirection)(UP+LEFT));
const qMove moveUR = qMove((qDirection)(UP+RIGHT));
const qMove moveDL = qMove((qDirection)(DOWN+LEFT));
const qMove moveDR = qMove((qDirection)(DOWN+RIGHT));

const qMove moveNull = qMove((guint8)0);