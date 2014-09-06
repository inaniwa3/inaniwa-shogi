#ifdef _WIN32
#include <windows.h>
#else
#include "WinAPI.h"
#endif

#include <string.h>
#include <algorithm>
#include <memory>

#include "Sikou.h"

using namespace std;

HashEntry Sikou::HashTbl[1024*1024];

extern bool isStopReceived; // stopコマンドを受信したか
extern bool canPonder; // 先読み可能か
extern bool canThrow; // 思考中断が可能かどうか
extern unsigned long thinkStartTime; // 思考開始時刻
extern unsigned long evaluatedNodes; // KyokumenKomagumi::Evaluate()が呼ばれた回数
extern unsigned long hashCount; // ハッシュに追加された数
extern int thinkDepthMax; // 読みの最大深さ。通常は4だが、go infiniteで思考する場合だけ8にする。
extern int InaniwaTimeTesu; //ina//

int itDeepCount = 0;

#define SHOWINFO 1
#define SHOWCURRMOVE 1

void Sikou::ClearHash()
{
	memset(HashTbl, 0, sizeof(HashTbl));
}

int Sikou::MakeMoveFirst(int SorE,int depth,Te teBuf[],KyokumenKomagumi k)
{
	int teNum=0;
	if (HashTbl[k.HashVal & 0xfffff].HashVal!=k.HashVal) {
		return 0;
	}
	if (HashTbl[k.HashVal & 0xfffff].Tesu%2!=k.Tesu%2) {
		// 手番が違う。
		return 0;
	}

	// 局面が一致したと思われる
	Te te=HashTbl[k.HashVal & 0xfffff].Best;
	if (!te.IsNull()) {
		if (k.IsLegalMove(SorE,te)) {
			teBuf[teNum++]=te;
		}
	}
	te=HashTbl[k.HashVal & 0xfffff].Second;
	if (!te.IsNull()) {
		if (k.IsLegalMove(SorE,te)) {
			teBuf[teNum++]=te;
		}
	}
	if (depth>1) {
		te=Best[depth-1][depth];
		if (!te.IsNull() && k.IsLegalMove(SorE,te)) {
			teBuf[teNum++]=te;
		}
	}
	return teNum;
}

Te Stack[32];	// 32と言う数字は適当。最大深さがこの程度までしか行かないことを期待している。

