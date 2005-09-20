typedef struct _qPositionEvaluation {
  gint16 score;         // Rating of how good position is
  guint8 complexity;    // Score's uncertainty: 0=sure, 100=0 plys, 255=unknown
  /*guint32 computations;*/ // Number of direct calculations that contributed
} qPositionEvaluation;

typedef gint8 qPositionFlag;
#define flag_WhiteToMove 0x01
#define flag_BlackToMove 0x02
#define flag_PosIllegal  0xf0 // This should be a big negative number

/************************************************************************
 * class qPositionHashElt                                               *
 * It's important not to have any constructor/destructor for this class *
 * because we allocate large arrays of these elts, and do not want to   *
 * have to perform initialization on all the elts.                      *
 ************************************************************************/
class qPositionHashElt {
 public:
  qPosition pos;

  // Use isPosExceptional 1st for fast comparison; then use other tests
  inline bool isPosExceptional { return(flagPosException); };
  inline bool isPosLegal() { return (flagPosException>=0); };
  inline bool isInMoveStack() { return (flagPosException>0)};
  inline bool isWhiteMoveInStack() { return (flagPosException>0)};
  inline bool isBlackMoveInStack() { return (flagPosException)};
  inline void setPositionIsIllegal() { flagPosException = flag_PosIllegal; };
  inline void setMoveStack(gint8  playerToMove) {
    if (playerToMove == WHITE)
      flagPosException |= flag_WhiteToMove;
    else
      flagPosException |= flag_BlackToMove;
  }
  inline void clearMoveStack(gint playerToMove) {
    if (playerToMove == WHITE)
      flagPosException &= ~flag_WhiteToMove;
    else
      flagPosException &= ~flag_BlackToMove;
  }
  inline gint16 getScore(qPlayer p) {return evaluation[p].score;};
  inline gint16 setScore(qPlayer p, gint16 val) {evaluation[p].score=val;};
  inline guint8 getComplexity(qPlayer p) {return evaluation[p].complexity;};
  inline guint8 setComplexity(qPlayer p, guint8 val)
    {evaluation[p].complexity=val;};
  inline guint8 getComputations(qPlayer p)
    {return evaluation[p].computations;};
  inline guint8 setComputations(qPlayer p, guint32 val)
    {evaluation[p].computations=val;};

  /* Routines for computing & setting this position's score/depth */
  // Defined in eval.cpp:
  gint16 ratePositionByComputation(qPlayer player2move);
  
 private:
  /* Stats for each player if his turn.   [0]=white, [1]=black */
  qPositionEvaluation evaluation[2];
  qPositionFlag flagPosException;
}

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
class qPositionHash {
 public:
  qPositionHash();
  ~qPositionHash();

  qPositionHashElt *getElt(qPosition *pos);
  bool              addElt(qPosition *pos);
  bool              rmElt(qPosition *pos);

 private:
  guint32 numElts;
  list<qPositionHashElt*> *hashBuffer;  // Array of qPositionHashElt buckets
  qPositionHashEltHeap    posHeap;     // We get unallocated Elts from here
}

/***************************************************************************
 * class qPositionHashEltHeap
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
}
