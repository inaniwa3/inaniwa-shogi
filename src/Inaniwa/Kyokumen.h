#pragma once

#include <stdio.h>
#include <time.h>

typedef unsigned char KomaInf;

enum {
	// �����Ȃ��Ƃ���
	EMPTY=0,
	EMP=0,				// �R�������������Ă����ƃ\�[�X�����₷���̂Łi�΁j
	// �����ɂ���ڈ�i�P�r�b�g�j
	PROMOTED=1<<3,

	// �������킷���l
	FU=1,
	KY=2,
	KE=3,
	GI=4,
	KI=5,
	KA=6,
	HI=7,
	OU=8,
	TO=FU+PROMOTED,
	NY=KY+PROMOTED,
	NK=KE+PROMOTED,
	NG=GI+PROMOTED,
	UM=KA+PROMOTED,
	RY=HI+PROMOTED,
	
	// �������g�̋�ɂ���ڈ�i�P�r�b�g�j
	SELF=1<<4,
	// �G�̋�ɂ���ڈ�(�P�r�b�g)
	ENEMY=1<<5,
	// �G���������i�߂Ȃ��Ƃ���i�Ղ̊O�j�̖ڈ�
	WALL=SELF+ENEMY,

	// ���ۂ̋�
	SFU=SELF+FU,		//�����̕�
	STO=SELF+TO,		//�����̂Ƌ�
	SKY=SELF+KY,		//�����̍���
	SNY=SELF+NY,		//�����̐��荁
	SKE=SELF+3,			//�����̌j�n
	SNK=SKE+PROMOTED,	//�����̐���j
	SGI=SELF+4,			//�����̋�
	SNG=SGI+PROMOTED,	//�����̐����
	SKI=SELF+5,			//�����̋�
	SKA=SELF+6,			//�����̊p
	SUM=SKA+PROMOTED,	//�����̔n
	SHI=SELF+7,			//�����̔��
	SRY=SHI+PROMOTED,	//�����̗�
	SOU=SELF+8,			//�����̋�

	EFU=ENEMY+1,		//�G�̕�
	ETO=EFU+PROMOTED,	//�G�̂Ƌ�
	EKY=ENEMY+2,		//�G�̍���
	ENY=EKY+PROMOTED,	//�G�̐��荁
	EKE=ENEMY+3,		//�G�̌j�n
	ENK=EKE+PROMOTED,	//�G�̐���j
	EGI=ENEMY+4,		//�G�̋�
	ENG=EGI+PROMOTED,	//�G�̐����
	EKI=ENEMY+5,		//�G�̋�
	EKA=ENEMY+6,		//�G�̊p
	EUM=EKA+PROMOTED,	//�G�̔n
	EHI=ENEMY+7,		//�G�̔��
	ERY=EHI+PROMOTED,	//�G�̗�
	EOU=ENEMY+8,		//�G�̋�
};

extern int Direct[12];
extern int CanPromote[];
extern int CanMove[12][64];
extern int CanJump[12][64];
extern const char *komaStr[];
extern const char *komaStr2[];

#define OLDMOVE 0
#define INANIWA_MAX_TESU 4096 //ina//

extern const char *danSFENNameArray[];
extern const char *mochiGomaSFENNameArray[];

// �R�͂Œǉ��B��̉��l�Ǝ�����̉��l
extern int KomaValue[];
extern int HandValue[];

// �����́A�W�����̗����{�j�n�̂S�����̗����{��ы�̂W�����̗����������܂��B
// �e�����ɂP�r�b�g�����蓖�ĂāA���v�Q�O�r�b�g���K�v�ł��B
// �����ł́Aint���R�Q�r�b�g�Ɖ��肵�Ă��܂��B
// �������A���Z�̍������̂��߂ɁA��ы�̗����͏�ʂP�U�r�b�g�̂����̂W�r�b�g�𗘗p���܂��B
typedef unsigned int Kiki;

// 5�͂Œǉ��B�U�S�r�b�g�^�̐����^
typedef long long int64;
typedef unsigned long long uint64;

class Kyokumen;

