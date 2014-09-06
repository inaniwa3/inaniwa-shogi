#pragma once

#include <stdio.h>
#include <time.h>

typedef unsigned char KomaInf;

enum {
	// 何もないところ
	EMPTY=0,
	EMP=0,				// ３文字も準備しておくとソースが見やすいので（笑）
	// 成り駒につける目印（１ビット）
	PROMOTED=1<<3,

	// 駒をあらわす数値
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
	
	// 自分自身の駒につける目印（１ビット）
	SELF=1<<4,
	// 敵の駒につける目印(１ビット)
	ENEMY=1<<5,
	// 敵も味方も進めないところ（盤の外）の目印
	WALL=SELF+ENEMY,

	// 実際の駒
	SFU=SELF+FU,		//味方の歩
	STO=SELF+TO,		//味方のと金
	SKY=SELF+KY,		//味方の香車
	SNY=SELF+NY,		//味方の成り香
	SKE=SELF+3,			//味方の桂馬
	SNK=SKE+PROMOTED,	//味方の成り桂
	SGI=SELF+4,			//味方の銀
	SNG=SGI+PROMOTED,	//味方の成り銀
	SKI=SELF+5,			//味方の金
	SKA=SELF+6,			//味方の角
	SUM=SKA+PROMOTED,	//味方の馬
	SHI=SELF+7,			//味方の飛車
	SRY=SHI+PROMOTED,	//味方の龍
	SOU=SELF+8,			//味方の玉

	EFU=ENEMY+1,		//敵の歩
	ETO=EFU+PROMOTED,	//敵のと金
	EKY=ENEMY+2,		//敵の香車
	ENY=EKY+PROMOTED,	//敵の成り香
	EKE=ENEMY+3,		//敵の桂馬
	ENK=EKE+PROMOTED,	//敵の成り桂
	EGI=ENEMY+4,		//敵の銀
	ENG=EGI+PROMOTED,	//敵の成り銀
	EKI=ENEMY+5,		//敵の金
	EKA=ENEMY+6,		//敵の角
	EUM=EKA+PROMOTED,	//敵の馬
	EHI=ENEMY+7,		//敵の飛車
	ERY=EHI+PROMOTED,	//敵の龍
	EOU=ENEMY+8,		//敵の玉
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

// ３章で追加。駒の価値と持ち駒の価値
extern int KomaValue[];
extern int HandValue[];

// 利きは、８方向の利き＋桂馬の４方向の利き＋飛び駒の８方向の利きを扱います。
// 各利きに１ビットを割り当てて、合計２０ビットが必要です。
// ここでは、intが３２ビットと仮定しています。
// ただし、演算の高速性のために、飛び駒の利きは上位１６ビットのうちの８ビットを利用します。
typedef unsigned int Kiki;

// 5章で追加。６４ビット型の整数型
typedef long long int64;
typedef unsigned long long uint64;

class Kyokumen;

// 手のクラス
class Te {
public:
	// どこから・どこへはそれぞれ１Byteであらわせます。
	// 詳しくは局面クラスを参照して下さい。
	unsigned char from,to;
	// 動かした駒
	KomaInf koma;
	// 取った駒
	KomaInf capture;
	// 成/不成り
	unsigned char promote;
	// これは、手の生成の際に種別を用いたい時に使います（将来の拡張用）
	unsigned char Kind;
	// その手の仮評価（手の価値）です
	short value;
public:
	inline bool IsNull() {
		return from==0 && to==0;
	}
	// Teの配列を宣言することが多いので、空のコンストラクタを用意します。
	// こうすることで、Teの配列を宣言するたびに無駄な初期化処理が走ることを避けられます。
	inline Te() {};
	// 本当にTeを空のデータで初期化したい時のためのコンストラクタです。
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
	// 手を表示したい時に使います。
	void Print() {
		FPrint(stdout);
	}
	// 同上
	void FPrint(FILE *fp);

	//void PrintToBuf(char *buf);

