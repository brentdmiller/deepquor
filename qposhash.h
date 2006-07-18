/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */

// $Id: qposhash.h,v 1.6 2006/07/18 18:58:09 bmiller Exp $

#ifndef INCLUDE_poshash_h
#define INCLUDE_poshash_h 1

#include "qtypes.h"
#include "qposinfo.h"
#include "qposition.h" /* Required for qPositionInfoHash at end */
#include <vector>
#include <list>
#include "parameters.h" /* Needed for inlined def of eltAlloc */
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
 *
 * If additional free memory is needed, we can then throw out the 19-wall
 * heap, followed by the 18-wall, ....   The idea is that even if we throw
 * away a high-numbered heap, the scores of it's positions will already
 * have been copied to "lower" positions and most of them are probably
 * no longer needed.  Another strategy that might be worth trying is to
 * throw away heaps at odd intervals, keeping every other or every Nth
 * heap.  The idea is that it's easy to regenerate a position's score from
 * the neighboring positions.  By throwing out every other heap, we will
 * avoid needing to regenerate many layers of neighbors to recalculate the
 * score for a discarded position.
 */

/***************************************************************************
 * class qGrowHash                                                         *
 * Because the intent is to use a different hash table for each number     *
 * of played walls, and blow away entire tables whenever a wall is played, *
 * our hash structure is made to be efficient at getting new one-elt       *
 * allocations (for storing new positions) and destroying entire hashes,   *
 * but we don't worry too much about speed or efficient memory use when    *
 * freeing single elts.  We don't worry about decreasing a hash's          *
 * memory footprint until the entire hash is destroyed.                    *
 *                                                                         *
 * Thus, this hash type is good at growing new value elements one-at-a     *
 * time as needed, and it can be destroyed in O(1) time; but it is not     *
 * memory-efficient if individual elements are often removed.              *
 **************************************************************************/

template <class keyType, class valType> class qGrowHash {
public:
  static const guint16   NumBuckets; // Useful for writing hash funcs
  typedef guint16 (*qGrowHash_hashFunc)(const keyType*);
  typedef void    (*qGrowHash_eltInitFunc)(valType*, const keyType*);

  // constructor using specified hashFunc
  // Note that the value used for hashing will actually be
  // the hashFunc's return value modulo POSITION_HASH_BUCKETS
  // Default hashFunc does "pretty good" hashing based on sizeof(keyType)
  qGrowHash
    (qGrowHash_eltInitFunc initCallbackFunc=NULL,
     qGrowHash_hashFunc hashCallbackFunc=(&qGrowHash::defaultqGrowHashFunc));

  /* Make constructor take paramaters as args???
   *  int32_t           numHashBuckets,  // approx. mem/sizeof(key + val)
   *  int32_t           initialHeapSize, // small to save mem until needed?
   *  int32_t           heapGrowSize;    // 1024 something like 1024 elts
   */

  ~qGrowHash();

  // Locate an existing position
  valType* getElt(const keyType *pos) const;

  // Acquires a new elt
  valType* addElt(const keyType *pos);

  valType* getOrAddElt(const keyType *pos)
  { valType *rval = getElt(pos); return rval ? rval : addElt(pos); };

  // free elt so getElt won't find it
  bool     rmElt(const keyType *pos);

 private:
  /************************************************************************
   * private subclass qGrowHashElt                                        *
   * Large arrays of these elts are allocated at a time, so clients       *
   * that require scalable performance should use datum types that        *
   * do not require initialization (i.e. types with constructors and/or   *
   * destructors).  Otherwise we'll have to perform initialization/       *
   * destruction  for all the inidividual elts.                           *
   ************************************************************************/
  class qGrowHashElt {
    // Note that I've deliberately avoided constructors or destructors;
    // instances of this object should only be created in large arrays that
    // are initialized by eltAlloc()
  public:
    keyType         pos;
    valType         posInfo;
  };  // HACK ALERT!!! See below:

  // I want to use the qGrowHashElt type as elements in a list<qGrowHashElt*>;
  // however, C++ won't allow me to use a template as the type in a
  // template parameter (unless the parameter is explicitly created to take
  // a template template parameter).
  typedef list<void*> qGrowHashEltList;
  inline void*         hackifyGrowHashEltType (qGrowHashElt *x) const
    { return static_cast<void*>(x); }
  inline qGrowHashElt* unhackGrowHashEltType  (void *x) const
    { return static_cast<qGrowHashElt*>(x); }

  /***************************************************************************
   * private subclass qGrowHashEltHeap                                       *
   * Now we have a heap from which we draw new positions.  We seldom if ever
   * free individual positions.  Thus, we'll use one free list for all
   * freed elts regardless of from which allocation block they came.  Once
   * the current block runs out of elts, then we'll start drawing from the
   * free list until there are no more.
   * If the current block and free list both run out of space, we just
   * another "currBlock" from which to draw new elts.  Blocks are kept in
   * a blocks2free list so we can remember to free them all later.
   **************************************************************************/
  class qGrowHashEltHeap {
  public:
    qGrowHashEltHeap();
    ~qGrowHashEltHeap();

    // ??? I couldn't figure out how to define this in a .cpp file with the
    // rval type being of local scope, so I've inlined it.  See commented-out
    // code in qposhash.cpp
    qGrowHashElt *eltAlloc() // returns uninitialized memory
      {
	if (currBlockAvailElts > 0) {
	  return &currBlock[--currBlockAvailElts];
	} else if (!freeEltList.empty()) {
	  qGrowHashElt *rval = freeEltList.back();
	  freeEltList.pop_back();
	  return rval;
	} else {
          //currBlock = new qGrowHashElt[HEAP_BLOCK_SIZE];
          currBlock = (qGrowHashElt*)calloc(HEAP_BLOCK_SIZE, sizeof(qGrowHashElt));
	  if (currBlock) {
	    blocks2free.push_front(currBlock);
	    currBlockAvailElts = HEAP_BLOCK_SIZE - 1; // subt. 1 cuz we're rtrning 1
	  } else
	    currBlockAvailElts = 0;
	  return &currBlock[currBlockAvailElts];
	}
      }

    void eltFree(qGrowHashElt*);

  private:
    qGrowHashElt            *currBlock;     // array of Elts to draw from
    guint32                  currBlockAvailElts;
    vector<qGrowHashElt*>    freeEltList;   // freed Elts that can be reused
    /* Optimization!!!  If we make a singly-linked list of freeElts, using
     * the Elt's storage to hold the *next pointer, this will be faster
     * because we'll never have to allocate any memory
     * Just set *elt=freeEltList; freeEltList=elt to push;
     * elt=freeEltlist; freeEltlist = *elt; return elt to pop.
     */
    list<qGrowHashElt*>      blocks2free;   // Pointer to arrays of Elts
  };

  guint32 numElts;
  qGrowHashEltList     *hashBuffer; // Array of qGrowHashElt buckets
  qGrowHashEltHeap      posHeap;    // We get unallocated Elts from here
  qGrowHash_hashFunc    hashCbFunc; // func for sorting keys into buckets
  qGrowHash_eltInitFunc initCbFunc; // func for initializing new elts

  static guint16 defaultqGrowHashFunc(const keyType *);
};



// For convenience, provide a type name for the AI's most common usage
typedef qGrowHash<qPosition, qPositionInfo> qPositionInfoHash;

#endif // INCLUDE_poshash_h
