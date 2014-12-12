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
		movStack->getPlayer2Move().isWhite() ? "white" : "black");
	printf("Current position:\n");
	dumpPos(movStack->getPos());
}

void printMove(qMove mv)
{
  if (mv.isWallMove()) {
    	printf("Move: wall drop at (%s=%d, pos=%d)\n",
               (mv.wallMoveIsRow() ? "ROW" : "COL"),
	       mv.wallRowOrColNo(),
	       mv.wallPosition());
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
			30,	// keep thinking until below
			1,	// keep thinking until beyond
			2,	// brute force search this many plies
			5,	// don't need to refine beyond this
			3600*24*1000,// Hard limit on our avail. time
			3600*16*1000);// Start relaxing criteria after this

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

qMove yo() {
  return qMove((guint8)3);
}

int main
(int argc, char **argv)
{

  qMove foo((guint8)5);
  //qMove foo(5);
  qMove bar((guint8)1);

  bar = yo();


  return 0;
}
