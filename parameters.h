/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */



#ifndef INCLUDE_parameters_h
#define INCLUDE_parameters_h

/* Many adjustable parameters are defined here rather than in their
 * component code, for convenience in tweaking parameters.
 */

/* To clean the OO design and prevent massive recompilation whenever
 * a parameter changes, this file should be rewritten as a bunch of
 * read-only global externs that get passed (or used, if sloppy OO
 * implementation is ok) as default values to each component!!!
 */


#define BLOCKED_POSITION_FUDGE 40
#define TURN_SCORE   64 /* Choosing multiples of 2 optimizes arithmetic */
#define PLY_SCORE    32
#define MOVESTACKSIZ 200 /* Must be big enough to hold entire game */
#define POSITION_HASH_BUCKETS 49152 /* Make this variable??? */
#define HEAP_INITIAL_BLOCK_SIZE 32
#define HEAP_BLOCK_SIZE       1024

// This macro is useful for defining an array of modifiers to a
// position's complexity, based on the number of opponent's walls remaining
#define WALL_COMPLEXITY_FUDGE \
{ -45, -24, -4, -1, 0, 1, 1, 2, 2, 2, 2 }

// This macro can define an array for boosting a player's position score
// based on the number of walls he and his opponent have.
// If not defined, scores are boosted one move per wall
#define WALL_SCORE_FUDGE \
{{   0,  72, 139, 205, 270, 334, 397, 459, 520, 580, 639 }, /*0*/\
 { -72,   0,  69, 135, 200, 264, 327, 388, 449, 509, 568 }, /*1*/\
 {-139, -69,   0,  67, 132, 196, 259, 321, 382, 442, 501 }, /*2*/\
 {-205,-135, -67,   0,  66, 130, 193, 255, 316, 376, 435 }, /*3*/\
 {-270,-200,-132, -66,   0,  65, 128, 190, 251, 311, 370 }, /*4*/\
 {-334,-264,-196,-130, -65,   0,  64, 126, 187, 247, 306 }, /*5*/\
 {-397,-327,-259,-193,-128, -64,   0,  63, 124, 184, 243 }, /*6*/\
 {-459,-388,-321,-255,-190,-126, -63,   0,  62, 122, 181 }, /*7*/\
 {-520,-449,-382,-316,-251,-187,-124, -62,   0,  61, 120 }, /*8*/\
 {-580,-509,-442,-376,-311,-247,-184,-122, -61,   0,  60 }, /*9*/\
 {-630,-568,-501,-435,-370,-306,-243,-181,-120, -60,   0 }}/*10*/


#endif // INCLUDE_parameters_h
