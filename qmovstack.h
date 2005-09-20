
/* Idea:
 * We'll construct a large array enumerating every possible wall location.
 * Then we'll use prev/next pointers in the elts of that array to link it
 * into a large doubly-linked list.
 * During move evaluation, we'll have a stack of moves, and each stack
 * frame will contain the position at that frame, a move, and glist of
 * wall moves eliminated by that move.
 * When we add a new move to the stack, we look up the wall positions
 * eliminated by that move, remove them from the possible wall move list
 * and insert them into that frame's "eliminated wall move list."
 * When we work our way back up the move stack and undo that move, we
 * insert the frame's eliminated wall move list" back into the possible wall
 * move list.
 * Note that as the game progresses, we can boost performance slightly by
 * regenerating the list of each wall move's "blocked" moves.  This can be
 * done by re-initializing the move stack from the current position.
 */

extern qPosition qInitialPosition;

typedef struct _wallMoveInfo {
  qMove move;            // Which wall move is this? (0x0-0x7f)
  bool possible;         // Is this move currently possible?
  qWallMoveInfoList eliminates; // Which moves get eliminated by this move?
  struct _wallMove *prev;
  struct _wallMove *next;
} qWallMoveInfo;

class qWallMoveInfoList {
 public:
  qWallMoveInfoList(); // Constructor
  void           push(qWallMoveInfo*);
  void           pop(qWallMoveInfo*);
  qWallMoveInfo *pop();
  void           getHead(void)  { return head; };
  void           getTail(void)  { return tail; };
  void           clearList(void){ head = tail = NULL; };

 private:
  qWallMoveInfo *head;
  qWallMoveInfo *tail;
}

/* Keep a cache of possible moves and associated scores??? */
typedef struct _qMoveStackFrame {
  qPosition         pos;
  qMove             prevMove;
  qWallMoveInfoList wallMovesBlockedByMove;
} qMoveStackFrame;

class qMoveStack {
 public:
  qMoveStack();           // Initialize from the normal starting open board
  qMoveStack(*qPosition); // Initialize from an existing position
  void      ~qMoveStack();

  /* This func generates each wall move's list of possible wall moves
   * that it blocks.  It can optionally be called after any move (real move,
   * not evaluated move) to generate updated (and hopefully shorter) lists.
   * A small performance boost might come from doing this during the opponent's
   * thinking time.  However, note that any time a move is "taken back," the
   * move table cannot be used if it is newer than the move being taken back.
   * If the move table is newer, then it will have to be regenerated from an
   * older position before stepping back.
   */ 
  void initWallMoveTable(qPosition*);

  void  pushMove(qPlayer, qMove, qPosition*); // ending pos is optional
  void  popMove(void);
  qMove peekLastMove(void)   {return moveStack[sp].prevMove;};
  qPosition* getPos(void)    {return &(moveStack[sp].pos);};
  qPosition* getPrevPos(void){return &(moveStack[sp-1].pos);};

 private:
  qMoveStackFrame moveStack[MOVESTACKSIZ];
  guint8          sp;

  qWallMoveInfo     allWallMoveArry[128];
  qWallMoveInfoList possibleWallMoves;
}