	// 手の同一性を比較したいときに使います。KindやValueが違っても同じ手です。
	int operator==(const Te &a) {
		return a.from==from && a.to==to && a.koma==koma && a.promote==promote;
	}
};

// 局面のクラス
class Kyokumen {
public:
	// 桂馬の利きがbanからはみ出すので、はみ出す分を確保しておきます。
	// C++では、構造体の内部の変数の並び順は宣言した順になることを利用しています。
	// 普通はあまり使わない「汚い」テクニックですけど、こういうテクニックもあるということで。
	KomaInf banpadding[16];
	// 2次元配列を使うと遅いので、１次元配列を使います。また、掛け算の際に、＊９とかを用いるよりも、
	// 2の階乗を掛け算に使うと掛け算が早くなるので、＊１６を使います。
	// 駒の位置は、例えば７七なら、７＊１６＋七のようにあらわします。
	// つまり、７七なら１６進数で0x77になるわけです。
	KomaInf ban[16*11];

	// 駒の利きを保持します。敵の駒と自分の駒の利きは別々に保持します。
	Kiki controlS[16*11];
	Kiki controlE[16*11];

	// 持ち駒です。王が持ち駒になることはないので、EHIまでで十分です。
	// Hand[FU]が１なら先手の持ち駒に歩が１枚、Hand[EKI]が３なら敵の持ち駒に金が３枚という要領です。
	int Hand[EHI+1];

	// この局面の手数です。
	int Tesu;

	// 互いの玉の位置です。
	int kingS;
	int kingE;

	// ３章で追加。局面の評価値
	int value;
protected:
	// １章で追加。controlS,controlEの初期化。
	void InitControl();
public:
	void InitKyokumen(int tesu, KomaInf board[9][9], int Motigoma[]); // 局面を初期化する。
protected:	
	// １章で追加。正しく駒を動かすための関数群
	int Utifudume(int SorE,int to,int *pin);
	void MoveKing(int SorE,int &teNum,Te *teTop,Kiki kiki);			//玉の動く手の生成
	void MoveTo(int SorE,int &teNum,Te *teTop,int to,int *pin);		//toに動く手の生成
	void PutTo(int SorE,int &teNum,Te *teTop,int to,int *pin);		//toに駒を打つ手の生成
public: //ina//
	Kiki CountControlS(int pos);
	Kiki CountControlE(int pos);
protected: //ina//
	Kiki CountMove(int SorE,int pos,int *pin);
	void AddMoves(int SorE,int &teNum,Te *teTop,int from,int pin,int Rpin=0);
	void AddStraight(int SorE,int &teNum,Te *teTop,int from,int dir,int pin,int Rpin=0);
	void AddMove(int SorE,int &teNum,Te *teTop,int from,int diff,int pin,int Rpin=0);

	// ３章で追加。交換値を求めるための関数群
	int IsCorrectMove(Te &te);
	int EvalMin(Te *MoveS,int NumMoveS,Te *MoveE,int NumMoveE);
	int EvalMax(Te *MoveS,int NumMoveS,Te *MoveE,int NumMoveE);
	int Eval(int pos);

	// 5章で追加。ハッシュの種
	static uint64 HashSeed[ERY+1][0x99+1];
	static uint64 HandHashSeed[EHI+1][18+1];
 
public:
	static void HashInit();

	uint64 KyokumenHashVal;
	uint64 HandHashVal;
	uint64 HashVal;

	// 1000手くらいしか指さないことにして置きましょう。
//ina// 	static uint64 HashHistory[1000];
//ina// 	static int OuteHistory[1000];
	static uint64 HashHistory[INANIWA_MAX_TESU]; //ina//
	static int OuteHistory[INANIWA_MAX_TESU];    //ina//

