void applyMove
(qPlayer player,
 qMove   move)
{
  if (move.isWallMove)
    {
      if (move.wallMoveIsRow())
	row_walls[move.wallRowOrColNo()] |= 1<<(move.wallPosition);
      else
	col_walls[move.wallRowOrColNo()] |= 1<<(move.wallPosition);
      return;
    }
  else // isPawnMove
    {
      if (player==WHITE)
	setWhitePawn(getWhitePawn(player) + move.pawnMoveDirection());
      else
	setBlackPawn(getBlackPawn(player) + move.pawnMoveDirection());
      return;
    }
}

guint16 qPosition::hashFunc
(void)
{
  guint32 mixer
    (mixer = pos->white_pawn_pos) |= (pos->black_pawn_pos<<9);

#define mixedRowNCol(n) ((((guint32)row_walls)[(3*(n))%8]<<(2*n)) ^ ((guint32)col_walls)[(5*(n)+2)%8]<<(2*n+7))
  mixer ^= mixedRowNCol(0) ^
    mixedRowNCol(1) ^
    mixedRowNCol(2) ^
    mixedRowNCol(3) ^
    mixedRowNCol(4) ^
    mixedRowNCol(5) ^
    mixedRowNCol(6) ^
    mixedRowNCol(7);
  mixer ^= (guint32)numwalls;

  // Now superimpose the top 16 bytes on the lower 16 bytes, so the lower
  // 16 bytes are well mixed.
  mixer ^= mixer>>16;
  return (guint16)(mixer%POSITION_HASH_BUCKETS);
}
