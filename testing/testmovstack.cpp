#include "qtypes.h"
#include "qposhash.h"
#include "qmovstack.h"
#include <stdio.h>

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

void dumpMvList(qMoveList *mvList)
{
	int i = 1;
	qMove mv;
	while (!mvList->empty()) {
		mv = mvList->front();
		printf("%d\t:", i);
		if (mv.isPawnMove())
			printf("Move pawn %d\n", ((int)mv.pawnMoveDirection()));
		else
			printf("Drop wall (%s %d, pos %d)\n",
				mv.wallMoveIsRow()?"ROW":"COL",
				mv.wallRowOrColNo(),
				mv.wallPosition());
		puts("\n");
		mvList->pop_front();
		i++;
	}
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
	movStack->pushEval(NULL, NULL, posHash, whoseMove, moveUp, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING DOWN\n");
	movStack->pushEval(NULL, NULL, posHash, whoseMove, moveDown, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING RIGHT\n");
	movStack->pushEval(NULL, NULL, posHash, whoseMove, moveRight, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING LEFT\n");
	movStack->pushEval(NULL, NULL, posHash, whoseMove, moveLeft, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER DROPPING A WALL\n");
	movStack->pushEval(NULL, NULL, posHash, whoseMove, qMove(ROW, 0, 3), NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING UP\n");
	movStack->pushEval(NULL, NULL, posHash, whoseMove, moveUp, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING UP\n");
	movStack->pushEval(NULL, NULL, posHash, whoseMove, moveUp, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING DOWN\n");
	movStack->pushEval(NULL, NULL, posHash, whoseMove, moveDown, NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nAFTER MOVING DOWN\n");
	movStack->pushEval(NULL, NULL, posHash, whoseMove, moveDown, NULL);
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
	movStack->pushEval(NULL, NULL, posHash, whoseMove, qMove(ROW, 3, 5), NULL);
        whoseMove.changePlayer();
	dumpSituation(movStack);

	printf("\nPOSSIBLE WALL MOVES REPORTED:\n");
        qMoveList mvList;
        movStack->getPossibleWallMoves(&mvList);
	dumpMvList(&mvList);

	return 0;
}
