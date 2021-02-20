#include "qtypes.h"
#include "qposhash.h"
#include "qmovstack.h"
#include "qsearcher.h"

void dumpPos(const qPosition *pos)
{
	printf(" White pawn: (%d, %d)\n",
		pos->getWhitePawn().x(), pos->getWhitePawn().y());
	printf(" Black pawn: (%d, %d)\n",
		pos->getBlackPawn().x(), pos->getBlackPawn().y());
	printf(" White walls left: %d\n", pos->numWhiteWallsLeft());
	printf(" Black walls left: %d\n", pos->numBlackWallsLeft());
}

void dumpSituation(qMoveStack *movStack)
{
	printf("Player to move: %s\n",
		movStack->getPlayerToMove().isWhite() ? "white" : "black");
	printf("Current position:\n");
	dumpPos(movStack->getPos());
}

void printMove(qMove mv)
{
  if (mv.isWallMove()) {
        if (mv.wallMoveIsRow()) {
                printf("Move: wall drop at (%s=%d.5, pos=%c-%c)\n",
                       "ROW",
                        mv.wallRowOrColNo(),
                        'A' + mv.wallPosition(),
                        'B' + mv.wallPosition());
        } else {
                printf("Move: wall drop at (%s=%c.5, pos=%d-%d)\n",
                       "COL",
                       'A' + mv.wallRowOrColNo(),
                       mv.wallPosition(),
                       1+mv.wallPosition());
        }       
  } else {
    printf("Move: pawn move ");

    switch (mv.pawnMoveDirection()) {
    case UP: printf("UP"); break;
    case DOWN: printf("DOWN"); break;
    case LEFT: printf("LEFT"); break;
    case RIGHT: printf("RIGHT"); break;

    case UP+UP: printf("2 UP"); break;
    case DOWN+DOWN: printf("2 DOWN"); break;
    case LEFT+LEFT: printf("2 LEFT"); break;
    case RIGHT+RIGHT: printf("2 RIGHT"); break;

    case UP+LEFT: printf("UP+LEFT"); break;
    case UP+RIGHT: printf("UP+RIGHT"); break;
    case DOWN+LEFT: printf("DOWN+LEFT"); break;
    case DOWN+RIGHT: printf("DOWN+RIGHT"); break;

    default:
      printf("%d", mv.pawnMoveDirection());
    }
    printf("\n");
  }
}

qMove doTest
(qPosition *myPos,
 qPlayer    whoseMove)
{
  qMove mv;

  qSearcher searchObj(myPos, whoseMove);

  // We really don't need this movstack; we could just apply moves
  // to our copy of "myPos"
  qMoveStack *movStack = new qMoveStack(myPos, whoseMove);

  printf("INITIAL POSITION\n");
  dumpSituation(movStack);

  mv = searchObj.search(
			whoseMove,	// Which player to find a move for
			20,	// keep thinking until below
			10,	// keep thinking until beyond
			2,	// brute force search this many plies
			3,	// don't need to refine beyond this
			120*1000,// Hard limit on our avail. time
			30*1000);// Start relaxing criteria after this

  printf("\nRETURNED MOVE FOR %s:\n", whoseMove.isWhite() ? "WHITE" : "BLACK");
  printMove(mv);
  movStack->pushMove(whoseMove, mv);
  //dumpSituation(movStack);
  searchObj.applyMove(mv, whoseMove);
  whoseMove.changePlayer();

  delete movStack;
  printf("\n\n");
  return mv;
}

int main
(int argc, char **argv)
{
//0   1   2   3   4   5   6    7
//1   2   4   8  16  32  64  128

  //        Position:   0   1   2   3   4   5   6   7
  guint8 testrows[] ={160, 32,  0,  0, 32,  0,160, 64};
  guint8 testcols[] ={  0,  0,  0,  0,  2,  0,  0,  0};

  qPosition testPos(testrows, testcols, // Walls
		    qSquare(7,6), qSquare(7,2), // Pawns
		    0, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::BlackPlayer)); // Whose turn

  return 0;
}
