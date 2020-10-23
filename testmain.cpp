#include "qtypes.h"
#include "qposhash.h"
#include "qmovstack.h"

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
	qPositionInfoHash *posHash = new qPositionInfoHash;
	qMoveStack *movStack = new qMoveStack;
	qPlayer   whoseMove(qPlayer::WhitePlayer);

	printf("INITIAL POSITION\n");
	dumpSituation(movStack);

	printf("\nAFTER MOVING UP\n");
	movStack->pushEval(NULL, posHash, whoseMove, moveUp, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING DOWN\n");
	movStack->pushEval(NULL, posHash, whoseMove, moveDown, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING RIGHT\n");
	movStack->pushEval(NULL, posHash, whoseMove, moveRight, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING LEFT\n");
	movStack->pushEval(NULL, posHash, whoseMove, moveLeft, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER DROPPING A WALL\n");
	movStack->pushEval(NULL, posHash, whoseMove, qMove(ROW, 0, 3), NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING UP\n");
	movStack->pushEval(NULL, posHash, whoseMove, moveUp, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING UP\n");
	movStack->pushEval(NULL, posHash, whoseMove, moveUp, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING DOWN\n");
	movStack->pushEval(NULL, posHash, whoseMove, moveDown, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING DOWN\n");
	movStack->pushEval(NULL, posHash, whoseMove, moveDown, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER POPPING A MOVE FROM THE STACK\n");
	movStack->popEval();
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER POPPING A MOVE FROM THE STACK\n");
	movStack->popEval();
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER DROPPING A WALL\n");
	movStack->pushEval(NULL, posHash, whoseMove, qMove(ROW, 0, 3), NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	return 0;
}
