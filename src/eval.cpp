#include "eval.hpp"
#include "hash.hpp"
#include "magic.hpp"
#include "ttable.hpp"

int drawScore = 0;

array<int, 64> pieceSquareTableOpening[12];
array<int, 64> pieceSquareTableEnding[12];

map<uint64_t, int> knownEndgames;

array<int, Squares> queenTropism[Squares];
array<int, Squares> rookTropism[Squares];
array<int, Squares> knightTropism[Squares];
array<int, Squares> bishopTropism[Squares];

void initializeKnownEndgames()
{
	// King vs king: draw
	uint64_t matHash = materialHash[WhiteKing][0] ^ materialHash[BlackKing][0];
	knownEndgames[matHash] = drawScore;

	// King and a minor piece vs king: draw
	for (int i = White; i <= Black; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			knownEndgames[matHash ^ materialHash[j + i * 6][0]] = drawScore;
		}
	}

	// King and two knights vs king: draw
	for (int i = White; i <= Black; i++)
	{
		knownEndgames[matHash ^ materialHash[Knight + i * 6][0] ^ materialHash[Knight + i * 6][1]] = drawScore;
	}

	// King and a minor piece vs king and a minor piece: draw
	for (int i = Knight; i <= Bishop; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			knownEndgames[matHash ^ materialHash[White + i][0] ^ materialHash[Black * 6 + j][0]] = drawScore;
		}
	}

	// King and two bishops vs king and a bishop: draw
	for (int i = White; i <= Black; i++)
	{
		knownEndgames[matHash ^ materialHash[Bishop + i * 6][0] ^ materialHash[Bishop + i * 6][1] ^ materialHash[Bishop + !i * 6][0]] = drawScore;
	}

	// King and either two knights or a knight and a bishop vs king and a minor piece: draw
	for (int i = White; i <= Black; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			for (int k = Knight; k <= Bishop; k++)
			{
				knownEndgames[matHash ^ materialHash[Knight + i * 6][0] ^ materialHash[j + i * 6][j == Knight] ^ materialHash[k + !i * 6][0]] = drawScore;
			}
		}
	}
}

void initializeKingTropism()
{
	int Distance[64][64];
	int DistanceNW[64] = {
		0, 1, 2, 3, 4, 5, 6, 7,
		1, 2, 3, 4, 5, 6, 7, 8,
		2, 3, 4, 5, 6, 7, 8, 9,
		3, 4, 5, 6, 7, 8, 9,10,
		4, 5, 6, 7, 8, 9, 10,11,
		5, 6, 7, 8, 9,10, 11,12,
		6, 7, 8, 9,10,11,12,13,
		7, 8, 9,10,11,12,13,14
	};
	int DistanceNE[64] = {
		 7, 6, 5, 4, 3, 2, 1, 0,
		 8, 7, 6, 5, 4, 3, 2, 1,
		 9, 8, 7, 6, 5, 4, 3, 2,
		10, 9, 8, 7, 6, 5, 4, 3,
		11,10, 9, 8, 7, 6, 5, 4,
		12,11,10, 9, 8, 7, 6, 5,
		13,12,11,10, 9, 8, 7, 6,
		14,13,12,11,10, 9, 8, 7
	};
	int nTropism[15] = { 14, 22, 29, 28, 19, -1, -6, -10, -11, -12, -13, -14, -15, -16, -17 };
	int bTropism[15] = { 6, -12, 4, -16, -11, -17, -6, -14, -9, -17, -1, -17, -4, 3, 7 };
	int rTropism[15] = { 7, 22, 23, 22, 22, 16, -2, -5, -14, -10, -8, -15, -16, -17, -17 };
	int qTropism[15] = { 35, 49, 47, 44, 40, 14, 3, 0, -2, 1, 4, -3, -5, -6, -9 };
	int i, j;

	for (i = A1; i <= H8; i++)
	{
		for (j = A1; j <= H8; j++)
		{
			Distance[i][j] = (abs((i % 8) - (j % 8)) + abs((i / 8) - (j / 8)));
		}
	}
	for (i = A1; i <= H8; i++)
	{
		for (j = A1; j <= H8; j++)
		{
			queenTropism[i][j] = qTropism[Distance[i][j]];
			rookTropism[i][j] = rTropism[Distance[i][j]];
			knightTropism[i][j] = nTropism[Distance[i][j]];
			bishopTropism[i][j] += bTropism[abs(DistanceNE[i] - DistanceNE[j])];
			bishopTropism[i][j] += bTropism[abs(DistanceNW[i] - DistanceNW[j])];
		}
	}
}

