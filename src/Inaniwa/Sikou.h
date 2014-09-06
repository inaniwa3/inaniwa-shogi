#pragma once

#include "Kyokumen.h"

#define MAX_DEPTH 32
//#define INFINITE 999999
#define INFINITEVAL 999999 // INFINITEという定数はwinbase.hで定義されているので、名前を変更した。

enum {
	EXACTLY_VALUE,		// 値は局面の評価値そのもの
	LOWER_BOUND,		// 値は局面の評価値の下限値(val>=β)
	UPPER_BOUND			// 値は局面の評価値の上限値(val<=α)
};


class HashEntry {
public:
	uint64 HashVal;		// ハッシュ値
	Te Best;			// 前回の反復深化での最善手
	Te Second;			// 前々回以前の反復深化での最善手
	int value;			// αβ探索で得た局面の評価値
	int flag;			// αβ探索で得た値が、局面の評価値そのものか、上限値か下限値か
	int Tesu;			// αβ探索を行った際の手数
	short depth;		// αβ探索を行った際の深さ
	short remainDepth;	// αβ探索を行った際の残り深さ
};

class Sikou {
protected:
	static HashEntry HashTbl[1024*1024];	// 20bitの大きさ、40Mバイト
	int MakeMoveFirst(int SorE,int depth,Te teBuf[],KyokumenKomagumi k);
public:
	// とりあえず、何らかの局面を与えて手を返す関数です。
	//Te Think(int SorE,KyokumenKomagumi k);
	Te Think(int SorE,KyokumenKomagumi k, bool isUseJoseki = true, Te* ponderTe = NULL);
	void ClearHash();
	// 第４章で追加。ある深さでの最善手を保持する。
	Te Best[MAX_DEPTH][MAX_DEPTH];
	int MinMax(int SorE,KyokumenKomagumi &k,int depth,int depthMax);
	int AlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax);
	int NegaAlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax,bool bITDeep=true);
	int ITDeep(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax);	// NegaAlphaBetaを用いた反復深化

	Te   InaniwaTime      (int SorE, KyokumenKomagumi &k);            //ina//
	bool InaniwaAlgorithm0(int SorE, KyokumenKomagumi &k);            //ina//
	Te   InaniwaAlgorithm1(int SorE, KyokumenKomagumi &k);            //ina//
	Te   InaniwaAlgorithm3(int SorE, KyokumenKomagumi &k);            //ina//
	Te   InaniwaAlgorithm4(int SorE, KyokumenKomagumi &k);            //ina//
	Te   InaniwaAlgorithmA(int SorE, KyokumenKomagumi &k);            //ina//
	Te   InaniwaAlgorithmD(int SorE, KyokumenKomagumi &k);            //ina//
	Te   InaniwaAlgorithmE(int SorE, KyokumenKomagumi &k);            //ina//
	Te   InaniwaAlgorithmF(int SorE, KyokumenKomagumi &k, int danTH); //ina//
	Te   InaniwaAlgorithmG(int SorE, KyokumenKomagumi &k);            //ina//
};
