/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qdijkstra.h,v 1.2 2006/07/09 06:37:38 bmiller Exp $

#ifndef INCLUDE_qdijkstra_h
#define INCLUDE_qdijkstra_h 1

#include "qtypes.h"
#include "qposition.h"

typedef struct _qDijkstra {
  const qPosition *pos; // in: Position to examine
  qPlayer player;       // in: which player's pawn to check
  bool getAllRoutes;    // in: TRUE  == find every accessible finish
                        //     FALSE == stop after finding fastest route 

  gint8 dist[10];  // out: number of moves required to reach finish square(s)
                   // dist[0] is # moves to reach fastest possible finish
                   // dist[1] is # moves to reach next closest finish, etc.
                   // dist[N] == -1 indicates no more accessible finishes.

  /* Maybe allow the following
   * bool          useCachedGraph;
   * qCalcGraphRec graph;
   * #define qDarg_CLEAR_CACHE(d) ((d).useCachedGraph=FALSE) (1)
   */
#define qDarg_CLEAR_CACHE(d) 1
} qDijkstraArg;

/* Returns: Number of accessible finish squares found
 * In theory this takes O(E*log V) time, but because we have an array of
 * vertices and must check for exactly 4 possible edges of each one plus
 * check whether the vertex has been seen for every edge that exists, our
 * time will be more like O(mV + nE)
 */ 
int qDijkstra (qDijkstraArg *arg);

#endif // INCLUDE_qpdijkstra_h
