/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: parameters.h,v 1.6 2006/07/27 05:59:27 bmiller Exp $


#ifndef INCLUDE_parameters_h
#define INCLUDE_parameters_h 1

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

#define COMPTREE_INITIAL_SIZE 32768
#define COMPTREE_GROW_SIZE    4096

#define BASE_COMPLEXITY   36 /* Before applying any modifiers */

// This macro can define an array for boosting a player's position score
// based on the number of walls he and his opponent have.
// If not defined, scores are boosted one move per wall
// ??? Based on vapor--needs empirical justification & tuning
#define WALL_SCORE_FUDGE \
{{   0,  72, 139, 205, 270, 334, 397, 459, 520, 580, 630 }, /*0*/\
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

// ??? Based on vapor--needs empirical justification & tuning
// Note that when only one player has walls, the complexity is
// equal to the score boost--this is because the walls can't
// hurt the player but could likely help
#define WALL_COMPLEXITY_FUDGE \
{{   0,  72, 139, 205, 270, 334, 397, 459, 520, 580, 630 }, /*0*/\
 {  72,  64,  79, 145, 210, 274, 337, 398, 459, 519, 578 }, /*1*/\
 { 139,  79,  80,  78, 143, 207, 270, 332, 393, 453, 512 }, /*2*/\
 { 205, 145,  78,  88,  78, 142, 205, 267, 328, 388, 447 }, /*3*/\
 { 270, 210, 143,  78,  92,  78, 141, 203, 264, 324, 383 }, /*4*/\
 { 334, 274, 207, 142,  78,  94,  78, 140, 201, 261, 320 }, /*5*/\
 { 397, 337, 270, 205, 141,  78,  95,  78, 139, 199, 258 }, /*6*/\
 { 459, 398, 332, 267, 203, 140,  78,  96,  78, 138, 197 }, /*7*/\
 { 520, 459, 393, 328, 264, 201, 139,  78,  96,  78, 137 }, /*8*/\
 { 580, 519, 453, 388, 324, 261, 199, 138,  78,  97,  78 }, /*9*/\
 { 630, 578, 512, 447, 383, 320, 258, 197, 137,  78,  98 }}/*10*/

/* When a qsearcher has N contending moves in a given position,
 * it picks which one to work on, and devotes an amount of effort
 * proportional to (effort available)/N.  But if N is large and
 * we are near the tail end of a thinking pass, that amount of
 * effort may be very small.  MIN_POSITIONS_EXAMINED_PER_PLY
 * guarantees some number of new positions to examine, no matter
 * how small (effort avail.)/N is.
 * Regardless of the value of MIN_POSITIONS_EXAMINED_PER_PLY,
 * every possible move will be examined for any ply visited.
 */
#define MIN_POSITIONS_EXAMINED_PER_PLY 75

/* This should equate to the maximum number of seconds conceivably required
 * to perform an exhaustive evaluation of all positions 1 ply away.
 * If there are more than this number of seconds available for the
 * searcher class to work, then calls to searcher.search() should probably
 * specify a min_breadth of 1.
 * If there are fewer than this number of seconds, then the searcher
 * mostly uses evaluations it has already performed to pick a move.
 */
#define MAX_TIME_FOR_1_PLY_SEARCH 4  /* ??? Needs empirical evidence of */ 
#define MAX_TIME_FOR_2_PLY_SEARCH 16 /* run-time computation at startup */
#define MAX_TIME_FOR_3_PLY_SEARCH ???
#define MAX_TIME_FOR_4_PLY_SEARCH ???
// Going to higher numbers of front-loaded N-ply breadth first searches is
// of dubious value because many searched positions would be useless.

// Used in qsearcher.cpp
#define MAXTIME_PER_THINK_SERVICE 4000U
#define SUGTIME_PER_THINK_SERVICE 3000U

/* Define the following if we support tracking the # of position
 * evaluations used to comprise the current position eval.
 */
// #define HAVE_NUM_COMPUTATIONS

template<typename T> inline T square(const T& x) { return x*x; }


/* Define the following if we want to attempt leveraging the "spread" in
 * required moves to reach varios finish squares in evaluating pos scores.
 */
// #define USE_FINISH_SPREAD_SCORE 1

#endif // INCLUDE_parameters_h