// 第４章で追加。αβ法による探索。
// 今後は、このアルゴリズムを基本に進めていく。
int Sikou::NegaAlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax,bool bITDeep)
{
	canThrow = depth > 1; // depthが1だと現在の最善手が確定していないので、throw可能なのはdepthが2以上になってから。
	if (isStopReceived) {
		if (canThrow) {
			throw 0; // 戻り先はSikou::ITDeep()
		}
	}
	if (depth==1) {
		// 千日手チェック
		int sennitite=0;
		for(int i=k.Tesu;i>0;i-=2) {
			if (k.HashHistory[i]==k.HashVal) {
				sennitite++;
			}
		}
		if (sennitite>=4) {
			// 千日手
			sennitite=0;
			// 連続王手の千日手チェック
			int i;
			for(i=k.Tesu;sennitite<=3&&i>0;i-=2) {
				if (!Kyokumen::OuteHistory[i]) {
					break;
				}
				if (k.HashHistory[i]==k.HashVal) {
					sennitite++;
				}
			}
			if (sennitite==4) {
				// 連続王手の千日手をかけられている
				return INFINITEVAL;
			}
			sennitite=0;
			for(i=k.Tesu;sennitite<=3&&i>1;i-=2) {
				if (!Kyokumen::OuteHistory[i-1]) {
					break;
				}
				if (k.HashHistory[i]==k.HashVal) {
					sennitite++;
				}
			}
			if (sennitite==4) {
				// 連続王手の千日手をかけている
				return -INFINITEVAL;
			}
			return 0;
		}
	}
//ina// 	if (depth==depthMax) {
	if (depth>=depthMax) { //ina//
		int value=k.Evaluate()+k.BestEval(SorE);
		// 自分の手番から見た得点を返す
		if (SorE==SELF) {
			return value;
		} else {
			return -value;
		}
	}
	if (HashTbl[k.HashVal & 0x0fffff].HashVal==k.HashVal) {
		HashEntry e=HashTbl[k.HashVal & 0x0fffff];
		if (e.value>=beta && e.Tesu>=k.Tesu && e.Tesu%2==k.Tesu%2 && e.depth<=depth && e.remainDepth>=depthMax-depth && e.flag!=UPPER_BOUND) {
			return e.value;
		}
		if (e.value<=alpha && e.Tesu>=k.Tesu && e.Tesu%2==k.Tesu%2 && e.depth<=depth && e.remainDepth>=depthMax-depth && e.flag!=LOWER_BOUND) {
			return e.value;
		}
	} else if (depthMax-depth>2 && bITDeep) {
		// 初めて訪れた局面で、深さも残っているので多重反復深化を行う。
		return ITDeep(SorE,k,alpha,beta,depth,depthMax);
	}
	Te teBuf[600];
	int retval=-INFINITEVAL-1;

	try {
		if (depth<1 && k.Mate(SorE,7,teBuf[0])==1) { // 詰みを探索するのは初手だけに変更。
			Best[depth][depth]=teBuf[0];
			Best[depth][depth+1]=Te(0);
			retval=INFINITEVAL+1;
			goto HashAdd;
		}
	} catch (int) {
	}

	int teNum;
	if (depth==0) {
		teNum=MakeMoveFirst(SorE,depth,teBuf,k);
	} else {
		teNum=MakeMoveFirst(SorE,depth,teBuf,k);
	}
	int i;
	k.EvaluateTe(SorE,teNum,teBuf);
	for(i=0;i<teNum;i++) {
		KyokumenKomagumi kk(k);
		if (teBuf[i].IsNull()) {
			continue;
		}
		Stack[depth]=teBuf[i];
		kk.Move(SorE,teBuf[i]);

#if SHOWCURRMOVE
		if (depth==0) {
			printf("info currmove ");
			teBuf[i].Print();
			printf("\n");
		}
#endif
		int v=-NegaAlphaBeta(SorE==SELF?ENEMY:SELF,kk,-beta,-max(alpha,retval),depth+1,depthMax);
		// 指し手の評価がマイナスの手が直前に指されていて、それを取る手がα値を更新しないようなら、
		// 読みを深くして読み直す
		if (depth>1 && Stack[depth-1].value<0 && Stack[depth-1].to==Stack[depth].to && v<=retval) {
			v=-NegaAlphaBeta(SorE==SELF?ENEMY:SELF,kk,-beta,-max(alpha,retval),depth+1,depthMax+2);
		}
		if (v>retval) {
			retval=v;
			Best[depth][depth]=teBuf[i];
			for(int i=depth+1;i<depthMax;i++) {
				Best[depth][i]=Best[depth+1][i];
			}
			if (depth==0) {
#if SHOWINFO
				unsigned long diffTime = timeGetTime() - thinkStartTime;
				printf("info ");
				printf("time %lu ",diffTime);
				printf("depth %d ",depthMax);
				//printf("seldepth %d ", 7); // seldepthの使い方の例。
				printf("nodes %lu ",evaluatedNodes);
				if (retval == INFINITEVAL+1) {
					printf("score mate +");
					//printf("%d ", tesu); // 本当はここで手数を表示する必要がある
					printf(" ");
				} else if (retval == -INFINITEVAL) {
					printf("score mate -");
					//printf("%d ", tesu); // 本当はここで手数を表示する必要がある
					printf(" ");
				} else {
					printf("score cp %d ",retval);
				}
				if (!Best[0][0].IsNull()) {
					printf("pv");
					for(int j=0;j<depthMax;j++) {
						if (!Best[0][j].IsNull()) {
							printf(" ");
							Best[0][j].Print();
						}
					}
				}
				printf("\n");
#endif
			}
			if (retval>=beta) {
				goto HashAdd;
			}
		}
	}
	teNum=k.MakeLegalMoves(SorE,teBuf);
	if (teNum==0) {
		// 負け
		return -INFINITEVAL;
	}
	k.EvaluateTe(SorE,teNum,teBuf);
	for(i=0;i<teNum;i++) {
		if (teBuf[i].value<-100 && i>0 && retval>-INFINITEVAL) {
			break;
		}

		KyokumenKomagumi kk(k);
		Stack[depth]=teBuf[i];
		kk.Move(SorE,teBuf[i]);
		int v=-NegaAlphaBeta(SorE==SELF?ENEMY:SELF,kk,-beta,-max(alpha,retval),depth+1,depthMax);
		// 指し手の評価がマイナスの手が直前に指されていて、それを取る手がα値を更新しないようなら、
		// 読みを深くして読み直す
		if (depth>1 && Stack[depth-1].value<0 && Stack[depth-1].to==Stack[depth].to && v<=beta) {
			v=-NegaAlphaBeta(SorE==SELF?ENEMY:SELF,kk,-beta,-max(alpha,retval),depth+1,depthMax+2);
		}
		if (v>retval) {
			retval=v;
			Best[depth][depth]=teBuf[i];
			for(int i=depth+1;i<depthMax;i++) {
				Best[depth][i]=Best[depth+1][i];
			}
			if (depth==0) {
#if SHOWINFO
				unsigned long diffTime = timeGetTime() - thinkStartTime;
				printf("info ");
				printf("time %lu ",diffTime);
				printf("depth %d ",depthMax);
				//printf("seldepth %d ", 7); // seldepthの使い方の例。
				printf("nodes %lu ",evaluatedNodes);
				//int tesu = 3;
				if (retval == INFINITEVAL+1) {
					printf("score mate +");
					//printf("%d ", tesu); // 本当はここで手数を表示する必要がある
					printf(" ");
				} else if (retval == -INFINITEVAL) {
					printf("score mate -");
					//printf("%d ", tesu); // 本当はここで手数を表示する必要がある
					printf(" ");
				} else {
					printf("score cp %d ",retval);
				}
				if (!Best[0][0].IsNull()) {
					printf("pv");
					for(int j=0;j<depthMax;j++) {
						if (!Best[0][j].IsNull()) {
							printf(" ");
							Best[0][j].Print();
						}
					}
				}
				printf("\n");
#endif
			}
			if (retval>=beta) {
				goto HashAdd;
			}
		}
	}
HashAdd:
	HashEntry e;
	e=HashTbl[k.HashVal & 0x0fffff];
	if (e.HashVal==k.HashVal) {
		e.Second=e.Best;
	} else {
		if (e.Tesu-e.depth==k.Tesu-depth && e.remainDepth>depthMax-depth) {
			// ハッシュにあるデータの方が重要なので上書きしない
			if (depth==0) {
				k.Print();
				Best[depth][depth].Print();
			}
			goto NotAdd;
		}
		e.HashVal=k.HashVal;
		e.Second=Te(0);
	}
	if (retval>alpha) {
		e.Best=Best[depth][depth];
	} else {
		e.Best=Te(0);
	}
	e.value=retval;
	if (retval<=alpha) {
		e.flag=UPPER_BOUND;
	} else if (retval>=beta) {
		e.flag=LOWER_BOUND;
	} else {
		e.flag=EXACTLY_VALUE;
	}
	e.depth=depth;
	e.remainDepth=depthMax-depth;
	e.Tesu=k.Tesu;
	HashTbl[k.HashVal & 0x0fffff]=e;

	++hashCount;

NotAdd:
	return retval;
}