	// １章で追加。posからdir方向にある何か駒（もしくは壁）を探す。
	inline int search(int pos,int dir) const {
		do {
			pos+=dir;
		}while(ban[pos]==EMPTY);
		return pos;
	}
	Kyokumen() {};
	// ２章で追加。盤面と持ち駒から局面を生成するコンストラクタ
	Kyokumen(int tesu,KomaInf ban[9][9],int Motigoma[]);
	// ２章で追加。画面へ局面を印字する。
	void Print() {
		FPrint(stdout);
	}
	// ２章で追加。ファイルに局面を印字する。
	void FPrint(FILE *fp);
	// １章で追加。駒の正しい動きの生成をするための関数群
	void MakePinInf(int *pin) const;
	int MakeLegalMoves(int SorE,Te *tebuf,int *pin=NULL);
	int AntiCheck(int SorE,Te *tebuf,int *pin,Kiki control);

	// 2章で追加。実際に１手動かす。
	void Move(int SorE,const Te &te);

	// ３章で追加。手番側の一番良い交換値を探して返す。
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
	// 第９章で追加。王手を生成。
	int MakeChecks(int SorE,Te *tebuf,int *pin=NULL);
	// 第９章で追加。詰め将棋ルーチン。
	int Mate(int SorE,int maxDepth,Te &te);
	int CheckMate(int SorE,int depth, int depthMax, Te *checks,Te &te);
	int AntiCheckMate(int SorE,int depth, int depthMax, Te *checks);

	// 以下、Lesserkaiで独自に追加。
	Te GetTsumeTe(int SorE); // 詰将棋を解いた時、攻め方の手を取得する。
	bool IsNyugyokuWin(int SorE); // 入玉で勝ったか（CSAルールに基づいて判定する。）
};

// 7章で追加。駒組みのボーナス点を与える。
class KyokumenKomagumi: public Kyokumen {
public:
	static int KomagumiValue[ERY+1][16*11];
	// 攻め駒、守り駒の相手玉・自玉との相対位置による評価点
	static int SemegomaValueS[16*11][16*11];
	static int SemegomaValueE[16*11][16*11];
	static int MamorigomaValueS[16*11][16*11];
	static int MamorigomaValueE[16*11][16*11];
public:
	// 終盤度：序盤から終盤に向かって上がっていく数値。大きいほど詰みが近い（はず）
	int Shuubando[2];
	// 駒組みによるボーナス点
	int KomagumiBonus[2];
	// 攻め駒、守り駒によるボーナス点
	int SemegomaBonus[2];
	int MamorigomaBonus[2];
	KyokumenKomagumi(int tesu,KomaInf ban[9][9],int Motigoma[]) : Kyokumen(tesu,ban,Motigoma) {
		InitShuubando();
		InitBonus();
	}
	KyokumenKomagumi() {} // コンパイルエラーを避けるため。
	void InitKyokumen(int tesu,KomaInf ban[9][9],int Motigoma[]) // 一番最初に局面を初期化するときに呼ぶ。
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
	// 今までのvalueの代わり。
	int Evaluate();
	// 8章で追加。手の評価と並び替えをする。
	void EvaluateTe(int SorE,int teNum,Te *te);
};

class Joseki {
	unsigned char **JosekiData;	// 定跡の棋譜データ
	int JosekiSize;		// この定跡のサイズ（棋譜が何枚あるか）
	Joseki *child;		// 定跡ファイルを複数選択した場合、子クラスが別の定跡ファイルを持っている。
public:
	// 初期化。filenamesには、,区切りで複数のファイル名を与えることが出来る。
	Joseki(char *filenames);
	// shokiは、初期局面。（駒落ちの場合などに対応するため。）
	// 現在の局面kが定跡データ内にあった場合、その局面でどんな手がどれくらいの頻度で指されたかを返す。
	// ただし、自分自身の定跡内で見つけた場合は、childまで定跡を参照はしない。
	// これにより、優先して指させたい定跡をファイルの先頭に置き、統括的な定跡を最後に置くことで、
	// ある指し方を優先しつつ、その定跡から外れた場合に大きな定跡でカバーすることが行える。
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
