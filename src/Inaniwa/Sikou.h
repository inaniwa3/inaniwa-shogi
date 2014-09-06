#pragma once

#include "Kyokumen.h"

#define MAX_DEPTH 32
//#define INFINITE 999999
#define INFINITEVAL 999999 // INFINITE�Ƃ����萔��winbase.h�Œ�`����Ă���̂ŁA���O��ύX�����B

enum {
	EXACTLY_VALUE,		// �l�͋ǖʂ̕]���l���̂���
	LOWER_BOUND,		// �l�͋ǖʂ̕]���l�̉����l(val>=��)
	UPPER_BOUND			// �l�͋ǖʂ̕]���l�̏���l(val<=��)
};


class HashEntry {
public:
	uint64 HashVal;		// �n�b�V���l
	Te Best;			// �O��̔����[���ł̍őP��
	Te Second;			// �O�X��ȑO�̔����[���ł̍őP��
	int value;			// �����T���œ����ǖʂ̕]���l
	int flag;			// �����T���œ����l���A�ǖʂ̕]���l���̂��̂��A����l�������l��
	int Tesu;			// �����T�����s�����ۂ̎萔
	short depth;		// �����T�����s�����ۂ̐[��
	short remainDepth;	// �����T�����s�����ۂ̎c��[��
};

class Sikou {
protected:
	static HashEntry HashTbl[1024*1024];	// 20bit�̑傫���A40M�o�C�g
	int MakeMoveFirst(int SorE,int depth,Te teBuf[],KyokumenKomagumi k);
public:
	// �Ƃ肠�����A���炩�̋ǖʂ�^���Ď��Ԃ��֐��ł��B
	//Te Think(int SorE,KyokumenKomagumi k);
	Te Think(int SorE,KyokumenKomagumi k, bool isUseJoseki = true, Te* ponderTe = NULL);
	void ClearHash();
	// ��S�͂Œǉ��B����[���ł̍őP���ێ�����B
	Te Best[MAX_DEPTH][MAX_DEPTH];
	int MinMax(int SorE,KyokumenKomagumi &k,int depth,int depthMax);
	int AlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax);
	int NegaAlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax,bool bITDeep=true);
	int ITDeep(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax);	// NegaAlphaBeta��p���������[��

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
