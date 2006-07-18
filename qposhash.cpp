/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */



#include "qposhash.h"
#include "parameters.h"

IDSTR("$Id: qposhash.cpp,v 1.7 2006/07/18 18:58:09 bmiller Exp $");


/****/

/*********************** 
 * class qGrowHash     *
 ***********************/

template <class keyType, class valType>
const guint16 qGrowHash<keyType, valType>::NumBuckets = POSITION_HASH_BUCKETS;

// default hash func
template <class keyType, class valType>
guint16 qGrowHash<keyType, valType>::defaultqGrowHashFunc
(const keyType *key)
{
  /* Adapted this from http://www.azillionmonkeys.com/qed/hash.html */
  // This code is for 32-bit, and I've simplified it by just lopping off
  // 2 bytes before returning!!!

  // #include "pstdint.h" << original header, available at above URL
#include <stdint.h>
#ifndef UINT32_C  /* No idea why this isn't getting defined by stdint.h -bdm */
#define UINT32_C(c)  ((uint32_t)(c))
#endif

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__)	\
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((const uint8_t *)(d))[1] << UINT32_C(8))	\
                      +((const uint8_t *)(d))[0])
#endif

  const char * data = (const char*)(&key);
  int len = sizeof(key);
  uint32_t hash = len, tmp;
  int rem;

  if (len <= 0 || data == NULL) return 0;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }

  /* Handle end cases */
  switch (rem) {
  case 3: hash += get16bits (data);
    hash ^= hash << 16;
    hash ^= data[sizeof (uint16_t)] << 18;
    hash += hash >> 11;
    break;
  case 2: hash += get16bits (data);
    hash ^= hash << 11;
    hash += hash >> 17;
    break;
  case 1: hash += *data;
    hash ^= hash << 10;
    hash += hash >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 2;
  hash += hash >> 15;
  hash ^= hash << 10;

  return hash % POSITION_HASH_BUCKETS;  // Use a cast to throw away extra bits???
}

template <class keyType, class valType>
qGrowHash<keyType, valType>::qGrowHash
(qGrowHash_eltInitFunc i,
 qGrowHash_hashFunc h)
{
  hashBuffer = new qGrowHashEltList[POSITION_HASH_BUCKETS];
  numElts = 0;
  hashCbFunc = h ? h : &qGrowHash::defaultqGrowHashFunc;
  initCbFunc = i;
}

template <class keyType, class valType>
qGrowHash<keyType, valType>::~qGrowHash
()
{  delete [] hashBuffer; }

template <class keyType, class valType>
valType *qGrowHash<keyType, valType>::getElt
(const keyType *pos) const
{
  guint16 hashBucket = hashCbFunc(pos);

  // Find the elt in the bucket
  qGrowHashEltList::const_iterator iter;
  for (iter = hashBuffer[hashBucket].begin();
       iter != hashBuffer[hashBucket].end();
       iter++) {
    if ((unhackGrowHashEltType(*iter)->pos) == *pos)
      return &(unhackGrowHashEltType(*iter)->posInfo);
  }
  return NULL;
}
 
template <class keyType, class valType>
valType *qGrowHash<keyType, valType>::addElt
(const keyType *pos)
{
  qGrowHashElt *newElt = posHeap.eltAlloc();
  if (!newElt)
    return FALSE;

  // clear newElt->evaluation[2] & newElt->flagPosException
  newElt->pos = *pos;
  if (initCbFunc)
    initCbFunc(&newElt->posInfo, &newElt->pos);

  hashBuffer[this->hashCbFunc(pos)].push_front(hackifyGrowHashEltType(newElt));
  numElts++;

  return &(newElt->posInfo);
}

template <class keyType, class valType>
bool qGrowHash<keyType, valType>::rmElt
(const keyType *pos)
{
  guint16 hashBucket = this->hashCbFunc(pos);

  // Find the elt in the bucket
  qGrowHashEltList::iterator iter;
  for (iter = hashBuffer[hashBucket].begin();
       iter != hashBuffer[hashBucket].end();
       iter++) {
    if (unhackGrowHashEltType(*iter)->pos == *pos) {
      (void)hashBuffer[hashBucket].erase(iter);
      posHeap.eltFree(unhackGrowHashEltType(*iter));
      numElts--;
      return TRUE;
    }
  }
  return FALSE;
}


/******************************
 * class qGrowHashEltHeap     *
 ******************************/
template <class keyType, class valType>
qGrowHash<keyType, valType>::qGrowHashEltHeap::qGrowHashEltHeap
()
{
  /* Avoid constructors & destructors on individual elts */
  // currBlock = new qGrowHashElt[HEAP_INITIAL_BLOCK_SIZE];
  currBlock = (qGrowHashElt*)calloc(HEAP_INITIAL_BLOCK_SIZE, sizeof(qGrowHashElt));
  if (currBlock) {
    blocks2free.push_front(currBlock);
    currBlockAvailElts = HEAP_INITIAL_BLOCK_SIZE;
  } else
    currBlockAvailElts = 0;
}

template <class keyType, class valType>
qGrowHash<keyType, valType>::qGrowHashEltHeap::~qGrowHashEltHeap
()
{
  while (!blocks2free.empty()) {
    //delete [] (blocks2free.front());
    free(blocks2free.front());
    blocks2free.pop_front();
  }
}

#if 0
template <class keyType, class valType>
qGrowHash<keyType, valType>::qGrowHashElt qGrowHash<keyType, valType>::qGrowHashEltHeap::eltAlloc
()
{
  if (currBlockAvailElts > 0) {
    return &currBlock[--currBlockAvailElts];
  } else if (!freeEltList.empty()) {
    qGrowHashElt *rval = freeEltList.back();
    freeEltList.pop_back();
    return rval;
  } else {
    // currBlock = new qGrowHashElt[HEAP_BLOCK_SIZE]; Avoid constructors
    currBlock = (qGrowHashElt*)calloc(HEAP_BLOCK_SIZE, sizeof(qGrowHashElt));
    if (currBlock) {
      blocks2free.push_front(currBlock);
      currBlockAvailElts = HEAP_BLOCK_SIZE - 1; // subt. 1 cuz we're rtrning 1
    } else
      currBlockAvailElts = 0;
    return &currBlock[currBlockAvailElts];
  }
}
#endif

template <class keyType, class valType>
void qGrowHash<keyType, valType>::qGrowHashEltHeap::eltFree
(qGrowHashElt* pos)
{
  freeEltList.push_back(pos);
}

// Compile qGrowHash object for (qPosition,qPositionInfo) types
template class qGrowHash<qPosition, qPositionInfo>;