// ��̃N���X
class Te {
public:
	// �ǂ�����E�ǂ��ւ͂��ꂼ��PByte�ł���킹�܂��B
	// �ڂ����͋ǖʃN���X���Q�Ƃ��ĉ������B
	unsigned char from,to;
	// ����������
	KomaInf koma;
	// �������
	KomaInf capture;
	// ��/�s����
	unsigned char promote;
	// ����́A��̐����̍ۂɎ�ʂ�p���������Ɏg���܂��i�����̊g���p�j
	unsigned char Kind;
	// ���̎�̉��]���i��̉��l�j�ł�
	short value;
public:
	inline bool IsNull() {
		return from==0 && to==0;
	}
	// Te�̔z���錾���邱�Ƃ������̂ŁA��̃R���X�g���N�^��p�ӂ��܂��B
	// �������邱�ƂŁATe�̔z���錾���邽�тɖ��ʂȏ��������������邱�Ƃ�������܂��B
	inline Te() {};
	// �{����Te����̃f�[�^�ŏ��������������̂��߂̃R���X�g���N�^�ł��B
	inline Te(int i) {
		from=to=koma=capture=promote=Kind=0;
		value=0;
	}
	inline Te(int f,int t,KomaInf k,KomaInf c,int p=0,int K=0,int v=0) {
		from=f;
		to=t;
		koma=k;
		capture=c;
		promote=p;
		Kind=K;
		value=v;
	}
	Te (int SorE,unsigned char f,unsigned char t,const Kyokumen &k);
	// ���\�����������Ɏg���܂��B
	void Print() {
		FPrint(stdout);
	}
	// ����
	void FPrint(FILE *fp);

	//void PrintToBuf(char *buf);

	// ��̓��ꐫ���r�������Ƃ��Ɏg���܂��BKind��Value������Ă�������ł��B
	int operator==(const Te &a) {
		return a.from==from && a.to==to && a.koma==koma && a.promote==promote;
	}
};

// �ǖʂ̃N���X
class Kyokumen {
public:
	// �j�n�̗�����ban����͂ݏo���̂ŁA�͂ݏo�������m�ۂ��Ă����܂��B
	// C++�ł́A�\���̂̓����̕ϐ��̕��я��͐錾�������ɂȂ邱�Ƃ𗘗p���Ă��܂��B
	// ���ʂ͂��܂�g��Ȃ��u�����v�e�N�j�b�N�ł����ǁA���������e�N�j�b�N������Ƃ������ƂŁB
	KomaInf banpadding[16];
	// 2�����z����g���ƒx���̂ŁA�P�����z����g���܂��B�܂��A�|���Z�̍ۂɁA���X�Ƃ���p��������A
	// 2�̊K����|���Z�Ɏg���Ɗ|���Z�������Ȃ�̂ŁA���P�U���g���܂��B
	// ��̈ʒu�́A�Ⴆ�΂V���Ȃ�A�V���P�U�{���̂悤�ɂ���킵�܂��B
	// �܂�A�V���Ȃ�P�U�i����0x77�ɂȂ�킯�ł��B
	KomaInf ban[16*11];

	// ��̗�����ێ����܂��B�G�̋�Ǝ����̋�̗����͕ʁX�ɕێ����܂��B
	Kiki controlS[16*11];
	Kiki controlE[16*11];

	// ������ł��B����������ɂȂ邱�Ƃ͂Ȃ��̂ŁAEHI�܂łŏ\���ł��B
	// Hand[FU]���P�Ȃ���̎�����ɕ����P���AHand[EKI]���R�Ȃ�G�̎�����ɋ����R���Ƃ����v�̂ł��B
	int Hand[EHI+1];

	// ���̋ǖʂ̎萔�ł��B
	int Tesu;

	// �݂��̋ʂ̈ʒu�ł��B
	int kingS;
	int kingE;

	// �R�͂Œǉ��B�ǖʂ̕]���l
	int value;
protected:
	// �P�͂Œǉ��BcontrolS,controlE�̏������B
	void InitControl();
public:
	void InitKyokumen(int tesu, KomaInf board[9][9], int Motigoma[]); // �ǖʂ�����������B
protected:	
	// �P�͂Œǉ��B��������𓮂������߂̊֐��Q
	int Utifudume(int SorE,int to,int *pin);
	void MoveKing(int SorE,int &teNum,Te *teTop,Kiki kiki);			//�ʂ̓�����̐���
	void MoveTo(int SorE,int &teNum,Te *teTop,int to,int *pin);		//to�ɓ�����̐���
	void PutTo(int SorE,int &teNum,Te *teTop,int to,int *pin);		//to�ɋ��ł�̐���
public: //ina//
	Kiki CountControlS(int pos);
	Kiki CountControlE(int pos);
protected: //ina//
	Kiki CountMove(int SorE,int pos,int *pin);
	void AddMoves(int SorE,int &teNum,Te *teTop,int from,int pin,int Rpin=0);
	void AddStraight(int SorE,int &teNum,Te *teTop,int from,int dir,int pin,int Rpin=0);
	void AddMove(int SorE,int &teNum,Te *teTop,int from,int diff,int pin,int Rpin=0);

