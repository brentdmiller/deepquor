#define BLOCKED_POSITION_FUDGE 40
#define TURN_SCORE   64 /* Choosing multiples of 2 optimizes arithmetic */
#define PLY_SCORE    32
#define MOVESTACKSIZ 200 /* Must be big enough to hold entire game */
#define POSITION_HASH_BUCKETS 49152 /* Make this variable??? */
#define HEAP_INITIAL_BLOCK_SIZE 32
#define HEAP_BLOCK_SIZE       1024

// This macro can define an array for modifying a position's complexity
// based on the number of opponent's walls
#define WALL_COMPLEXITY_FUDGE \
{ -45, -24, -4, -1, 0, 1, 1, 2, 2, 2, 2 }

// This macro can define an array for boosting a player's position score
// based on the number of walls he and his opponent have.
// If not defined, scores are boosted one move per wall
#define WALL_SCORE_FUDGE \
{{   0,  72, 139, 205, 270, 334, 397, 459, 520, 580, 639 }, /*0*/\
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


/***************************************
 * Above here is tweakable parameters; *
 * below here is code                  *
 ***************************************/
typedef gint8 qDirection;
#define  LEFT  -1;
#define  RIGHT  1;
#define  DOWN  -9;
#define  UP     9;

typedef guint8 qSquare;
#define SQUARE(x,y) ((x)+(9*(y)))
#define SQUARE_X(sq) ((sq)%9)
#define SQUARE_Y(sq) ((sq)/9)

#define COL 0
#define ROW 1

/* We'd really like to turn off alignment so positions can be packed into
 * a tight array!!!
 */
class qPosition {
 public:
  inline guint8 numWhiteWallsLeft() { return (numwalls & 0x15); };
  inline guint8 numBlackWallsLeft() { return (numwalls >> 4); };
  inline guint8 numWallsLeft(qPlayer p)
    { return (p)==WHITE ? numWhiteWallsLeft() : numBlackWallsLeft(); };
  inline gbool  isBlockedByWall(qSquare fromSq, qDirection dir)
    { return isBlockedByWall(SQUARE_X(fromSq), SQUARE_Y(fromSq), dir); };
  inline gbool  isBlockedByWall(guint8 x, guint8 y, qDirection dir)
    {
      switch(dir) {
      case UP:
	return( (y>=8) ? FALSE : (row_walls[y] & (x?((1<<x)|(1<<(x-1))):1) ));
      case DOWN:
	return( (y==0) ? FALSE : (row_walls[y] & (x?((1<<x)|(1<<(x-1))):1) ));
      case LEFT:
	return( (x==0) ? FALSE : (col_walls[x] & (y?((1<<y)|(1<<(y-1))):1) ));
      case RIGHT:
	return( (x>=8) ? FALSE : (col_walls[x] & (y?((1<<y)|(1<<(y-1))):1) ));
      }
      g_assert(0);
      return FALSE;
    };
  inline qSquare getPawn(qPlayer p)
    { return (p==WHITE)?white_pawn_pos:black_pawn_pos; };
  inline qSquare getWhitePawn() { return white_pawn_pos; };
  inline qSquare getBlackPawn() { return black_pawn_pos; };
  inline void setPawn(qPlayer p, qSquare s)
    { if (p==WHITE) white_pawn_pos=s; else black_pawn_pos=s; };
  inline void setWhitePawn(qSquare s) { white_pawn_pos=s; };
  inline void setBlackPawn(qSquare s) { black_pawn_pos=s; };

  // This is slow, but we use to initialize a fast cached lookup of whether
  // any wall move is possible and also a linked list of all possible wall
  // moves in the qMoveStack class.
  // Use fast lookups of cached data from there instead.
  bool canPutWall(bool, guint8, guint8);

  void applyMove(qPlayer, qMove);
  guint16 hashFunc(qPosition *);

#if 0 /* Don't need any of these yet */
  inline void applyPawnMove(qPlayer, qDirection);
  inline void putWall(bool, guint8, guint8);
  inline void putWall(qMove);
bool qPosition::wallAt(bool HorV, guint8 row, guint8 x)
{ return (HorV == ROW) ? };???
bool qPosition::wallAt(qMove)
{};???
#endif

 private:
  guint8 row_walls[8];
  guint8 col_walls[8];
  guint8 white_pawn_pos;
  guint8 black_pawn_pos;
  guint8 numwalls; /* low 4 bits white, high 4 bits black */
}


typedef gint8 qPlayer;
#define WHITE 0
#define BLACK 1

#define qScore_won   0x7fff
#define qScore_lost -0x7fff
#define qScore_PLY   PLY_SCORE
#define qScore_TURN  TURN_SCORE
#define qScore_WALL  TURN_SCORE /* Maybe this should be a function */


class qMove {
 private:
  /* The move is encoded as follows.
   *
   * Setting the low bit indicates a wall drop; unset indicates a pawn move
   *
   * For a pawn move, (signed)move>>1 gives the qDirection of the move
   *
   * For a wall move, there following are indicates the drop position:
   *  (0x02 & mv)==TRUE  - place wall at row
   *  (0x02 & mv)==FALSE - place wall at column
   *  (0x1f & mv)>>2     - indicates row/col in which to lay wall
   *  (mv>>5) - indicates at which position within row/col to lay wall
   */
  guint8 move;

 public:
  // Constructor for a wall placement
  qMove(bool RorC, guint8 rowColNo, guint8 posNo) // Last 2 args must be < 8
    { move = (posNo<<5)|(rowColNo<<3)|(RorC); };

  // Constructor for a pawn move
  qMove(gint8 x, gint y) { move = (x+9*y)<<1; }; // Both args must be < abs(3)
  qMove(qDirection d)    { move = (d<<1);     };

  // Constructor using a previously encoded move
  // qMove(gint8 mv) { move = mv; };

  // Members for accessing wall moves
  inline bool isWallMove()      { return   move&0x01;      };
  inline bool   wallMoveIsRow() { return   move&0x02;      };
  inline bool   wallMoveIsCol() { return !(move&0x02);     };
  inline guint8 wallRowOrColNo(){ return   (move&0x1f)>>2; };
  inline guint8 wallPosition()  { return   move>>5;;       };

  // Members for accessing pawn moves
  inline bool       isPawnMove()        { return !(move&0x01);             };
  inline qDirection pawnMoveDirection() { return  (((qDirection)move)>>1); };

  // Gets the binary representation of a move (in one byte)
  inline guint8 getEncoding(void) { return move; };
}

/* Notes:
 * For hashed positions, we should generally give preference to keeping
 * positions that required greater computation to derive.
 * Perhaps it is a good idea to count the number of computed positions
 * that contributed.  For example, a directly computed position would score
 * one accumulated computed position.
 * A position that was computed from 82 neighboring positions, 81 of which
 * were directly computed and 1 of which was computed from 30 directly-computed
 * neighbors, would score 111 computations.
 */
