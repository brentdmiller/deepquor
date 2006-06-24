/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */



#include "qposhash.h"
#include "parameters.h"

IDSTR("$Id: qposhash.cpp,v 1.4 2006/06/24 00:24:05 bmiller Exp $");


/*********************** 
 * class qGrowHash     *
 ***********************/

// default hash func
template <class keyType> static guint16
qGrowHash::defaultqGrowHashFunc
(keyType *key)
{
  /* Adapted this from http://www.azillionmonkeys.com/qed/hash.html */
  // This code is for 32-bit, and I've simplified it by just lopping off
  // 2 bytes before returning!!!

#include "pstdint.h" /* Replace with <stdint.h> if appropriate */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((const uint8_t *)(d))[1] << UINT32_C(8))\
                      +((const uint8_t *)(d))[0])
#endif

  const char * data = &(const char*)(key);
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

  return hash;
}


qGrowHash::qGrowHash
()
{
  qGrowHash(NULL, NULL);
}
qGrowHash::qGrowHash
(qGrowHash_hashFunc h,
 qGrowHash_initFunc i)
{
  hashBuffer = new list<qGrowHashElt>[POSITION_HASH_BUCKETS];
  numElts = 0;
  hashCbFunc = h ? h : &qGrowHash::defaultqGrowHashFunc;
  initCbFunc = i;
}

qGrowHash::~qGrowHash
()
{  delete [] hashBuffer; }

template <class keyType, class valType> valType *qGrowHash::getElt
(const keyType *pos)
{
  guint16 hashBucket = hashFunc(pos);

  // Find the elt in the bucket
  list<qGrowHashElt*>::const_iterator iter;
  for (iter = hashBuffer[hashBucket].begin();
       iter != hashBuffer[hashBucket].end;
       iter++) {
    if ((*iter)->pos == *pos)
      return *iter;
  }
  return NULL;
}

template <class keyType, class valType> valType *qGrowHash::addElt
(const keyType *pos)
{
  qGrowHashElt *newElt = posHeap.eltAlloc();
  if (!newElt)
    return False;

  // clear newElt->evaluation[2] & newElt->flagPosException
  newElt->pos = *pos;
  if (initCbFunc)
    initCbFunc(&newElt->posInfo, &newElt->pos);

  hashBuffer[pos.hashFunc()].push_front(newElt);
  numElts++;

  return newElt->posInfo;
}

qGrowHashElt *qGrowHash::rmElt
(qPosition *pos)
{
  guint16 hashBucket = pos.hashFunc();

  // Find the elt in the bucket
  list<qGrowHashElt*>::const_iterator iter;
  for (iter = hashBuffer[hashBucket].begin();
       iter != hashBuffer[hashBucket].end;
       iter++) {
    if ((*iter)->pos == *pos) {
      (void)hashBuffer[hashBucket].erase(iter);
      posHeap.eltFree(*iter);
      numElts--;
      return True;
    }
  }
  return False;
}


/******************************
 * class qGrowHashEltHeap     *
 ******************************/
qGrowHashEltHeap::qGrowHashEltHeap
()
{
  currBlock = new qGrowHashElt[HEAP_INITIAL_BLOCK_SIZE];;
  if (currBlock)
    currBlockAvailElts = HEAP_INITIAL_BLOCK_SIZE;
  else
    currBlockAvailElts = 0;
}

qGrowHashEltHeap::~qGrowHashEltHeap
()
{
  while (!blocks2free->empty()) {
    delete [] (blocks2free->front());
    blocks2free->pop_front();
  }
}

qGrowHashElt *qGrowHashEltHeap::eltAlloc
()
{
  if (currBlockAvailElts > 0) {
    return &currBlock[--currBlockAvailElts];
  } else if (!freeEltList.empty()) {
    qGrowHashElt *rval = freeEltList.back();
    freeEltList.pop_back();
    return rval;
  } else {
    blocks2free.push_front(currBlock);
    currBlock = new qGrowHashElt[HEAP_BLOCK_SIZE];
    if (currBlock)
      currBlockAvailElts = HEAP_BLOCK_SIZE - 1; // subt. 1 cuz we're rtrning 1
    else
      currBlockAvailElts = 0;
    return &currBlock[currBlockAvailElts];
  }
}

void qGrowHashEltHeap::eltFree
(qGrowHashElt* pos)
{
  freeEltList.push_back(pos);
}