	// �R�͂Œǉ��B�����l�����߂邽�߂̊֐��Q
	int IsCorrectMove(Te &te);
	int EvalMin(Te *MoveS,int NumMoveS,Te *MoveE,int NumMoveE);
	int EvalMax(Te *MoveS,int NumMoveS,Te *MoveE,int NumMoveE);
	int Eval(int pos);

	// 5�͂Œǉ��B�n�b�V���̎�
	static uint64 HashSeed[ERY+1][0x99+1];
	static uint64 HandHashSeed[EHI+1][18+1];
 
public:
	static void HashInit();

	uint64 KyokumenHashVal;
	uint64 HandHashVal;
	uint64 HashVal;

	// 1000�肭�炢�����w���Ȃ����Ƃɂ��Ēu���܂��傤�B
//ina// 	static uint64 HashHistory[1000];
//ina// 	static int OuteHistory[1000];
	static uint64 HashHistory[INANIWA_MAX_TESU]; //ina//
	static int OuteHistory[INANIWA_MAX_TESU];    //ina//

	// �P�͂Œǉ��Bpos����dir�����ɂ��鉽����i�������͕ǁj��T���B
	inline int search(int pos,int dir) const {
		do {
			pos+=dir;
		}while(ban[pos]==EMPTY);
		return pos;
	}
	Kyokumen() {};
	// �Q�͂Œǉ��B�ՖʂƎ������ǖʂ𐶐�����R���X�g���N�^
	Kyokumen(int tesu,KomaInf ban[9][9],int Motigoma[]);
	// �Q�͂Œǉ��B��ʂ֋ǖʂ��󎚂���B
	void Print() {
		FPrint(stdout);
	}
	// �Q�͂Œǉ��B�t�@�C���ɋǖʂ��󎚂���B
	void FPrint(FILE *fp);
	// �P�͂Œǉ��B��̐����������̐��������邽�߂̊֐��Q
	void MakePinInf(int *pin) const;
	int MakeLegalMoves(int SorE,Te *tebuf,int *pin=NULL);
	int AntiCheck(int SorE,Te *tebuf,int *pin,Kiki control);

	// 2�͂Œǉ��B���ۂɂP�蓮�����B
	void Move(int SorE,const Te &te);

	// �R�͂Œǉ��B��ԑ��̈�ԗǂ������l��T���ĕԂ��B
	int BestEval(int SorE);

	int IsLegalMove(int SorE,Te &te);

	int operator==(const Kyokumen &k) {
		int ret=true;
		for(int suji=0x10;suji<=0x90;suji+=0x10) {
			for(int dan=1;dan<=9;dan++) {
				if (ban[suji+dan]!=k.ban[suji+dan]) {
					ret=false;
					break;
				}
			}
		}
		return ret;
	}
	void Dump();
	// ��X�͂Œǉ��B����𐶐��B
	int MakeChecks(int SorE,Te *tebuf,int *pin=NULL);
	// ��X�͂Œǉ��B�l�ߏ������[�`���B
	int Mate(int SorE,int maxDepth,Te &te);
	int CheckMate(int SorE,int depth, int depthMax, Te *checks,Te &te);
	int AntiCheckMate(int SorE,int depth, int depthMax, Te *checks);

	// �ȉ��ALesserkai�œƎ��ɒǉ��B
	Te GetTsumeTe(int SorE); // �l���������������A�U�ߕ��̎���擾����B
	bool IsNyugyokuWin(int SorE); // ���ʂŏ��������iCSA���[���Ɋ�Â��Ĕ��肷��B�j
};

