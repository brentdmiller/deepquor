/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#ifndef INCLUDE_poshash_h
#define INCLUDE_poshash_h

#include <vector>
#include <list>
using namespace std;

// #include <glib.h>
#include "qtypes.h"  // REMOVE THIS--it's just here for a glib hack
// Remove qtypes.h when glib.h works

/* The intent is to keep 20 position hashes, each one corresponding to the
 * number of played walls in the positions it contains.
 *
 * Whenever a player places a wall, we can throw away the smallest hash.
 * Since walls cannot be removed, we will not need it any more.
 * If the process is running out of memory, the 2 least expensive operations
 * for freeing memory are (1) throw away the smallest heap (since it mostly
 * contains previously computed positions...the only positions we can
 * actually reach are by pawn moves and they only account for about 4%
 * of the positions); and (2) throw away the 20-wall heap (it is
 * easy to recompute because there are no more walls available).
 *
 * In fact, upon reaching a position with 20 walls placed, we should probably
 * discard the 20-wall heap once to purge all the unreachable positions.
 * This discard could be skipped if there is too little time on the clock
 * and memory to spare.
 */

/***************************************************************************
 * class qPositionHash                                                     *
 * Because the intent is to use a different hash table for each number     *
 * of played walls, and blow away entire tables whenever a wall is played, *
 * our hash structure is made to be efficient at getting new one-elt       *
 * allocations (for storing new positions) and destroying entire hashes,   *
 * but we don't worry too much about speed or efficient memory use when    *
 * freeing single elts.  We don't worry about decreasing a hash's          *
 * memory footprint until the entire hash is destroyed.                    *
 **************************************************************************/

template <class keyType, class valType> class qPositionHash {
public:
  typedef guint16 (*qPositionHashFunc)(keyType*);
  typedef void    (*qPositionInitFunc)(valType*, keyType*);

  // constructor using specified hashFunc
  // Note that the value used for hashing will actually be
  // the hashFunc's return value modulo POSITION_HASH_BUCKETS
  qPositionHash
    (qPositionHashFunc hashCallbackFunc, qPositionInitFunc initCallbackFunc);

  /* Make constructor take paramaters as args???
   *  int32_t           numHashBuckets,  // approx. mem/sizeof(key + val)
   *  int32_t           initialHeapSize, // small to save mem until needed?
   *  int32_t           heapGrowSize;    // 1024 something like 1024 elts
   */

  // constructor using default hashFunc & parameters
  // Default hashFunc does "pretty good" hashing based on sizeof(keyType)
  qPositionHash();

  ~qPositionHash();

  // Locate an existing position
  valType* getElt(const keyType *pos);

  // Acquires a new elt
  valType* addElt(const keyType *pos);

  // free elt so getElt won't find it
  bool           rmElt(const keyType *pos);

 private:
  /************************************************************************
   * private subclass qPositionHashElt                                    *
   * Large arrays of these elts are allocated at a time, so clients       *
   * that require scalable performance should use datum types that        *
   * do not require initialization (i.e. types with constructors and/or   *
   * destructors).  Otherwise we'll have to perform initialization/       *
   * destruction  for all the inidividual elts.                           *
   ************************************************************************/
  typedef struct _qPositionHashElt {
    public:
    keyType         pos;
    valType         posInfo;
  } qPositionHashElt;

  /***************************************************************************
   * private subclass qPositionHashEltHeap                                   *
   * Now we have a heap from which we draw new positions.  We seldom if ever
   * free individual positions.  Thus, we'll use one free list for all
   * freed elts regardless of from which allocation block they came.  Once
   * the current block runs out of elts, then we'll start drawing from the
   * free list until there are no more.
   * If the current block and free list both run out of space, we push the
   * current block onto the blocks2free list and allocate a new block.
   **************************************************************************/
  class qPositionHashEltHeap {
  public:
    qPositionHashEltHeap();
    ~qPositionHashEltHeap();

    qPositionHashElt *eltAlloc();    // returns uninitialized memory
    void eltFree(qPositionHashElt*);

  private:
    qPositionHashElt         *currBlock;     // array of Elts to draw from
    guint32                   currBlockAvailElts;
    vector<qPositionHashElt*> freeEltList;   // freed Elts that can be reused
    /* Optimization!!!  If we make a singly-linked list of freeElts, using
     * the Elt's storage to hold the *next pointer, this will be faster
     * because we'll never have to allocate any memory
     * Just set *elt=freeEltList; freeEltList=elt to push;
     * elt=freeEltlist; freeEltlist = *elt; return elt to pop.
     */
    list<qPositionHashElt*>   blocks2free;   // Pointer to arrays of Elts
  };

  guint32 numElts;
  list<qPositionHashElt*> *hashBuffer; // Array of qPositionHashElt buckets
  qPositionHashEltHeap     posHeap;    // We get unallocated Elts from here
  qPositionHashFunc        hashCbFunc; // func for sorting keys into buckets
  qPositionInitFunc        initCbFunc; // func for initializing new elts

  static guint16 defaultqPositionHashFunc(keyType *);
};

#endif // INCLUDE_poshash_h
