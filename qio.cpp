/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qtypes.h"

IDSTR("$Id: qio.cpp,v 1.1 2014/12/12 21:20:21 bmiller Exp $");


/****/
const qPlayer qPlayer_white(qPlayer::WhitePlayer);
const qPlayer qPlayer_black(qPlayer::BlackPlayer);

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
