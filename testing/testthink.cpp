#include "qtypes.h"
#include "qposhash.h"
#include "qmovstack.h"
#include "qsearcher.h"
#include <stdio.h>

void dumpSituation(qMoveStack *movStack)
{
	printf("Player to move: %s\n",
		movStack->getPlayer2Move().isWhite() ? "white" : "black");
	printf("Current position:\n");
	movStack->getPos()->dump();
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
			/*3600**/24*1000,// Hard limit on our avail. time
			/*3600**/16*1000);// Start relaxing criteria after this

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
  printf("case 1 (expect UP)\n");
  qPosition testPos(NULL, NULL, // Walls
		    qSquare(1,7), qSquare(4,8), // Pawns
		    0, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 1A (expect UP)\n");
  testPos = qPosition (NULL, NULL, // Walls
		    qSquare(1,7), qSquare(4,1), // Pawns
		    0, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 2 (expect UP)\n");
  testPos = qPosition (NULL, NULL, // Walls
		    qSquare(1,7), qSquare(4,8), // Pawns
		    2, 2); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 3 (expect wall at ROW 0, pos 3 or 4)\n");
  testPos = qPosition (NULL, NULL, // Walls
		    qSquare(1,5), qSquare(4,1), // Pawns
		    5, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  guint8 testrows[] = {0,0,0,0,0,0,0,2};

  printf("case 4 (expect LEFT or RIGHT)\n");
  testPos = qPosition (testrows, NULL, // Walls
		    qSquare(1,7), qSquare(4,8), // Pawns
		    0, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 5 (expect LEFT)\n");
  testPos = qPosition (testrows, NULL, // Walls
		    qSquare(1,7), qSquare(4,2), // Pawns
		    0, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 6 (expect RIGHT)\n");
  guint8 testcols[] = {64,0,0,0,0,0,0,0};
  testPos = qPosition (testrows, testcols, // Walls
		    qSquare(1,7), qSquare(4,8), // Pawns
		    0, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 7a (expect DOWN probably--lost pos)\n");
  guint8 testrows7a[] = {0,0,0,0,0,0,0,2};
  guint8 testcols7a[] = {64,0,64,0,0,0,1,0};

  testPos = qPosition(testrows7a, testcols7a, // Walls
		    qSquare(54), qSquare(80), // Pawns
		    0, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::BlackPlayer)); // Whose turn

  guint8 testcols2[] = {64,0,64,0,0,0,0,0};
  printf("case 7b (expect DOWN)\n");
  testPos = qPosition (testrows, testcols2, // Walls
		    qSquare(1,7), qSquare(4,8), // Pawns
		    0, 0); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 8 (forced loss)\n");
  testPos = qPosition (NULL, NULL, // Walls
		    qSquare(1,4), qSquare(4,1), // Pawns
		    0, 1); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 9==case 3 + walls (expect wall at ROW 0, pos 3 or 4)\n");
  testPos = qPosition (NULL, NULL, // Walls
		    qSquare(1,5), qSquare(4,1), // Pawns
		    5, 1); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 10 (expect wall at ROW 5, pos 1)\n");
  testPos = qPosition (NULL, testcols2, // Walls
		    qSquare(1,6), qSquare(4,4), // Pawns
		    1, 1); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 11 (expect UP+UP)\n");
  testPos = qPosition (NULL, NULL, // Walls
		    qSquare(31), qSquare(40), // Pawns
		    1, 1); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  printf("case 12 (expect ?)\n");
  testPos = qPosition (NULL, NULL, // Walls
		    qSquare(4), qSquare(76), // Pawns
		    10, 10); // Walls remaining
  doTest(&testPos,
	 qPlayer(qPlayer::WhitePlayer)); // Whose turn

  return 0;
}