// 7�͂Œǉ��B��g�݂̃{�[�i�X�_��^����B
class KyokumenKomagumi: public Kyokumen {
public:
	static int KomagumiValue[ERY+1][16*11];
	// �U�ߋ�A����̑���ʁE���ʂƂ̑��Έʒu�ɂ��]���_
	static int SemegomaValueS[16*11][16*11];
	static int SemegomaValueE[16*11][16*11];
	static int MamorigomaValueS[16*11][16*11];
	static int MamorigomaValueE[16*11][16*11];
public:
	// �I�Փx�F���Ղ���I�ՂɌ������ďオ���Ă������l�B�傫���قǋl�݂��߂��i�͂��j
	int Shuubando[2];
	// ��g�݂ɂ��{�[�i�X�_
	int KomagumiBonus[2];
	// �U�ߋ�A����ɂ��{�[�i�X�_
	int SemegomaBonus[2];
	int MamorigomaBonus[2];
	KyokumenKomagumi(int tesu,KomaInf ban[9][9],int Motigoma[]) : Kyokumen(tesu,ban,Motigoma) {
		InitShuubando();
		InitBonus();
	}
	KyokumenKomagumi() {} // �R���p�C���G���[������邽�߁B
	void InitKyokumen(int tesu,KomaInf ban[9][9],int Motigoma[]) // ��ԍŏ��ɋǖʂ�����������Ƃ��ɌĂԁB
	{
		Kyokumen::InitKyokumen(tesu, ban, Motigoma);
		InitShuubando();
		InitBonus();
	}

	void Initialize();
	static void InitKanagomaValue();
	void InitShuubando();
	void InitBonus();
	void SenkeiInit();
	void Move(int SorE,const Te &te);
	void narabe(int tesu,const char *t[]);
	// ���܂ł�value�̑���B
	int Evaluate();
	// 8�͂Œǉ��B��̕]���ƕ��ёւ�������B
	void EvaluateTe(int SorE,int teNum,Te *te);
};

class Joseki {
	unsigned char **JosekiData;	// ��Ղ̊����f�[�^
	int JosekiSize;		// ���̒�Ղ̃T�C�Y�i�������������邩�j
	Joseki *child;		// ��Ճt�@�C���𕡐��I�������ꍇ�A�q�N���X���ʂ̒�Ճt�@�C���������Ă���B
public:
	// �������Bfilenames�ɂ́A,��؂�ŕ����̃t�@�C������^���邱�Ƃ��o����B
	Joseki(char *filenames);
	// shoki�́A�����ǖʁB�i����̏ꍇ�ȂǂɑΉ����邽�߁B�j
	// ���݂̋ǖ�k����Ճf�[�^���ɂ������ꍇ�A���̋ǖʂłǂ�Ȏ肪�ǂꂭ�炢�̕p�x�Ŏw���ꂽ����Ԃ��B
	// �������A�������g�̒�Փ��Ō������ꍇ�́Achild�܂Œ�Ղ��Q�Ƃ͂��Ȃ��B
	// ����ɂ��A�D�悵�Ďw����������Ղ��t�@�C���̐擪�ɒu���A�����I�Ȓ�Ղ��Ō�ɒu�����ƂŁA
	// ����w������D�悵�A���̒�Ղ���O�ꂽ�ꍇ�ɑ傫�Ȓ�ՂŃJ�o�[���邱�Ƃ��s����B
	void fromJoseki(Kyokumen &shoki,int shokiTeban,Kyokumen &k,int tesu,int &teNum,Te te[],int hindo[]); 
};


#define RETRY_MAX 9

struct TsumeVal {
	uint64 HashVal;
	uint64 Motigoma;
	int NextEntry;
	int mate;
	Te te;
};

#define TSUME_HASH_SIZE 1024*1024
#define TSUME_HASH_AND  (TSUME_HASH_SIZE-1)

class TsumeHash {
	static TsumeVal HashTbl[TSUME_HASH_SIZE];
	static uint64 FU_BIT_TBL[19];
	static uint64 KY_BIT_TBL[5];
	static uint64 KE_BIT_TBL[5];
	static uint64 GI_BIT_TBL[5];
	static uint64 KI_BIT_TBL[5];
	static uint64 KA_BIT_TBL[3];
	static uint64 HI_BIT_TBL[3];
	static TsumeVal *FindFirst(uint64 KyokumenHashVal);
	static TsumeVal *FindNext(TsumeVal* Now);
	static uint64 CalcHand(int Motigoma[]);
public:
	static void Clear();
	static void Add(uint64 KyokumenHashVal,uint64 HandHashVal,int Motigoma[],int mate,Te te);
	static TsumeVal *Find(uint64 KyokumenHashVal,uint64 HandHashVal,int Motigoma[]);
	static TsumeVal *DomSearchCheckMate(uint64 KyokumenHashVal,int Motigoma[]);
	static TsumeVal *DomSearchAntiMate(uint64 KyokumenHashVal,int Motigoma[]);
};
