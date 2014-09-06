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

extern bool isStopReceived; // stop�R�}���h����M������
extern bool canPonder; // ��ǂ݉\��
extern bool canThrow; // �v�l���f���\���ǂ���
extern unsigned long thinkStartTime; // �v�l�J�n����
extern unsigned long evaluatedNodes; // KyokumenKomagumi::Evaluate()���Ă΂ꂽ��
extern unsigned long hashCount; // �n�b�V���ɒǉ����ꂽ��
extern int thinkDepthMax; // �ǂ݂̍ő�[���B�ʏ��4�����Ago infinite�Ŏv�l����ꍇ����8�ɂ���B
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
		// ��Ԃ��Ⴄ�B
		return 0;
	}

	// �ǖʂ���v�����Ǝv����
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

Te Stack[32];	// 32�ƌ��������͓K���B�ő�[�������̒��x�܂ł����s���Ȃ����Ƃ����҂��Ă���B

// ��S�͂Œǉ��B�����@�ɂ��T���B
// ����́A���̃A���S���Y������{�ɐi�߂Ă����B
int Sikou::NegaAlphaBeta(int SorE,KyokumenKomagumi &k,int alpha,int beta,int depth,int depthMax,bool bITDeep)
{
	canThrow = depth > 1; // depth��1���ƌ��݂̍őP�肪�m�肵�Ă��Ȃ��̂ŁAthrow�\�Ȃ̂�depth��2�ȏ�ɂȂ��Ă���B
	if (isStopReceived) {
		if (canThrow) {
			throw 0; // �߂���Sikou::ITDeep()
		}
	}
	if (depth==1) {
		// �����`�F�b�N
		int sennitite=0;
		for(int i=k.Tesu;i>0;i-=2) {
			if (k.HashHistory[i]==k.HashVal) {
				sennitite++;
			}
		}
		if (sennitite>=4) {
			// �����
			sennitite=0;
			// �A������̐����`�F�b�N
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
				// �A������̐������������Ă���
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
				// �A������̐����������Ă���
				return -INFINITEVAL;
			}
			return 0;
		}
	}
//ina// 	if (depth==depthMax) {
	if (depth>=depthMax) { //ina//
		int value=k.Evaluate()+k.BestEval(SorE);
		// �����̎�Ԃ��猩�����_��Ԃ�
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
		// ���߂ĖK�ꂽ�ǖʂŁA�[�����c���Ă���̂ő��d�����[�����s���B
		return ITDeep(SorE,k,alpha,beta,depth,depthMax);
	}
	Te teBuf[600];
	int retval=-INFINITEVAL-1;

	try {
		if (depth<1 && k.Mate(SorE,7,teBuf[0])==1) { // �l�݂�T������̂͏��肾���ɕύX�B
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
		// �w����̕]�����}�C�i�X�̎肪���O�Ɏw����Ă��āA��������肪���l���X�V���Ȃ��悤�Ȃ�A
		// �ǂ݂�[�����ēǂݒ���
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
				//printf("seldepth %d ", 7); // seldepth�̎g�����̗�B
				printf("nodes %lu ",evaluatedNodes);
				if (retval == INFINITEVAL+1) {
					printf("score mate +");
					//printf("%d ", tesu); // �{���͂����Ŏ萔��\������K�v������
					printf(" ");
				} else if (retval == -INFINITEVAL) {
					printf("score mate -");
					//printf("%d ", tesu); // �{���͂����Ŏ萔��\������K�v������
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
		// ����
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
		// �w����̕]�����}�C�i�X�̎肪���O�Ɏw����Ă��āA��������肪���l���X�V���Ȃ��悤�Ȃ�A
		// �ǂ݂�[�����ēǂݒ���
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
				//printf("seldepth %d ", 7); // seldepth�̎g�����̗�B
				printf("nodes %lu ",evaluatedNodes);
				//int tesu = 3;
				if (retval == INFINITEVAL+1) {
					printf("score mate +");
					//printf("%d ", tesu); // �{���͂����Ŏ萔��\������K�v������
					printf(" ");
				} else if (retval == -INFINITEVAL) {
					printf("score mate -");
					//printf("%d ", tesu); // �{���͂����Ŏ萔��\������K�v������
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
			// �n�b�V���ɂ���f�[�^�̕����d�v�Ȃ̂ŏ㏑�����Ȃ�
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

//Joseki joseki(".\\public.bin"); // �����҂��̒�Ճt�@�C����
auto_ptr<Joseki> joseki;
Kyokumen *shoki = NULL;


// �{�i�I�ɐ�ǂ݂�����v�l���[�`���ɂȂ�܂����B
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
	if (isUseJoseki) { // �C�Ӌǖʂ���n�߂��ꍇ�͒�Ղ��g��Ȃ����Ƃɂ���B
		joseki->fromJoseki(*shoki,SELF,k,k.Tesu,teNum,teBuf,hindo);
		if (teNum>0) {
#if 0
			int max,maxhindo;
			// ��ԕp�x�̍�����Ղ�I�ԁB
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
			// ��Ղ������_���ɑI�т����Ȃ炱����ɂ���
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
			return teBuf[0]; // �����ɂ͗��Ȃ��͂������O�̂��߁B
#endif
		}
	}

	// info�ŕԂ����̏������B
	thinkStartTime = timeGetTime();
	evaluatedNodes = 0;

	// depthMax�͓K���Ɏc�莞�Ԃɍ��킹�Ē�������Ȃǂ̍H�v���K�v�ł��B
	//int depthMax=8; // 4����8�ɕύX�B
	int depthMax = thinkDepthMax; // �ʏ��4�ŁAgo infinite�Ȃ�8
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
		//printf("%d ", tesu); // �{���͂����Ŏ萔��\������K�v������
		printf(" ");
	} else if (bestVal == -INFINITEVAL) {
		printf("score mate -");
		//printf("%d ", tesu); // �{���͂����Ŏ萔��\������K�v������
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
