/*
 * Copyright (c) 2005
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */



#include "poshash.h"
#include "parameters.h"

IDSTR("$Id: qposhash.cpp,v 1.2 2005/11/09 20:27:25 bmiller Exp $");


/*********************** 
 * class qPositionHash *
 ***********************/

// default hash func
template <class keyType> static guint16
qPositionHash::defaultqPositionHashFunc
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


qPositionHash::qPositionHash
()
{
  qPositionHash(NULL, NULL);
}
qPositionHash::qPositionHash
(qPositionHashFunc h,
 qPositionInitFunc i)
{
  hashBuffer = new list<qPositionHashElt>[POSITION_HASH_BUCKETS];
  numElts = 0;
  hashCbFunc = h ? h : &qPositionHash::defaultqPositionHashFunc;
  initCbFunc = i;
}

qPositionHash::~qPositionHash
()
{  delete [] hashBuffer; }

template <class keyType, class valType> valType *qPositionHash::getElt
(const keyType *pos)
{
  guint16 hashBucket = hashFunc(pos);

  // Find the elt in the bucket
  list<qPositionHashElt*>::const_iterator iter;
  for (iter = hashBuffer[hashBucket].begin();
       iter != hashBuffer[hashBucket].end;
       iter++) {
    if ((*iter)->pos == *pos)
      return *iter;
  }
  return NULL;
}

template <class keyType, class valType> valType *qPositionHash::addElt
(const keyType *pos)
{
  qPositionHashElt *newElt = posHeap.eltAlloc();
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

qPositionHashElt *qPositionHash::rmElt
(qPosition *pos)
{
  guint16 hashBucket = pos.hashFunc();

  // Find the elt in the bucket
  list<qPositionHashElt*>::const_iterator iter;
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
 * class qPositionHashEltHeap *
 ******************************/
qPositionHashEltHeap::qPositionHashEltHeap
()
{
  currBlock = new qPositionHashElt[HEAP_INITIAL_BLOCK_SIZE];;
  if (currBlock)
    currBlockAvailElts = HEAP_INITIAL_BLOCK_SIZE;
  else
    currBlockAvailElts = 0;
}

qPositionHashEltHeap::~qPositionHashEltHeap
()
{
  while (!blocks2free->empty()) {
    delete [] (blocks2free->front());
    blocks2free->pop_front();
  }
}

qPositionHashElt *qPositionHashEltHeap::eltAlloc
()
{
  if (currBlockAvailElts > 0) {
    return &currBlock[--currBlockAvailElts];
  } else if (!freeEltList.empty()) {
    qPositionHashElt *rval = freeEltList.back();
    freeEltList.pop_back();
    return rval;
  } else {
    blocks2free.push_front(currBlock);
    currBlock = new qPositionHashElt[HEAP_BLOCK_SIZE];
    if (currBlock)
      currBlockAvailElts = HEAP_BLOCK_SIZE - 1; // subt. 1 cuz we're rtrning 1
    else
      currBlockAvailElts = 0;
    return &currBlock[currBlockAvailElts];
  }
}

void qPositionHashEltHeap::eltFree
(qPositionHashElt* pos)
{
  freeEltList.push_back(pos);
}