int Sikou::ITDeep(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax)
{
	++itDeepCount;

	int retval = 0;
	try {
		for(int i=depth+1;i<=depthMax;i++) {
			retval=NegaAlphaBeta(SorE,k,alpha,beta,depth,i,false);
		}
	} catch (int) {
		if (itDeepCount > 1) {
			--itDeepCount;
			throw 0;
		}
	}
	--itDeepCount;
	return retval;
}

//Joseki joseki(".\\public.bin"); // うさぴょんの定跡ファイル名
auto_ptr<Joseki> joseki;
Kyokumen *shoki = NULL;


// 本格的に先読みをする思考ルーチンになりました。
Te Sikou::Think(int SorE,KyokumenKomagumi k, bool isUseJoseki, Te* ponderTe)
{
	if(k.Tesu <= InaniwaTimeTesu) {   //ina//
		Te te = InaniwaTime(SorE, k); //ina//
		if(te.IsNull()==0) return te; //ina//
	}                                 //ina//
	int teNum;
	Te teBuf[600];
	int hindo[600];
	int i,j;
	Te nullTe(0);
	*ponderTe = nullTe;
	if (isUseJoseki) { // 任意局面から始めた場合は定跡を使わないことにする。
		joseki->fromJoseki(*shoki,SELF,k,k.Tesu,teNum,teBuf,hindo);
		if (teNum>0) {
#if 0
			int max,maxhindo;
			// 一番頻度の高い定跡を選ぶ。
			max=0;
			maxhindo=hindo[max];
			for(i=1;i<teNum;i++) {
				if (hindo[i]>maxhindo) {
					maxhindo=hindo[i];
					max=i;
				}
			}
			return teBuf[max];
#else
			// 定跡をランダムに選びたいならこちらにする
			int sumHind = 0;
			for (i = 0; i < teNum; i++) {
				sumHind += hindo[i];
			}
			int mod = (rand() * 0x1000 + rand()) % sumHind;
			sumHind = 0;
			for (i = 0; i < teNum; i++) {
				int prev = sumHind;
				sumHind += hindo[i];
				if (prev <= mod && mod < sumHind) {
					return teBuf[i];
				}
			}
			return teBuf[0]; // ここには来ないはずだが念のため。
#endif
		}
	}

	// infoで返す情報の初期化。
	thinkStartTime = timeGetTime();
	evaluatedNodes = 0;

	// depthMaxは適当に残り時間に合わせて調整するなどの工夫が必要です。
	//int depthMax=8; // 4から8に変更。
	int depthMax = thinkDepthMax; // 通常は4で、go infiniteなら8
	for(i=0;i<MAX_DEPTH;i++) {
		for(j=0;j<MAX_DEPTH;j++) {
			Best[i][j]=Te(0);
		}
	}
	itDeepCount = 0;
	int bestVal=ITDeep(SorE,k,-INFINITEVAL+1,INFINITEVAL-1,0,depthMax);
//	int bestVal=NegaAlphaBeta(SorE,k,-INFINITE+1,INFINITE-1,0,depthMax);
#if SHOWINFO
	unsigned long diffTime = timeGetTime() - thinkStartTime;
	printf("info ");
	printf("time %lu ",diffTime);
	printf("nodes %lu ",evaluatedNodes);
	//int tesu = 3;
	if (bestVal == INFINITEVAL+1) {
		printf("score mate +");
		//printf("%d ", tesu); // 本当はここで手数を表示する必要がある
		printf(" ");
	} else if (bestVal == -INFINITEVAL) {
		printf("score mate -");
		//printf("%d ", tesu); // 本当はここで手数を表示する必要がある
		printf(" ");
	} else {
		printf("score cp %d ",bestVal);
	}
	if (!Best[0][0].IsNull()) {
		printf("pv");
		for(i=0;i<depthMax;i++) {
			if (!Best[0][i].IsNull()) {
				printf(" ");
				Best[0][i].Print();
			}
		}
	}
	printf("\n");
#endif
	if (canPonder && ponderTe != NULL) {
		*ponderTe = Best[0][1];
	}
	return Best[0][0];
}
