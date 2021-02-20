#include "qtypes.h"
#include "qposhash.h"
#include "qmovstack.h"
#include "qsearcher.h"
#include <stdio.h>

void dumpSituation(qPlayer whoseMove, const qPosition *pos)
{
	printf("Player to move: %s\n", whoseMove.getPlayerName());
	printf("Current position:\n");
	pos->dump();
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

int main
(int argc, char **argv)
{
  qPosition startPos(NULL, NULL, // Walls
		     qSquare(4), qSquare(76), // Pawns
		     10, 10); // Walls remaining
  qPosition *myPos = &startPos;

  // We really don't need this movstack; we could just apply moves
  // to our copy of "myPos"
  qPlayer whoseMove = qPlayer::WhitePlayer;

  qSearcher searchObj(myPos, whoseMove);

  // We really don't need this movstack; we could just apply moves
  // to our copy of "myPos"
  qMoveStack *movStack = new qMoveStack(myPos, whoseMove);

  qMove mv;

  whoseMove.changePlayer(); // Hack to simplify the main loop
  while (1) {
    whoseMove.changePlayer();

    printf("%s to move\n", whoseMove.getPlayerName());
    dumpSituation(searchObj.getPlayerToMove(), searchObj.getPos());

    // ??? This is where we would prompt for user commands
    // but for now we'll just let the computer play itself
    // to test the flow.

    mv = searchObj.search(
			  whoseMove,	// Which player to find a move for
			  30,	// keep thinking until below
			  1,	// keep thinking until beyond
			  2,	// brute force search this many plies
			  5,	// don't need to refine beyond this
			  /*3600**/7*1000,// Hard limit on our avail. time
			  /*3600**/3*1000);// Start relaxing criteria after this

    printf("\n%s plays:\n", whoseMove.getPlayerName());
    printMove(mv);
    printf("\n\n");

    searchObj.applyMove(mv, whoseMove);

  } while (!movStack->getPos()->isWon(whoseMove));

  printf("%s wins\n\n", whoseMove.getPlayerName());
    
  delete movStack;
  return 0;
}
