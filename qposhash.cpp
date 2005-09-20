#include <list>
using namespace std;

/*********************** 
 * class qPositionHash *
 ***********************/
qPositionHash::qPositionHash
()
{
  hashBuffer = new list<qPositionHashElt>[POSITION_HASH_BUCKETS];
  numElts = 0;
}

qPositionHash::~qPositionHash
()
{  delete [] hashBuffer; }

qPositionHashElt *qPositionHash::getElt
(qPosition *pos)
{
  guint16 hashBucket = pos.hashFunc();

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

bool qPositionHash::addElt
(qPosition *pos)
{
  qPositionHashElt *newElt = posHeap.eltAlloc();
  if (!newElt)
    return False;

  // clear newElt->evaluation[2] & newElt->flagPosException
  newElt->clearEval(???);
  newElt->pos = *pos;

  hashBuffer[pos.hashFunc()].push_front(newElt);
  numElts++;

  return True;
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
HEAP_BLOCK_SIZE
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