void initializeEval()
{
	initializeKnownEndgames();
	initializeKingTropism();

	array<int, Squares> flip = {
		56, 57, 58, 59, 60, 61, 62, 63,
		48, 49, 50, 51, 52, 53, 54, 55,
		40, 41, 42, 43, 44, 45, 46, 47,
		32, 33, 34, 35, 36, 37, 38, 39,
		24, 25, 26, 27, 28, 29, 30, 31,
		16, 17, 18, 19, 20, 21, 22, 23,
		8, 9, 10, 11, 12, 13, 14, 15,
		0, 1, 2, 3, 4, 5, 6, 7
	};

	for (int i = Pawn; i <= King; i++)
	{
		for (int sq = A1; sq <= H8; sq++)
		{
			pieceSquareTableOpening[i][sq] = openingPST[i][sq] + pieceValuesOpening[i];
			pieceSquareTableEnding[i][sq] = endingPST[i][sq] + pieceValuesEnding[i];

			pieceSquareTableOpening[i + Black * 6][sq] = -(openingPST[i][flip[sq]] + pieceValuesOpening[i]);
			pieceSquareTableEnding[i + Black * 6][sq] = -(endingPST[i][flip[sq]] + pieceValuesEnding[i]);
		}
	}
}

int mobilityEval(Position & pos, int phase, int & kingTropismScore)
{
	int scoreOp = 0;
	int scoreEd = 0;
	int from, count;
	uint64_t occupied = pos.getOccupiedSquares();
	uint64_t tempPiece, tempMove;

	// White
	uint64_t targetBitboard = ~pos.getPieces(White);
	int kingLocation = bitScanForward(pos.getBitboard(Black, King));

	tempPiece = pos.getBitboard(White, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = knightAttacks[from] & targetBitboard;

		count = popcnt(tempMove);
		scoreOp += mobilityOpening[Knight][count];
		scoreEd += mobilityEnding[Knight][count];
		kingTropismScore += knightTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(White, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = bishopAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp += mobilityOpening[Bishop][count];
		scoreEd += mobilityEnding[Bishop][count];
		kingTropismScore += bishopTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(White, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = rookAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp += mobilityOpening[Rook][count];
		scoreEd += mobilityEnding[Rook][count];
		kingTropismScore += rookTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(White, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = queenAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp += mobilityOpening[Queen][count];
		scoreEd += mobilityEnding[Queen][count];
		kingTropismScore += queenTropism[from][kingLocation];
	}

	// Black
	kingLocation = bitScanForward(pos.getBitboard(White, King));
	targetBitboard = ~pos.getPieces(Black);

	tempPiece = pos.getBitboard(Black, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = knightAttacks[from] & targetBitboard;

		count = popcnt(tempMove);
		scoreOp -= mobilityOpening[Knight][count];
		scoreEd -= mobilityEnding[Knight][count];
		kingTropismScore -= knightTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(Black, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = bishopAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp -= mobilityOpening[Bishop][count];
		scoreEd -= mobilityEnding[Bishop][count];
		kingTropismScore -= bishopTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(Black, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = rookAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp -= mobilityOpening[Rook][count];
		scoreEd -= mobilityEnding[Rook][count];
		kingTropismScore -= rookTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(Black, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = queenAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp -= mobilityOpening[Queen][count];
		scoreEd -= mobilityEnding[Queen][count];
		kingTropismScore -= queenTropism[from][kingLocation];
	}

	return ((scoreOp * (256 - phase)) + (scoreEd * phase)) / 256;
}

int pawnStructureEval(Position & pos, int phase)
{
	int scoreOp = 0, scoreEd = 0, value, from;
	uint64_t whitePawns, blackPawns, tempPiece;

	if ((value = pttProbe(pos)) != probeFailed)
	{
		return value;
	}

	tempPiece = whitePawns = pos.getBitboard(White, Pawn);
	blackPawns = pos.getBitboard(Black, Pawn);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		if (whitePawns & rays[N][from])
		{
			scoreOp -= doubledPenaltyOpening;
			scoreEd -= doubledPenaltyEnding;
		}

		if (!(blackPawns & passed[White][from]))
		{
			scoreOp += passedBonusOpening;
			scoreEd += passedBonusEnding;
		}

		if (!(whitePawns & isolated[from]))
		{
			scoreOp -= isolatedPenaltyOpening;
			scoreEd -= isolatedPenaltyEnding;
		}

		if ((pawnAttacks[White][from + 8] & blackPawns)) // && pos.getPiece(from - 8) == Empty)
		{
			if (!(whitePawns & backward[White][from]))
			{
				scoreOp -= backwardPenaltyOpening;
				scoreEd -= backwardPenaltyEnding;
			}
		}
	}

	tempPiece = blackPawns;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		if (blackPawns & rays[S][from])
		{
			scoreOp += doubledPenaltyOpening;
			scoreEd += doubledPenaltyEnding;
		}
		if (!(whitePawns & passed[Black][from]))
		{
			scoreOp -= passedBonusOpening;
			scoreEd -= passedBonusEnding;
		}
		if (!(blackPawns & isolated[from]))
		{
			scoreOp += isolatedPenaltyOpening;
			scoreEd += isolatedPenaltyEnding;
		}
		if ((pawnAttacks[Black][from - 8] & whitePawns))// && pos.getPiece(from - 8) == Empty)
		{
			if (!(blackPawns & backward[Black][from]))
			{
				scoreOp += backwardPenaltyOpening;
				scoreEd += backwardPenaltyEnding;
			}
		}
	}

	value = ((scoreOp * (256 - phase)) + (scoreEd * phase)) / 256;

	pttSave(pos, value);

	return value;
}

int kingSafetyEval(Position & pos, int phase, int score)
{
	int zone1, zone2;

	// White
	if (pos.getBitboard(White, King) & kingSide)
	{
		// Penalize pawns which have moved more than one square.
		zone1 = popcnt(0x00E0E0E0E0000000 & pos.getBitboard(White, Pawn));
		score -= pawnShelterAdvancedPawnPenalty * zone1;

		// Penalize missing pawns from our pawn shelter.
		// Penalize missing opponent pawns as they allow the opponent to use his semi-open/open files to attack us.
		for (int i = 5; i < 8; i++)
		{
			if (!(files[i] & pos.getBitboard(White, Pawn)))
			{
				score -= pawnShelterMissingPawnPenalty;
			}
			if (!(files[i] & pos.getBitboard(Black, Pawn)))
			{
				score -= pawnShelterMissingOpponentPawnPenalty;
			}
		}

		// Pawn storm evaluation.
		// Penalize pawns on the 6th rank(from black's point of view).
		zone1 = popcnt(0x0000000000E00000 & pos.getBitboard(Black, Pawn));
		score -= pawnStormClosePenalty * zone1;

		// Penalize pawns on the 5th rank(from black's point of view).
		zone2 = popcnt(0x00000000E0000000 & pos.getBitboard(Black, Pawn));
		score -= pawnStormFarPenalty * zone2;
	}
	else if (pos.getBitboard(White, King) & queenSide)
	{
		zone1 = popcnt(0x0007070707000000 & pos.getBitboard(White, Pawn));
		score -= pawnShelterAdvancedPawnPenalty * zone1;

		for (int i = 0; i < 3; i++)
		{
			if (!(files[i] & pos.getBitboard(White, Pawn)))
			{
				score -= pawnShelterMissingPawnPenalty;
			}
			if (!(files[i] & pos.getBitboard(Black, Pawn)))
			{
				score -= pawnShelterMissingOpponentPawnPenalty;
			}
		}

		zone1 = popcnt(0x0000000000070000 & pos.getBitboard(Black, Pawn));
		score -= pawnStormClosePenalty * zone1;

		zone2 = popcnt(0x0000000007000000 & pos.getBitboard(Black, Pawn));
		score -= pawnStormFarPenalty * zone2;
	}
	else 
	{
		// Penalize open files near the king.
		int kingFile = (bitScanForward(pos.getBitboard(White, King))) % 8;
		for (int i = -1; i <= 1; i++)
		{
			if (!(files[kingFile + i] & pos.getBitboard(White, Pawn) & pos.getBitboard(Black, Pawn)))
			{
				score -= kingInCenterOpenFilePenalty;
			}
		}
	}

	// Black
	if (pos.getBitboard(Black, King) & kingSide)
	{
		zone1 = popcnt(0x000000E0E0E0E000 & pos.getBitboard(Black, Pawn));
		score += pawnShelterAdvancedPawnPenalty * zone1;

		for (int i = 5; i < 8; i++)
		{
			if (!(files[i] & pos.getBitboard(Black, Pawn)))
			{
				score += pawnShelterMissingPawnPenalty;
			}
			if (!(files[i] & pos.getBitboard(White, Pawn)))
			{
				score += pawnShelterMissingOpponentPawnPenalty;
			}
		}

		zone1 = popcnt(0x0000E00000000000 & pos.getBitboard(White, Pawn));
		score += pawnStormClosePenalty * zone1;

		zone2 = popcnt(0x000000E000000000 & pos.getBitboard(White, Pawn));
		score += pawnStormFarPenalty * zone2;
	}
	else if (pos.getBitboard(Black, King) & queenSide)
	{
		zone1 = popcnt(0x0000000707070700 & pos.getBitboard(Black, Pawn));
		score += pawnShelterAdvancedPawnPenalty * zone1;

		for (int i = 0; i < 3; i++)
		{
			if (!(files[i] & pos.getBitboard(Black, Pawn)))
			{
				score += pawnShelterMissingPawnPenalty;
			}
			if (!(files[i] & pos.getBitboard(White, Pawn)))
			{
				score += pawnShelterMissingOpponentPawnPenalty;
			}
		}

		zone1 = popcnt(0x0000070000000000 & pos.getBitboard(White, Pawn));
		score += pawnStormClosePenalty * zone1;

		zone2 = popcnt(0x0000000700000000 & pos.getBitboard(White, Pawn));
		score += pawnStormFarPenalty * zone2;
	}
	else
	{
		int kingFile = (bitScanForward(pos.getBitboard(Black, King))) % 8;
		for (int i = -1; i <= 1; i++)
		{
			if (!(files[kingFile + i] & pos.getBitboard(White, Pawn) & pos.getBitboard(Black, Pawn)))
			{
				score += kingInCenterOpenFilePenalty;
			}
		}
	}

	return ((score * (256 - phase)) / 256);
}

int eval(Position & pos)
{
	int score, kingTropismScore = 0;
	int phase = pos.calculateGamePhase();

	// Checks if we are in a known endgame.
	// If we are we can straight away return the score for the endgame.
	// At the moment only detects draws, if wins will be included this must be made to return things in negamax fashion.
	if (knownEndgames.count(pos.getMaterialHash()))
	{
		return knownEndgames[pos.getMaterialHash()];
	}

	// Material + Piece-Square Tables
	score = ((pos.getScoreOp() * (256 - phase)) + (pos.getScoreEd() * phase)) / 256;

	if (popcnt(pos.getBitboard(White, Bishop)) == 2)
	{
		score += ((bishopPairBonusOpening * (256 - phase)) + (bishopPairBonusEnding * phase)) / 256;
	}
	if (popcnt(pos.getBitboard(Black, Bishop)) == 2)
	{
		score -= ((bishopPairBonusOpening * (256 - phase)) + (bishopPairBonusEnding * phase)) / 256;
	}

	// Most important rule of king and pawn endgames - the side with more pawns wins.
	// Therefore add a big bonus for the side with more pawns.
	// Maybe add code dealing with doubled isolated pawns? Make them only worth one pawn?
	if (phase == 256)
	{
		score += (popcnt(pos.getBitboard(White, Pawn)) - popcnt(pos.getBitboard(Black, Pawn))) * pieceValuesEnding[Pawn];
	}

	// Mobility
	score += mobilityEval(pos, phase, kingTropismScore);

	// Pawn structure
	score += pawnStructureEval(pos, phase);

	// King safety
	score += kingSafetyEval(pos, phase, kingTropismScore);

	if (pos.getSideToMove())
	{
		return -score;
	}
	else
	{
		return score;
	}
}