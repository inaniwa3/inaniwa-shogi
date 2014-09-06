#pragma once

#include "Kyokumen.h"

// USIコマンドを元に局面の作成などを行うクラス

class USIUtil
{
public:
	static KomaInf MochigomaSFENKomaNameToValue(char komaName, int SorE);
	static KomaInf SFENKomaNameToValue(char komaName);
	static void ClearMochigoma(int motigoma[]);
	static void MakeCustomKyokumen(const char *buf, KomaInf customBan[9][9], int motigoma[]);
	static int AddMove(const char *buf, Kyokumen *k, int SorE, int teNum, Te *teBuf);
	static int AddAllMoves(const char *buf, int len, Kyokumen *k, int& SorE, int& teNum, Te *teBuf);
	static int ParseTime(const char *buf);
	static void ParseAllTimes(const char *buf, int SorE);
	static void ParseLimitTimes(const char *buf);
};
