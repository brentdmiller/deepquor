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

int main
(int argc, char **argv)
{
  qPosition myPos(NULL, NULL,                              // Walls
		  qSquare(1,7), initialBlackPawnLocation,  // Pawns
		  2, 2);                                   // Walls left
  qPlayer   whoseMove(qPlayer::WhitePlayer);
  qMove mv;

  qSearcher searchObj(&myPos, whoseMove);

  // We really don't need this movstack; we could just apply moves
  // to our copy of "myPos"
  qMoveStack *movStack = new qMoveStack(&myPos, whoseMove);

  printf("INITIAL POSITION\n");
  dumpSituation(movStack);

  mv = searchObj.search(
			whoseMove,	// Which player to find a move for
			30,	// keep thinking until below
			1,	// keep thinking until beyond
			2,	// brute force search this many plies
			5,	// don't need to refine beyond this
			10*1000,	// Hard limit on our avail. time
			5*1000); // Start relaxing criteria after this

  printf("\nAFTER MAKING RECOMMENDED MOVE FOR WHITE:\n");
  movStack->pushMove(whoseMove, mv);
  dumpSituation(movStack);
  searchObj.applyMove(mv, whoseMove);
  whoseMove.changePlayer();

  delete movStack;
  return 0;
}
