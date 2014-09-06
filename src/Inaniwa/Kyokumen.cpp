#ifdef _WIN32
#include <windows.h>
#else
#include "WinAPI.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <algorithm>

#include "Kyokumen.h"

using namespace std;

uint64 Kyokumen::HashSeed[ERY+1][0x99+1];
uint64 Kyokumen::HandHashSeed[EHI+1][18+1];

//ina// uint64 Kyokumen::HashHistory[1000];
//ina// int Kyokumen::OuteHistory[1000];
uint64 Kyokumen::HashHistory[INANIWA_MAX_TESU]; //ina//
int Kyokumen::OuteHistory[INANIWA_MAX_TESU];    //ina//

void Kyokumen::HashInit()
{
	int i;
	for(i=0;i<=ERY;i++) {
		for(int pos=0x11;pos<=0x99;pos++) {
			HashSeed[i][pos]=(((uint64)rand())<<49)|
							 (((uint64)rand())<<34)|
							 (((uint64)rand())<<19)|
							 (((uint64)rand())<<4)|
							  (rand() & 0x07);
		}
	}
	
	for(i=SFU;i<=EHI;i++) {
		//for(int maisuu=0;maisuu<=0x99;maisuu++) {
		// http://lesserpyon.bbs.coocan.jp/?m=listthread&t_id=3により修正
		for(int maisuu=0;maisuu<=18;maisuu++) {
			HandHashSeed[i][maisuu]=(((uint64)rand())<<49)|
								 (((uint64)rand())<<34)|
								 (((uint64)rand())<<19)|
								 (((uint64)rand())<<4)|
								  (rand() & 0x07);
		}
	}
}



// 局面のコンストラクタ：盤面の状態と手数、持ち駒から生成
Kyokumen::Kyokumen(int tesu,KomaInf board[9][9],int Motigoma[])
{
	// 盤面をWALL（壁）で埋めておきます。
	memset(banpadding,WALL,sizeof(banpadding));
	memset(ban,WALL,sizeof(ban));
	// 初期化
	value=0;
	kingS=0;
	kingE=0;
	Tesu=tesu;

	HashVal=0;
	HandHashVal=0;
	KyokumenHashVal=0;
	// boardで与えられた局面を設定します。
	for(int dan=1;dan<=9;dan++) {
		for(int suji=0x10;suji<=0x90;suji+=0x10) {
			// 将棋の筋は左から右なので、配列の宣言と逆になるため、筋はひっくり返さないとなりません。
			ban[suji+dan]=board[dan-1][9-suji/0x10];
			KyokumenHashVal^=HashSeed[ban[suji+dan]][suji+dan];
			if (ban[suji+dan]==SOU) {
				kingS=suji+dan;
			}
			if (ban[suji+dan]==EOU) {
				kingE=suji+dan;
			}
			value+=KomaValue[ban[suji+dan]];
		}
	}
	// 持ち駒はそのまま利用します。
	int i;
	for(i=0;i<=EHI;i++) {
		Hand[i]=Motigoma[i];
		value+=HandValue[i]*Hand[i];
		for(int j=1;j<=Hand[i];j++) {
			HandHashVal^=HandHashSeed[i][j];
		}
	}
	HashVal=KyokumenHashVal^HandHashVal;
	for(i=0;i<Tesu;i++) {
		HashHistory[i]=0;
		OuteHistory[i]=0;
	}
	HashHistory[Tesu]=HashVal;
	OuteHistory[Tesu]=((Tesu%2)==0)?controlS[kingE]:controlE[kingS];
	// controlS/controlEを初期化します。
	InitControl();
}

// 局面の初期化。コンストラクタと同じことをやらせる。（コンストラクタ以外からでも初期化できるように。）
void Kyokumen::InitKyokumen(int tesu, KomaInf board[9][9], int Motigoma[])
{
	// 盤面をWALL（壁）で埋めておきます。
	memset(banpadding,WALL,sizeof(banpadding));
	memset(ban,WALL,sizeof(ban));
	// 初期化
	value=0;
	kingS=0;
	kingE=0;
	Tesu=tesu;

	HashVal=0;
	HandHashVal=0;
	KyokumenHashVal=0;
	// boardで与えられた局面を設定します。
	for(int dan=1;dan<=9;dan++) {
		for(int suji=0x10;suji<=0x90;suji+=0x10) {
			// 将棋の筋は左から右なので、配列の宣言と逆になるため、筋はひっくり返さないとなりません。
			ban[suji+dan]=board[dan-1][9-suji/0x10];
			KyokumenHashVal^=HashSeed[ban[suji+dan]][suji+dan];
			if (ban[suji+dan]==SOU) {
				kingS=suji+dan;
			}
			if (ban[suji+dan]==EOU) {
				kingE=suji+dan;
			}
			value+=KomaValue[ban[suji+dan]];
		}
	}
	// 持ち駒はそのまま利用します。
	int i;
	for(i=0;i<=EHI;i++) {
		Hand[i]=Motigoma[i];
		value+=HandValue[i]*Hand[i];
		for(int j=1;j<=Hand[i];j++) {
			HandHashVal^=HandHashSeed[i][j];
		}
	}
	HashVal=KyokumenHashVal^HandHashVal;
	for(i=0;i<Tesu;i++) {
		HashHistory[i]=0;
		OuteHistory[i]=0;
	}
	HashHistory[Tesu]=HashVal;
	OuteHistory[Tesu]=((Tesu%2)==0)?controlS[kingE]:controlE[kingS];
	// controlS/controlEを初期化します。
	InitControl();
}

// controlS,controlEの初期化
void Kyokumen::InitControl()
{
	int dan, suji;
	int i, j, b, bj;

	memset(controlS,0,sizeof(controlS));
	memset(controlE,0,sizeof(controlE));

	for (suji = 0x10; suji <= 0x90; suji += 0x10) {
		for (dan = 1 ; dan <= 9 ; dan++) {
			if (ban[suji + dan] & ENEMY) { //敵の駒
				//駒の効きを追加する
				for (i = 0, b = 1, bj = (1<<16); i < 12; i++, b<<=1, bj<<=1) {
					if (CanJump[i][ban[dan + suji]]) {
						j = dan + suji;
						do {
							j += Direct[i];
							controlE[j] |= bj;
						} while (ban[j] == EMPTY);
					} else if (CanMove[i][ban[dan + suji]]) {
						controlE[dan + suji + Direct[i]] |= b;
					}
				}
			} else if (ban[suji + dan] & SELF) { //味方の駒が有る
				//駒の効きを追加する
				for (i = 0, b = 1, bj = (1<<16); i < 12; i++, b<<=1, bj<<=1) {
					if (CanJump[i][ban[dan + suji]]) {
						j = dan + suji;
						do {
							j += Direct[i];
							controlS[j] |= bj;
						} while (ban[j] == EMPTY);
					} else if (CanMove[i][ban[dan + suji]]) {
						controlS[dan + suji + Direct[i]] |= b;
					}
				}
			}	
		}
	}
}

// 手で局面を進める
void Kyokumen::Move(int SorE,const Te &te)
{
	int i,j,b,bj;
	if (te.from>0x10) {
		// 元いた駒のコントロールを消す
		int dir;
		for(dir=0,b=1,bj=1<<16;dir<12;dir++,b<<=1,bj<<=1) {
			if (SorE==SELF) {
				controlS[te.from+Direct[dir]]&=~b;
			} else {
				controlE[te.from+Direct[dir]]&=~b;
			}
			if (CanJump[dir][te.koma]) {
				int j = te.from;
				do {
					j += Direct[dir];
					if (SorE==SELF) {
						controlS[j] &= ~bj;
					} else {
						controlE[j] &= ~bj;
					}
				} while (ban[j] == EMPTY);
			}
		}
		// 元いた位置は空白になる
		ban[te.from]=EMPTY;
		// 空白になったことで変わるハッシュ値
		KyokumenHashVal^=HashSeed[te.koma][te.from];
		KyokumenHashVal^=HashSeed[EMPTY][te.from];
		// 飛び利きを伸ばす
		for (i = 0, bj = (1<<16); i < 8; i++, bj<<=1) {
			int Dir=Direct[i];
			if (controlS[te.from] & bj) {
				j = te.from;
				do {
					j += Dir;
					controlS[j] |= bj;
				} while (ban[j] == EMPTY);
			}
			if (controlE[te.from] & bj) {
				j = te.from;
				do {
					j += Dir;
					controlE[j] |= bj;
				} while (ban[j] == EMPTY);
			}
		}
	} else {
		// 持ち駒から一枚減らす
		HandHashVal^=HandHashSeed[te.koma][Hand[te.koma]];
		Hand[te.koma]--;
		value-=HandValue[te.koma];
		value+=KomaValue[te.koma];
	}
	if (ban[te.to]!=EMPTY) {
		// 相手の駒を持ち駒にする。
		// 持ち駒にする時は、成っている駒も不成りに戻す。（&~PROMOTED）
		value-=KomaValue[ban[te.to]];
		value+=HandValue[SorE|(ban[te.to]&~PROMOTED&~SELF&~ENEMY)];
		int koma=SorE|(ban[te.to]&~PROMOTED&~SELF&~ENEMY);
		Hand[koma]++;
		// ハッシュに取った駒を加える
		HandHashVal^=HandHashSeed[koma][Hand[koma]];
		//取った駒の効きを消す
		for (i = 0, b = 1, bj = (1<<16); i < 12; i++, b<<=1, bj<<=1) {
			int Dir=Direct[i];
			if (CanJump[i][ban[te.to]]) {
				j = te.to;
				do {
					j += Dir;
					if (SorE==SELF) {
						controlE[j] &= ~bj;
					} else {
						controlS[j] &= ~bj;
					}
				} while (ban[j] == EMPTY);
			} else {
				j=te.to + Dir;
				if (SorE==SELF) {
					controlE[j] &= ~b;
				} else {
					controlS[j] &= ~b;
				}
			}
		}
	} else {
		// 移動先で遮った飛び利きを消す
		for (i = 0, bj = (1<<16); i < 8; i++, bj<<=1) {
			int Dir=Direct[i];
			if (controlS[te.to] & bj) {
				j = te.to;
				do {
					j += Dir;
					controlS[j] &= ~bj;
				} while (ban[j] == EMPTY);
			}
			if (controlE[te.to] & bj) {
				j = te.to;
				do {
					j += Dir;
					controlE[j] &= ~bj;
				} while (ban[j] == EMPTY);
			}
		}
	}
	// ban[te.to]にあったものをＨａｓｈから消す
	KyokumenHashVal^=HashSeed[ban[te.to]][te.to];
	if (te.promote) {
		value-=KomaValue[te.koma];
		value+=KomaValue[te.koma|PROMOTED];
		ban[te.to]=te.koma|PROMOTED;
	} else {
		ban[te.to]=te.koma;
	}
	// 新しい駒をＨａｓｈに加える
	KyokumenHashVal^=HashSeed[ban[te.to]][te.to];
	// 移動先の利きをつける
	for (i = 0, b = 1, bj = (1<<16); i < 12; i++, b<<=1, bj<<=1) {
		if (CanJump[i][ban[te.to]]) {
			j = te.to;
			do {
				j += Direct[i];
				if (SorE==SELF) {
					controlS[j] |= bj;
				} else {
					controlE[j] |= bj;
				}
			} while (ban[j] == EMPTY);
		} else if (CanMove[i][ban[te.to]]) {
			if (SorE==SELF) {
				controlS[te.to+Direct[i]] |= b;
			} else {
				controlE[te.to+Direct[i]] |= b;
			}
		}
	}
	// 王様の位置は覚えておく。
	if (te.koma==SOU) {
		kingS=te.to;
	}
	if (te.koma==EOU) {
		kingE=te.to;
	}

	HashVal=KyokumenHashVal^HandHashVal;
	Tesu++;
	HashHistory[Tesu]=HashVal;
	OuteHistory[Tesu]=(SorE==SELF)?controlS[kingE]:controlE[kingS];
}

// ピン（動かすと王を取られてしまうので動きが制限される駒）の状態を設定する
void Kyokumen::MakePinInf(int *pin) const
{
	int i;
	// ピン情報を設定する
	for (i = 0x11; i <= 0x99; i++) {
		// 0はピンされていない、という意味
		pin[i] = 0;
	}
	if (kingS) {	//自玉が盤面にある時のみ有効
		for (i = 0; i < 8; i++) {
			int p;
			p = search(kingS, -Direct[i]); 
			if ((ban[p] != WALL) && !(ban[p] & ENEMY)) { //味方の駒が有る
				if (controlE[p]&(1<<(16+i))) {
					pin[p]=Direct[i];
				}
			}
		}
	}
	if (kingE) {	//敵玉が盤面にある時のみ有効
		for (i = 0; i < 8; i++) {
			int p;
			p = search(kingE, -Direct[i]);
			if ((ban[p] != WALL) && (ban[p] & ENEMY)) { //敵の駒が有る
				if (controlS[p]&(1<<(16+i))) {
					pin[p]=Direct[i];
				}
			}
		}
	}
}

// 駒の動きとして正しい動きを全て生成する。
int Kyokumen::MakeLegalMoves(int SorE,Te *teBuf,int *pin)
{
	int pbuf[16*11];
	int teNum=0;
	if (pin==NULL) {
		MakePinInf(pbuf);
		pin=pbuf;
	}
	//if (SorE==SELF && controlE[kingS]!=0) {
	//	return AntiCheck(SorE,teBuf,pin,controlE[kingS]);
	//}
	//if (SorE==ENEMY && controlS[kingE]!=0) {
	//	return AntiCheck(SorE,teBuf,pin,controlS[kingE]);
	//}
	if (SorE==SELF && kingS != 0 && controlE[kingS]!=0) {
		// 先手玉があるかどうかという条件を追加。（詰将棋などで先手玉がないなら入らない）
		return AntiCheck(SorE,teBuf,pin,controlE[kingS]);
	}
	if (SorE==ENEMY && kingE != 0 && controlS[kingE]!=0) {
		// 後手玉があるかどうかという条件を追加。（詰将棋などで後手玉がないなら入らない）
		return AntiCheck(SorE,teBuf,pin,controlS[kingE]);
	}

	int suji,dan;
	int StartDan,EndDan;
	// 盤上の駒を動かす
	for(suji=0x10;suji<=0x90;suji+=0x10) {
		for(dan=1;dan<=9;dan++) {
			if (ban[suji+dan]&SorE) {
				AddMoves(SorE,teNum,teBuf,suji+dan,pin[suji+dan]);
			}
		}
	}
	// 歩を打つ
	if (Hand[SorE|FU]>0) {
		for(suji=0x10;suji<=0x90;suji+=0x10) {
			// 二歩チェック
			int nifu=0;
			for(dan=1;dan<=9;dan++) {
				if (ban[suji+dan]==(SorE|FU)) {
					nifu=true;
					break;
				}
			}
			if (nifu) continue;
			//(先手なら２段目より下に、後手なら８段目より上に打つ）
			if (SorE==SELF) {
				StartDan=2;
				EndDan=9;
			} else {
				StartDan=1;
				EndDan=8;
			}
			for(dan=StartDan;dan<=EndDan;dan++) {
				// 打ち歩詰めもチェック
				if (ban[dan+suji]==EMPTY && !Utifudume(SorE,dan+suji,pin)) {
					teBuf[teNum++]=Te(0,suji+dan,SorE|FU,EMPTY);
				}
			}
		}
	}
	// 香を打つ
	if (Hand[SorE|KY]>0) {
		for(suji=0x10;suji<=0x90;suji+=0x10) {
			//(先手なら２段目より下に、後手なら８段目より上に打つ）
			if (SorE==SELF) {
				StartDan=2;
				EndDan=9;
			} else {
				StartDan=1;
				EndDan=8;
			}
			for(dan=StartDan;dan<=EndDan;dan++) {
				if (ban[dan+suji]==EMPTY) {
					teBuf[teNum++]=Te(0,suji+dan,SorE|KY,EMPTY);
				}
			}
		}
	}
	//桂を打つ
	if (Hand[SorE|KE]>0) {
		//(先手なら３段目より下に、後手なら７段目より上に打つ）
		for(suji=0x10;suji<=0x90;suji+=0x10) {
			if (SorE==SELF) {
				StartDan=3;
				EndDan=9;
			} else {
				StartDan=1;
				EndDan=7;
			}
			for(dan=StartDan;dan<=EndDan;dan++) {
				if (ban[dan+suji]==EMPTY) {
					teBuf[teNum++]=Te(0,suji+dan,SorE|KE,EMPTY);
				}
			}
		}
	}
	// 銀〜飛車は、どこにでも打てる
	for(int koma=GI;koma<=HI;koma++) {
		if (Hand[SorE|koma]>0) {
			for(suji=0x10;suji<=0x90;suji+=0x10) {
				for(dan=1;dan<=9;dan++) {
					if (ban[dan+suji]==EMPTY) {
						teBuf[teNum++]=Te(0,suji+dan,SorE|koma,EMPTY);
					}
				}
			}
		}
	}
	
	return teNum;
}

// 盤面のfromにある駒を動かす手を生成する。
void Kyokumen::AddMoves(int SorE,int &teNum,Te *teTop,int from,int pin,int Rpin)
{
	switch(ban[from]) {
	case SFU:
		AddMove(SorE,teNum,teTop,from,-1,pin,Rpin);
		break;
	case EFU:
		AddMove(SorE,teNum,teTop,from,+1,pin,Rpin);
		break;
	case SKY:
		AddStraight(SorE,teNum,teTop,from,-1,pin,Rpin);
		break;
	case EKY:
		AddStraight(SorE,teNum,teTop,from,+1,pin,Rpin);
		break;
	case SKE:
		AddMove(SorE,teNum,teTop,from,+14,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-18,pin,Rpin);
		break;
	case EKE:
		AddMove(SorE,teNum,teTop,from,-14,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,+18,pin,Rpin);
		break;
	case SGI:
		AddMove(SorE,teNum,teTop,from, -1,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-17,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, 15,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, 17,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-15,pin,Rpin);
		break;
	case EGI:
		AddMove(SorE,teNum,teTop,from, +1,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, 17,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-15,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-17,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, 15,pin,Rpin);
		break;
	case SKI:case STO:case SNY:case SNK:case SNG:
		AddMove(SorE,teNum,teTop,from, -1,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-17,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, 15,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, +1,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-16,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, 16,pin,Rpin);
		break;
	case EKI:case ETO:case ENY:case ENK:case ENG:
		AddMove(SorE,teNum,teTop,from, +1,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, 17,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-15,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, -1,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-16,pin,Rpin);
		AddMove(SorE,teNum,teTop,from, 16,pin,Rpin);
		break;
	case SRY:case ERY:
		AddMove(SorE,teNum,teTop,from, 17,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-15,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-17,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,+15,pin,Rpin);
	case SHI:case EHI:
		AddStraight(SorE,teNum,teTop,from,+1,pin,Rpin);
		AddStraight(SorE,teNum,teTop,from,-1,pin,Rpin);
		AddStraight(SorE,teNum,teTop,from,-16,pin,Rpin);
		AddStraight(SorE,teNum,teTop,from,+16,pin,Rpin);
		break;
	case SUM:case EUM:
		AddMove(SorE,teNum,teTop,from,+1,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,+16,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-16,pin,Rpin);
		AddMove(SorE,teNum,teTop,from,-1,pin,Rpin);
	case SKA:case EKA:
		AddStraight(SorE,teNum,teTop,from,+17,pin,Rpin);
		AddStraight(SorE,teNum,teTop,from,-17,pin,Rpin);
		AddStraight(SorE,teNum,teTop,from,+15,pin,Rpin);
		AddStraight(SorE,teNum,teTop,from,-15,pin,Rpin);
		break;
	case SOU:case EOU:
		MoveKing(SorE,teNum,teTop,0);	// 王手がかかっている時には、AntiCheckの方が呼ばれるから、Kikiは０です。
	}
}

// ある場所の利き情報を作成して返す。普段は使わない関数（差分計算しているから）だが、
// 打ち歩詰めのチェックなど、駒を仮に置いてみて何かするようなときに使用する。
Kiki Kyokumen::CountControlS(int pos)
{
	Kiki ret=0;
	int i,b,bj;
	b=1;
	bj=1<<16;
	for(i=0;i<12;i++,b<<=1,bj<<=1) {
		if (CanMove[i][ban[pos-Direct[i]]] && (ban[pos-Direct[i]]&SELF)) {
			ret|=b;
		} else if (CanJump[i][ban[search(pos,-Direct[i])]] && (ban[search(pos,-Direct[i])]&SELF)) {
			ret|=bj;
		}
	}
	return ret;
}

// ある場所の利き情報を作成して返す。普段は使わない関数（差分計算しているから）だが、
// 打ち歩詰めのチェックなど、駒を仮に置いてみて何かするようなときに使用する。
Kiki Kyokumen::CountControlE(int pos)
{
	Kiki ret=0;
	int i,b,bj;
	b=1;
	bj=1<<16;
	for(i=0;i<12;i++,b<<=1,bj<<=1) {
		if (CanMove[i][ban[pos-Direct[i]]] && (ban[pos-Direct[i]]&ENEMY)) {
			ret|=b;
		} else if (CanJump[i][ban[search(pos,-Direct[i])]] && (ban[search(pos,-Direct[i])]&ENEMY)) {
			ret|=bj;
		}
	}
	return ret;
}

// ある場所に移動できる駒を全部集めて、Kiki情報にして返す。
// このとき、pinされている駒はpinの方向にしか動けない。
Kiki Kyokumen::CountMove(int SorE,int pos,int *pin)
{
	Kiki ret=0;
	int i,b,bj;
	b=1;
	bj=1<<16;
	for(i=0;i<12;i++,b<<=1,bj<<=1) {
		if (CanMove[i][ban[pos-Direct[i]]] && (ban[pos-Direct[i]]&SorE) && (pin[pos-Direct[i]]==0||pin[pos-Direct[i]]==Direct[i]||pin[pos-Direct[i]]==-Direct[i])) {
			ret|=b;
		} else if (CanJump[i][ban[search(pos,-Direct[i])]] && (ban[search(pos,-Direct[i])]&SorE)&& (pin[search(pos,-Direct[i])]==0||pin[search(pos,-Direct[i])]==Direct[i]||pin[search(pos,-Direct[i])]==-Direct[i])) {
			ret|=bj;
		}
	}
	return ret;
}

// 打ち歩詰めの判定
int Kyokumen::Utifudume(int SorE,int to,int *pin)
{
	if (SorE==SELF) {
		// まず、玉の頭に歩を打つ手じゃなければ打ち歩詰めの心配はない。
		if (kingE+1!=to) {
			return 0;
		}
	} else {
		// まず、玉の頭に歩を打つ手じゃなければ打ち歩詰めの心配はない。
		if (kingS-1!=to) {
			return 0;
		}
	}
	//実際に歩を打って確かめてみる。
	ban[to]=FU|SorE;
	if (SorE==SELF) {
		// 自分の利きがあったら相手は玉で取れない　＆　取る動きを列挙してみたら玉で取る手しかない
		if (controlS[to] && (CountMove(ENEMY,to,pin)==1<<1)) {
			// 玉に逃げ道があるかどうかをチェック
			for(int i=0;i<8;i++) {
				KomaInf koma=ban[kingE+Direct[i]];
				if (!(koma & ENEMY) && !CountControlS(kingE+Direct[i])) {
					// 逃げ道があったので、盤面を元の状態に戻して、
					ban[to]=EMPTY;
					// 打ち歩詰めではなかった。
					return 0;
				}
			}
			// 玉の逃げ道もないのなら、打ち歩詰め。盤面の状態は元に戻す。
			ban[to]=EMPTY;
			return 1;
		}
		// 玉以外で取れる手があるか、玉で取れる。
		ban[to]=EMPTY;
		return 0;
	} else {
		// 自分の利きがあったら相手は玉で取れない　＆　取る動きを列挙してみたら玉で取る手しかない
		if (controlE[to] && (CountMove(SELF,to,pin)==1<<6)) {
			// 玉に逃げ道があるかどうかをチェック
			for(int i=0;i<8;i++) {
				KomaInf koma=ban[kingS+Direct[i]];
				if (!(koma & SELF) && !CountControlE(kingS+Direct[i])) {
					// 逃げ道があったので、盤面を元の状態に戻して、
					ban[to]=EMPTY;
					// 打ち歩詰めではなかった。
					return 0;
				}
			}
			// 玉の逃げ道もないのなら、打ち歩詰め。盤面の状態は元に戻す。
			ban[to]=EMPTY;
			return 1;
		}
		// 玉以外で取れる手があるか、玉で取れる。
		ban[to]=EMPTY;
		return 0;
	}
}

// ある場所（to）に駒を打つ手の生成
void Kyokumen::PutTo(int SorE,int &teNum,Te *teTop,int to,int *pin)
{
	int dan=to &0x0f;
	if (SorE==ENEMY) {
		dan=10-dan;
	}
	if (Hand[SorE|FU]>0 && dan>1) {
		// 歩を打つ手を生成
		// 二歩チェック
		int suji=to & 0xf0;
		int nifu=0;
		for(int d=1;d<=9;d++) {
			if (ban[suji+d]==(SorE|FU)) {
				nifu=1;
				break;
			}
		}
		// 打ち歩詰めもチェック
		if (!nifu && !Utifudume(SorE,to,pin)) {
			teTop[teNum++]=Te(0,to,SorE|FU,EMPTY);
		}
	}
	if (Hand[SorE|KY]>0 && dan>1) {
		// 香を打つ手を生成
		teTop[teNum++]=Te(0,to,SorE|KY,EMPTY);
	}
	if (Hand[SorE|KE]>0 && dan>2) {
		teTop[teNum++]=Te(0,to,SorE|KE,EMPTY);
	}
	for(int koma=GI;koma<=HI;koma++) {
		if (Hand[SorE|koma]>0) {
			teTop[teNum++]=Te(0,to,SorE|koma,EMPTY);
		}
	}
}

// 王手を受ける手の生成
int Kyokumen::AntiCheck(int SorE,Te *teBuf,int *pin,Kiki kiki)
{
	int king;
	int teNum=0;
	if ((kiki & (kiki-1))!=0) {
        //両王手は玉を動かすしかない
        MoveKing(SorE,teNum, teBuf, kiki);
	} else {
		if (SorE==SELF) {
			king=kingS;
		} else {
			king=kingE;
		}
		unsigned int id;
		int check;
        for (id = 0; id <= 31; id++) {
            if (kiki == (1u << id)) break;
        }
		if (id < 16) {
            check = king - Direct[id];
		} else {
			check = search(king,-Direct[id-16]);
		}
        //王手駒を取る
        MoveTo(SorE,teNum, teBuf, check, pin);
		
        //玉を動かす
        MoveKing(SorE,teNum, teBuf, kiki);

		if (id >= 16) {
            //合駒をする手を生成する
            int i;
            for (i = king - Direct[id-16]; ban[i] == EMPTY; i -= Direct[id-16]) {
				MoveTo(SorE,teNum, teBuf, i, pin); //移動合
            }
            for (i = king - Direct[id-16]; ban[i] == EMPTY; i -= Direct[id-16]) {
				PutTo(SorE,teNum, teBuf, i, pin);  //駒を打つ合
            }
        } 
	}
	return teNum;
}

// 玉を動かす手の生成
// 普通の駒と違い、相手の利きのあるところには動けないので、そのための特殊な処理をしています。
void Kyokumen::MoveKing(int SorE,int &teNum,Te *teTop,Kiki kiki)
{
	int i;
	int id = -1;	//隣接王手駒の位置のid
	// 両王手でないなら王手駒の位置を探す
	for (i = 0; i < 8; i++) {
		if (kiki & (1 << i)) {
			id = i;
			break;
		}
	}
	if (id >= 0) {
		// 隣接の王手 最初に取る手を生成するのだ
		if (SorE==SELF) {
			KomaInf koma=ban[kingS-Direct[id]];
			if (( koma==EMPTY || (koma & ENEMY))
				&& !CountControlE(kingS - Direct[id]) //敵の駒が効いていない
				&& !(kiki & (1 << (23-id))))  //敵の飛駒で貫かれていない
			AddMove(SorE,teNum, teTop, kingS, -Direct[id], 0);
		} else {
			KomaInf koma=ban[kingE-Direct[id]];
			if (( koma==EMPTY || (koma & SELF))
				&& !CountControlS(kingE - Direct[id]) //敵の駒が効いていない
				&& !(kiki & (1 << (23-id))))  //敵の飛駒で貫かれていない
			AddMove(SorE,teNum, teTop, kingE, -Direct[id], 0);
		}
	}
	for (i = 0; i < 8; i++) {
		if (i == id) continue;
		if (SorE==SELF) {
			KomaInf koma=ban[kingS-Direct[i]];
			if (( koma==EMPTY || (koma & ENEMY))
				&& !CountControlE(kingS - Direct[i]) //敵の駒が効いていない
				&& !(kiki & (1 << (23-i))))  //敵の飛駒で貫かれていない
			AddMove(SorE,teNum, teTop, kingS, -Direct[i], 0);
		} else {
			KomaInf koma=ban[kingE-Direct[i]];
			if (( koma==EMPTY || (koma & SELF))
				&& !CountControlS(kingE - Direct[i]) //敵の駒が効いていない
				&& !(kiki & (1 << (23-i))))  //敵の飛駒で貫かれていない
			AddMove(SorE,teNum, teTop, kingE, -Direct[i], 0);
		}
	}
}

// 手の生成：成り・不成りも意識して、駒の動く手を生成する。
void Kyokumen::AddMove(int SorE,int &teNum,Te *teTop,int from,int diff,int pin,int Rpin)
{
	if (Rpin==diff||Rpin==-diff) {
		return;
	}
	int to=from+diff;
	int dan=to&0x0f;
	int fromDan=from&0x0f;

	if ((pin==0 || pin==diff || pin==-diff) && !(ban[to]&SorE)) {
		if (ban[from]==SKE && dan<=2) {
			// 必ず成る
			teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
		} else if ((ban[from]==SFU || ban[from]==SKY) && dan<=1) {
			// 必ず成る
			teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
		} else if (ban[from]==EKE && dan>=8) {
			// 必ず成る
			teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
		} else if ((ban[from]==EFU || ban[from]==EKY) && dan>=9) {
			// 必ず成る
			teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
		} else {
			if (SorE==SELF && (fromDan<=3 || dan <=3) && CanPromote[ban[from]]) {
				teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
			} else if (SorE==ENEMY && (fromDan>=7 || dan>=7) && CanPromote[ban[from]]){
				teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
			}
			// 成らない手も生成する。
			teTop[teNum++]=Te(from,to,ban[from],ban[to],0);
		}
	}
}

// 飛車角香車がまっすぐに進む手の生成
void Kyokumen::AddStraight(int SorE,int &teNum,Te *teTop,int from,int dir,int pin,int Rpin)
{
	if (dir==Rpin || dir==-Rpin) {
		return;
	}
	int i;
	if (pin==0 || pin==dir || pin==-dir) {
		// 空白の間、動く手を生成する
		for(i=dir;ban[from+i]==EMPTY;i+=dir) {
			AddMove(SorE,teNum,teTop,from,i,0);
		}
		// 味方の駒でないなら、そこへ動く
		if (!(ban[from+i] & SorE)) {
			AddMove(SorE,teNum,teTop,from,i,0);
		}
	}
}

//toに動く手の生成
void Kyokumen::MoveTo(int SorE,int &teNum,Te *teTop,int to,int* pin)
{
	int p;
	KomaInf koma;

	for(int i=0;i<12;i++) {
		if ((koma=ban[to-Direct[i]]) == EMPTY) {
			p=search(to,-Direct[i]);
			if ((ban[p]&SorE) && CanJump[i][ban[p]]) {
				AddMove(SorE,teNum,teTop,p,to-p,pin[p]);
			}
		} else {
			if ((koma&~SorE)!=OU && (koma&SorE) && (CanMove[i][koma]||CanJump[i][koma])) {
				AddMove(SorE,teNum,teTop,to-Direct[i],Direct[i],pin[to-Direct[i]]);
			}
		}
	}
}

// それっぽく表示する。
void Kyokumen::FPrint(FILE *fp)
{
	int x,y;
	y = 0;
	fprintf(fp,"Hash:%016llu Hand:%016llu Kyokumen:%016llu\n",HashVal,HandHashVal,KyokumenHashVal);

	//fprintf(fp,"持ち駒：");
	fprintf(fp,"Mochigoma:");
	for (x = EHI; x >=EFU; x--) {
		if (Hand[x] > 1) {
			y = 1;
			//fprintf(fp,"%s%2.2s", komaStr2[x], "一二三四五六七八九101112131415161718"+2*Hand[x]-2);
			fprintf(fp,"%s%2.2s", komaStr2[x], "010203040506070809101112131415161718"+2*Hand[x]-2);
		} else if (Hand[x] == 1) {
			y = 1;
			fprintf(fp,"%s", komaStr2[x]);
		}
	}
	if (y) {
		fprintf(fp,"\n");
	} else {
		//fprintf(fp,"なし\n");
		fprintf(fp,"Nothing\n");
	}
	//fprintf(fp,"  ９ ８ ７ ６ ５ ４ ３ ２ １ \n");
	fprintf(fp,"  09 08 07 06 05 04 03 02 01 \n");
	fprintf(fp,"+---------------------------+\n");
	for(y=1;y<=9;y++) {
		fprintf(fp,"|");
		for(x=9;x>=1;x--) {
			fprintf(fp,"%s", komaStr[ban[x*16+y]]);
		}
		//fprintf(fp,"|%2.2s","一二三四五六七八九" + y*2-2);
		fprintf(fp,"|%2.2s","010203040506070809" + y*2-2);
		fprintf(fp,"\n");
	}
	fprintf(fp,"+---------------------------+\n");
	//fprintf(fp,"持ち駒：");
	fprintf(fp,"Mochigoma:");
	y = 0;
	for (x = SHI; x >= SFU; x--) {
		if (Hand[x] > 1) {
			y = 1;
			//fprintf(fp,"%s%2.2s", komaStr2[x], "一二三四五六七八九101112131415161718"+2*Hand[x]-2);
			fprintf(fp,"%s%2.2s", komaStr2[x], "010203040506070809101112131415161718"+2*Hand[x]-2);
		} else if (Hand[x] == 1) {
			y = 1;
			fprintf(fp,"%s", komaStr2[x]);
		}
	}
	if (y) {
		fprintf(fp,"\n");
	} else {
		//fprintf(fp,"なし\n");
		fprintf(fp,"Nothing\n");
	}
}

const char *komaStrForDump[]={
"EMP","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
"   ","SFU","SKY","SKE","SGI","SKI","SKA","SHI","SOU","STO","SNY","SNK","SNG","SKI","SUM","SRY",
"   ","EFU","EKY","EKE","EGI","EKI","EKA","EHI","EOU","ETO","ENY","ENK","ENG","EKI","EUM","ERY",
"WAL","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};


void Kyokumen::Dump()
{
	int x,y;
	printf("TestBan[9][9]={\n");
	for(y=1;y<=9;y++) {
		printf("\t{");
		for(x=9;x>=1;x--) {
			printf("%s",komaStrForDump[ban[x*16+y]]);
			if (x>1) printf(",");
		}
		printf("}");
		if (y<9) printf(",");
		printf("\n");
	}
	printf("};\n");
	printf("Motigoma[EHI+1]={");
	for(int koma=0;koma<=EHI;koma++) {
		printf("%d,",Hand[koma]);
	}
	printf("};\n");
}

// 第３章で追加。交換値を求める。
int kyori(int p1,int p2)
{
	return max(abs(p1/16-p2/16),abs((p1 & 0x0f)-(p2 &0x0f)));
}

int Kyokumen::IsCorrectMove(Te &te)
{
	// 第5章で追加。駒打ちの場合に、正しい手かどうかチェックする。
	if (te.from==0) {
		if (ban[te.to]!=EMPTY) return 0;
		if (te.koma==SFU) {
			// 二歩と打ち歩詰めのチェック
			for(int dan=1;dan<=9;dan++) {
				if (ban[(te.to&0xf0)+dan]==SFU) return 0;
			}
			if (te.to==kingE+1) {
				int pin[16*11];
				MakePinInf(pin);
				if (Utifudume(SELF,te.to,pin)) return 0;
			}
		}
		if (te.koma==EFU) {
			// 二歩と打ち歩詰めのチェック
			for(int dan=1;dan<=9;dan++) {
				if (ban[(te.to&0xf0)+dan]==EFU) return 0;
			}
			if (te.to==kingS-1) {
				int pin[16*11];
				MakePinInf(pin);
				if (Utifudume(ENEMY,te.to,pin)) return 0;
			}
		}
		return 1;
	}
	if (ban[te.from]==SOU) {
		if (controlE[te.to]!=0) {
			return 0;
		} else {
			te.capture=ban[te.to];
			return 1;
		}
	} else if (ban[te.from]==EOU) {
		if (controlS[te.to]!=0) {
			return 0;
		} else {
			te.capture=ban[te.to];
			return 1;
		}
	}
	if (ban[te.from]==SKE || ban[te.from]==EKE) {
		te.capture=ban[te.to];
		return 1;
	}
	int d=kyori(te.from,te.to);
	if (d==0) return 0;
	int dir=(te.to-te.from)/d;
	if (d==1) {
		te.capture=ban[te.to];
		return 1;
	}
	// ジャンプなので、途中に邪魔な駒がいないかどうかチェックする
	for(int i=1,pos=te.from+dir;i<d;i++,pos=pos+dir) {
		if (ban[pos]!=EMPTY) {
			return 0;
		}
	}
	te.capture=ban[te.to];
	return 1;
}

int Kyokumen::IsLegalMove(int SorE,Te &te)
{
	if (!(te.koma & SorE)) {
		// 自分の駒でない駒を動かしている
		return 0;
	}
	if (te.from<OU) {
		if (Hand[te.koma]==0) return 0;
	} else {
		if (ban[te.from]!=te.koma) return 0;
	}
	if (ban[te.to] & SorE) {
		// 自分の駒を取っている
		return 0;
	}
	if (IsCorrectMove(te)) {
		// 自玉に王手をかけていないか、実際に動かして調べる
		Kyokumen kk(*this);
		kk.Move(SorE,te);
		if (SorE==SELF && kk.controlE[kk.kingS]) {
			return 0;
		}
		if (SorE==ENEMY && kk.controlS[kk.kingE]) {
			return 0;
		}
		return 1;
	}
	return 0;
}


int Kyokumen::EvalMin(Te *AtackS,int NumAtackS,Te *AtackE,int NumAtackE)
{
	int v=value;
	if (NumAtackE>0) {
		int k=0;
		while(!IsCorrectMove(AtackE[k]) && k<NumAtackE) {
			k++;
		}
		if (k==0) {
		} else if (k<NumAtackE) {
			Te t=AtackE[k];
			for(int i=k;i>0;i--) {
				AtackE[i]=AtackE[i-1];
			}
			AtackE[0]=t;
		} else {
			// 他に手がない＝取れない。
			return v;
		}
		AtackE[0].capture=ban[AtackE[0].to];
		Move(ENEMY,AtackE[0]);
		return min(v,
			EvalMax(AtackS,NumAtackS,AtackE+1,NumAtackE-1));
	} else {
		return v;
	}
}

int Kyokumen::EvalMax(Te *AtackS,int NumAtackS,Te *AtackE,int NumAtackE)
{
	int v=value;
	if (NumAtackS>0) {
		// 邪魔駒の処理
		int k=0;
		while(!IsCorrectMove(AtackS[k]) && k<NumAtackS) {
			k++;
		}
		if (k==0) {
		} else if (k<NumAtackS) {
			Te t=AtackS[k];
			for(int i=k;i>0;i--) {
				AtackS[i]=AtackS[i-1];
			}
			AtackS[0]=t;
		} else {
			// 他に手がない＝取れない。
			return v;
		}
		AtackS[0].capture=ban[AtackS[0].to];
		Move(SELF,AtackS[0]);
		return max(v,
			EvalMin(AtackS+1,NumAtackS-1,AtackE,NumAtackE));
	} else {
		return v;
	}
}

int Kyokumen::Eval(int position)
{
	if (ban[position]==EMPTY) {
		return 0;
	}
	if ((ban[position]&SELF) && !controlE[position]) {
		// 取られる心配がない
		return 0;
	}
	if ((ban[position]&ENEMY) && !controlS[position]) {
		// 取られる心配がない
		return 0;
	}

	int ret;
	Te teTop[40];
	int ToPos=position;

	// AtackCountを得るように、駒のリストを得る
	Te *AtackS=teTop;
	// 一個所への利きは、最大隣接8+桂馬2+飛飛角角香香香香=18だから。
	Te *AtackE=teTop+18;

	int AtackCountE=0;
	int AtackCountS=0;
	int pos2;

	int PromoteS,PromoteE;
	int b=1;
	int bj=1<<16;
	int i;
	int pos=ToPos;
	if ((ToPos&0x0f)<=3) {
		PromoteS=1;
	} else {
		PromoteS=0;
	}
	if ((ToPos&0x0f)>=7) {
		PromoteE=1;
	} else {
		PromoteE=0;
	}

	// 桂馬の利きは別に数える
	for (i = 0; i < 8; i++) {
		pos2=pos;
		if (controlS[pos] & b) {
			pos2-=Direct[i];
			AtackS[AtackCountS].from=pos2;
			AtackS[AtackCountS].koma=ban[pos2];
			AtackS[AtackCountS].to=pos;
			if ((PromoteS || (pos2 & 0x0f)<=3) && CanPromote[AtackS[AtackCountS].koma]) {
				AtackS[AtackCountS].promote=1;
			} else {
				AtackS[AtackCountS].promote=0;
			}
			AtackCountS++;
		} else if (controlE[pos] & b) {
			pos2-=Direct[i];
			AtackE[AtackCountE].from=pos2;
			AtackE[AtackCountE].koma=ban[pos2];
			AtackE[AtackCountE].to=pos;
			if ((PromoteE || (pos2 & 0x0f)>=7) && CanPromote[AtackE[AtackCountE].koma]) {
				AtackE[AtackCountE].promote=1;
			} else {
				AtackE[AtackCountE].promote=0;
			}
			AtackCountE++;
		}
		//if (ban[pos-Direct[i]]!=OU && ban[pos-Direct[i]]!=EOU) {
		// http://lesserpyon.bbs.coocan.jp/?m=listthread&t_id=5により修正
		if (ban[pos-Direct[i]]!=SOU && ban[pos-Direct[i]]!=EOU) {
			while((controlS[pos2] & bj) || (controlE[pos2] & bj)) {
				pos2-=Direct[i];
				while(ban[pos2]==EMPTY) {
					pos2-=Direct[i];
				}
//				if (ban[pos2]==WALL) continue;//不要なはずなのに
				if ((ban[pos2])&ENEMY) {
					AtackE[AtackCountE].from=pos2;
					AtackE[AtackCountE].koma=ban[pos2];
					AtackE[AtackCountE].to=pos;
					if ((PromoteE || (pos2 & 0x0f)>=7) && CanPromote[AtackE[AtackCountE].koma]) {
						AtackE[AtackCountE].promote=1;
					} else {
						AtackE[AtackCountE].promote=0;
					}
					AtackCountE++;
				} else if ((ban[pos2])&SELF) {
					AtackS[AtackCountS].from=pos2;
					AtackS[AtackCountS].koma=ban[pos2];
					AtackS[AtackCountS].to=pos;
					if ((PromoteS || (pos2 & 0x0f)<=3) && CanPromote[AtackS[AtackCountS].koma]) {
						AtackS[AtackCountS].promote=1;
					} else {
						AtackS[AtackCountS].promote=0;
					}
					AtackCountS++;
				}
			}
		}
		b<<=1;
		bj<<=1;
	}
	// 桂馬の利き
	b=1<<8;
	for(i=8;i<12;i++) {
		if (controlS[pos] & b) {
			pos2=pos-Direct[i];
			AtackS[AtackCountS].from=pos2;
			AtackS[AtackCountS].koma=ban[pos2];
			AtackS[AtackCountS].to=pos;
			if (PromoteS && CanPromote[AtackS[AtackCountS].koma]) {
				AtackS[AtackCountS].promote=1;
			} else {
				AtackS[AtackCountS].promote=0;
			}
			AtackCountS++;
		}
		if (controlE[pos] & b) {
			pos2=pos-Direct[i];
			AtackE[AtackCountE].from=pos2;
			AtackE[AtackCountE].koma=ban[pos2];
			AtackE[AtackCountE].to=pos;
			if (PromoteE && CanPromote[AtackE[AtackCountE].koma]) {
				AtackE[AtackCountE].promote=1;
			} else {
				AtackE[AtackCountE].promote=0;
			}
			AtackCountE++;
		}
		b<<=1;
	}
	// 駒の価値でソート。
	for (i=0; i < AtackCountS-1; i++) {
		int max_id = i; int max_val = KomaValue[AtackS[i].koma];
		for (int j = i+1; j < AtackCountS ; j++) {
			int v=KomaValue[AtackS[j].koma];
			if (v < max_val) {
				max_id = j;
				max_val= v;
			} else if (v==max_val) {
				if (KomaValue[AtackS[j].koma]<KomaValue[AtackS[max_id].koma]) {
					max_id=j;
				}
			}
		}
		//最大値との交換
		if (i!=max_id) {
			swap(AtackS[i],AtackS[max_id]);
		}
	}
	// 駒の価値でソート。
	for (i=0; i < AtackCountE-1; i++) {
		int max_id = i; int max_val = KomaValue[AtackE[i].koma];
		for (int j = i+1; j < AtackCountE ; j++) {
			int v=KomaValue[AtackE[j].koma];
			if (v> max_val) {
				max_id = j;
				max_val= v;
			} else if (v==max_val) {
				if (KomaValue[AtackE[j].koma]>KomaValue[AtackE[max_id].koma]) {
					max_id=j;
				}
			}
		}
		//最大値との交換
		if (i!=max_id) {
			swap(AtackE[i],AtackE[max_id]);
		}
	}

	int IsEnemy=(ban[position] & ENEMY);
	int IsSelf=!IsEnemy && ban[position]!=EMPTY;
	if (IsEnemy && AtackCountS>0) {
		int Eval=value;
		Kyokumen now(*this);
		ret=now.EvalMax(AtackS,AtackCountS,AtackE,AtackCountE)-Eval;
	} else if (IsSelf && AtackCountE>0) {
		int Eval=value;
		Kyokumen now(*this);
		ret=Eval-now.EvalMin(AtackS,AtackCountS,AtackE,AtackCountE);
	} else {
		ret=0;
	}
	return ret;
}

int Kyokumen::BestEval(int SorE)
{
	// SorEの利きのある敵の駒（&SorE==0の駒）について、Evalを呼び出して、
	// 一番いい交換値を探す。
	int best=0;
	for(int suji=0x10;suji<=0x90;suji+=0x10) {
		for(int dan=1;dan<=9;dan++) {
			if ((ban[suji+dan]&SorE)==0) {
				int value=Eval(suji+dan);
				if (value>best) {
					best=value;
				}
			}
		}
	}
	if (SorE==ENEMY) return -best;
	return best;	
}

int KyokumenKomagumi::KomagumiValue[ERY+1][16*11];
int KyokumenKomagumi::SemegomaValueS[16*11][16*11];
int KyokumenKomagumi::SemegomaValueE[16*11][16*11];
int KyokumenKomagumi::MamorigomaValueS[16*11][16*11];
int KyokumenKomagumi::MamorigomaValueE[16*11][16*11];

int DanValue[ERY+1][10]={
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//空
	{0,0,0,0,0,0,0,0,0,0},
//歩
	{ 0,  0,15,15,15,3,1, 0, 0, 0},
//香
	{ 0, 1,2,3,4,5,6,7,8,9},
//桂
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//銀
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//金
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//角
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//飛
	{ 0,10,10,10, 0, 0, 0,  -5, 0, 0},
//王
	{ 0,1200,1200,900,600,300,-10,0,0,0},
//と
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//成香
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//成桂
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//成銀
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//金
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//馬
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//龍
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//空
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//歩
	{ 0, 0, 0, 0, -1, -3,-15,-15,-15, 0},
//香
	{ 0,-9,-8,-7, -6, -5, -4, -3, -2,-1},
//桂
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//銀
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//金
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//角
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//飛
	{ 0, 0, 0, 5, 0, 0, 0,-10,-10,-10},
//王
	{ 0, 0, 0, 0,10,-300,-600,-900,-1200,-1200},
//と
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//成香
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//成桂
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//成銀
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//金
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//馬
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//龍
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

enum {
	IvsFURI,		// 居飛車対振り飛車
	IvsNAKA,		// 居飛車対中飛車
	FURIvsFURI,		// 相振り飛車
	FURIvsI,		// 振り飛車対居飛車
	NAKAvsI,		// 中飛車対居飛車
	KAKUGAWARI,		// 角換り
	AIGAKARI,		// 相掛かり（または居飛車の対抗系）
	HUMEI			// 戦形不明
};

int JosekiKomagumiSGI[HUMEI+1][9][9]=
{
	{	// IvsFURI 舟囲い、美濃、銀冠
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10, -7,-10,-10,-10,-10,-10,  7,-10},
		{-10,  7, -8, -7, 10,-10, 10,  6,-10},
		{-10, -2, -6, -5,-10,  6,-10,-10,-10},
		{-10, -7,  0,-10,-10,-10,-10,-10,-10}
	},{	// IvsNAKA　舟囲い
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10, -7,-10,-10, -7,-10,-10,  7,-10},
		{-10, -5, -8, -7, 10,-10, 10,  6,-10},
		{-10, -2, -3,  0,-10,  6,-10,-10,-10},
		{-10, -7, -5,-10,-10,-10,-10,-10,-10}
	},{ // FURIvsFURI　矢倉（逆）、美濃、銀冠
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10, -7, -7,-10},
		{-10,-10,-10,-10,-10,  5, 10, 10,-10},
		{-10,-10,-10,-10,-10,-10,  0,-10,-10},
		{-10,-10,-10,-10,-10,-10, -5,-10,-10}
	},{ // FURIvsI 美濃囲い、銀冠
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10, -3, -7,-10,-10,-10,-10,-10},
		{-10, -7,  4,  6,-10,-10,-10,  6,-10},
		{-10,  2,  3,  3,-10,-10,  4,-10,-10},
		{-10,-10,-10,  0,-10,-10,  0,-10,-10}
	},{ // NAKAvsI 中飛車
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  8,  5,  8,-10,-10,-10},
		{-10,-10,  4,  4,  3,  4,  4,-10,-10},
		{-10,-10,  0,-10,-10,-10,  0,-10,-10}
	},{ // KAKUGAWARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,  7,  5, -3,-10,-10},
		{-10,  8, 10,  7,  4,  0, -4,-10,-10},
		{-10,  0,-8,  -4,-10,-10, -5,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10}
	},{ // AIGAKARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,  0,-10,-10,-10,-10,-10,-10},
		{-10, -5,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10}
	},{ // HUMEI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,  5,-10,-10},
		{-10,-10,-10,-10,-10,-10, -4,  0,-10},
		{-10,-10,  0,-10,-10,-10, -4, -3,-10},
		{-10, -5,-10, -5,-10,-10, -5,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10}
	}
};

int JosekiKomagumiSKI[HUMEI+1][9][9]=
{
	{	// IvsFURI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  1,  2,-10,-10,-10,-10},
		{-10,-10,-10,  0,-10, -4,-10,-10,-10}
	},{	// IvsNAKA
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  1,  2,-10,-10,-10,-10},
		{-10,-10,-10,  0,-10, -4,-10,-10,-10}
	},{ // FURIvsFURI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  7, -3,-10,-10},
		{-10,-10,-10,-10,  5,  3,  6,-10,-10},
		{-10,-10,-10,-10,-10,  5,  4,-10,-10}
	},{ // FURIvsI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  5,  1,-10,-10},
		{-10,-10,-10,-10,  4,  3,  7, -3,-10},
		{-10,-10,-10,  0,  1,  5,  2, -7,-10}
	},{ // NAKAvsI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10, -7, -4, -4,-10, -4, -4, -7,-10},
		{-10, -5, 10,  6,-10,  8, 10, -5,-10},
		{-10, -7, -6, -3, -6, -3, -6, -7,-10}
	},{ // KAKUGAWARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  6, -4, -4, -4, -8,-10},
		{-10,-10, 10,-10,  3,  0,  0, -7,-10},
		{-10,-10,-10,  0,-10,  0, -5, -7,-10}
	},{ // AIGAKARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  6,-10,-10,-10,-10,-10},
		{-10,-10, 10,-10,  3,-10,-10,-10,-10},
		{-10,-10,-10,  0,-10,  0,-10,-10,-10}
	},{ // HUMEI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,  3,-10,  5,-10,-10,-10,-10},
		{-10,-10,-10,  0,-10,  0,-10,-10,-10}
	}
};

int JosekiKomagumiSOU[HUMEI+1][9][9]=
{
	{	// IvsFURI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{- 7,  9,-10,-10,-10,-10,-10,-10,-10},
		{  5,  7,  8,  4,-10,-10,-10,-10,-10},
		{ 10,  5,  3,-10,-10,-10,-10,-10,-10}
	},{	// IvsNAKA
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{- 7,  9,-10,-10,-10,-10,-10,-10,-10},
		{  5,  7,  8,  4,-10,-10,-10,-10,-10},
		{ 10,  5,  3,-10,-10,-10,-10,-10,-10}
	},{ // FURIvsFURI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  4,  6, 10,  6},
		{-10,-10,-10,-10,-10,  4,  6,  5, 10}
	},{ // FURIvsI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  4,  6, 10,  6},
		{-10,-10,-10,-10,-10,  4,  6,  5, 10}
	},{ // NAKAvsI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  4,  6, 10,  6},
		{-10,-10,-10,-10,-10,  4,  6,  5, 10}
	},{ // KAKUGAWARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{- 3, -4, -3,-10,-10,-10,-10,-10,-10},
		{  6,  8, -2,  0, -3,-10,-10,-10,-10},
		{ 10,  6, -4,- 6,- 7,-10,-10,-10,-10}
	},{ // AIGAKARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{- 3, -4, -3,-10,-10,-10,-10,-10,-10},
		{  6,  8,  0,- 4,-10,-10,-10,-10,-10},
		{ 10,  6, -4,- 6,- 7,-10,-10,-10,-10}
	},{ // HUMEI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{- 3, -4, -3,-10,-10,-10,-10,-10,-10},
		{  6,  8,  0,- 4,-10,-10,-10,-10,-10},
		{ 10,  6, -4,- 6,- 7,-10,-10,-10,-10}
	}
};

int JosekiKomagumi[9][ERY+1][16*11];

void KyokumenKomagumi::Initialize()
{
	int suji,dan,koma;
	for(suji=0x10;suji<=0x90;suji+=0x10) {
		for(dan=1;dan<=9;dan++) {
			for(koma=SFU;koma<=ERY;koma++) {
				KomagumiValue[koma][suji+dan]=0;
				JosekiKomagumi[0][koma][suji+dan]=DanValue[koma][dan];
			}
		}
	}
	InitKanagomaValue();
	InitShuubando();
	InitBonus();
}

void KyokumenKomagumi::SenkeiInit()
{
	int SHI1,SHI2;
	int EHI1,EHI2;
	int SKA1,SKA2;
	int EKA1,EKA2;
	int suji,dan,koma;
	SHI1=SHI2=EHI1=EHI2=SKA1=SKA2=EKA1=EKA2=0;
	for(suji=0x10;suji<=0x90;suji+=0x10) {
		for(dan=1;dan<=9;dan++) {
			if (ban[suji+dan]==SHI) {
				if (SHI1==0) SHI1=suji+dan; else SHI2=suji+dan;
			}
			if (ban[suji+dan]==EHI) {
				if (EHI1==0) EHI1=suji+dan; else EHI2=suji+dan;
			}
			if (ban[suji+dan]==SKA) {
				if (SKA1==0) SKA1=suji+dan; else SKA2=suji+dan;
			}
			if (ban[suji+dan]==EKA) {
				if (EKA1==0) EKA1=suji+dan; else EKA2=suji+dan;
			}
		}
	}
	if (Hand[SHI]==1) { if (SHI1==0) SHI1=1; else SHI2=1; }
	if (Hand[SHI]==2) SHI1=SHI2=1;
	if (Hand[EHI]==1) { if (EHI1==0) EHI1=1; else EHI2=1; }
	if (Hand[EHI]==2) EHI1=EHI2=1;
	if (Hand[SKA]==1) { if (SKA1==0) SKA1=1; else SKA2=1; }
	if (Hand[SKA]==2) SKA1=SKA2=1;
	if (Hand[EKA]==1) { if (EKA1==0) EKA1=1; else EKA2=1; }
	if (Hand[EKA]==2) EKA1=EKA2=1;

	int Senkei,GyakuSenkei;
	if (SHI1<=0x50 && EHI1<=0x50) {
		Senkei=IvsFURI;
		GyakuSenkei=FURIvsI;
	} else if (0x50<=EHI1 && EHI1<=0x5f && SHI1<=0x50) {
		Senkei=IvsNAKA;
		GyakuSenkei=NAKAvsI;
	} else if (SHI1<=0x5f && EHI1<=0x5f) {
		Senkei=FURIvsFURI;
		GyakuSenkei=FURIvsFURI;
	} else if (EHI1>=0x60 && SHI1>=0x60) {
		Senkei=FURIvsI;
		GyakuSenkei=IvsFURI;
	} else if (0x50<=SHI1 && SHI1<=0x5f && EHI1<=0x50) {
		Senkei=NAKAvsI;
		GyakuSenkei=IvsNAKA;
	} else if (SKA1==1 && EKA1==1) {
		Senkei=KAKUGAWARI;
		GyakuSenkei=KAKUGAWARI;
	} else if (0x20<=SHI1 && SHI1<=0x2f && 0x80<=EHI1 && EHI1<=0x8f) {
		Senkei=AIGAKARI;
		GyakuSenkei=AIGAKARI;
	} else {
		Senkei=HUMEI;
		GyakuSenkei=HUMEI;
	}
	KomagumiBonus[0]=KomagumiBonus[1]=0;
	for(suji=0x10;suji<=0x90;suji+=0x10) {
		for(dan=1;dan<=9;dan++) {
			value-=KomagumiValue[ban[suji+dan]][suji+dan];
			for(koma=SFU;koma<=ERY;koma++) {
				if (koma==SGI) {
					JosekiKomagumi[Senkei][koma][suji+dan]=JosekiKomagumiSGI[Senkei][dan-1][9-(suji/0x10)];
				} else if (koma==EGI) {
					JosekiKomagumi[Senkei][koma][suji+dan]=-JosekiKomagumiSGI[GyakuSenkei][9-dan][suji/0x10-1];
				} else if (koma==SKI) {
					JosekiKomagumi[Senkei][koma][suji+dan]=JosekiKomagumiSKI[Senkei][dan-1][9-(suji/0x10)];
				} else if (koma==EKI) {
					JosekiKomagumi[Senkei][koma][suji+dan]=-JosekiKomagumiSKI[GyakuSenkei][9-dan][suji/0x10-1];
				} else if (koma==SOU) {
					JosekiKomagumi[Senkei][koma][suji+dan]=JosekiKomagumiSOU[Senkei][dan-1][9-(suji/0x10)];
				} else if (koma==EOU) {
					JosekiKomagumi[Senkei][koma][suji+dan]=-JosekiKomagumiSOU[GyakuSenkei][9-dan][suji/0x10-1];
				} else {
					JosekiKomagumi[Senkei][koma][suji+dan]=DanValue[koma][dan];
				}
				KomagumiValue[koma][suji+dan]=JosekiKomagumi[Senkei][koma][suji+dan];
			}
			if (ban[suji+dan] & SELF) {
				KomagumiBonus[0]+=KomagumiValue[ban[suji+dan]][suji+dan];
			} else if (ban[suji+dan] & ENEMY) {
				KomagumiBonus[1]+=KomagumiValue[ban[suji+dan]][suji+dan];
			}
		}
	}
}

static int Mamorigoma[17][9]={
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 56, 52, 50, 50, 50, 50, 50, 50, 50},
	{ 64, 61, 55, 50, 50, 50, 50, 50, 50},
	{ 79, 77, 70, 65, 54, 51, 50, 50, 50},
	{100, 99, 95, 87, 74, 58, 50, 50, 50},
	{116,117,101, 95, 88, 67, 54, 50, 50},
	{131,129,124,114, 90, 71, 59, 51, 50},
	{137,138,132,116, 96, 76, 61, 53, 50},
	{142,142,136,118, 98, 79, 64, 52, 50},
	{132,132,129,109, 95, 75, 60, 51, 50},
	{121,120,105, 97, 84, 66, 54, 50, 50},
	{ 95, 93, 89, 75, 68, 58, 51, 50, 50},
	{ 79, 76, 69, 60, 53, 50, 50, 50, 50},
	{ 64, 61, 55, 51, 50, 50, 50, 50, 50},
	{ 56, 52, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
};

static int Semegoma[17][9]={
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 54, 53, 51, 51, 50, 50, 50, 50, 50},
	{ 70, 66, 62, 55, 53, 50, 50, 50, 50},
	{ 90, 85, 80, 68, 68, 60, 53, 50, 50},
	{100, 97, 95, 85, 84, 71, 51, 50, 50},
	{132,132,129,102, 95, 71, 51, 50, 50},
	{180,145,137,115, 91, 75, 57, 50, 50},
	{170,165,150,121, 94, 78, 58, 52, 50},
	{170,160,142,114, 98, 80, 62, 55, 50},
	{140,130,110,100, 95, 75, 54, 50, 50},
	{100, 99, 95, 87, 78, 69, 50, 50, 50},
	{ 80, 78, 72, 67, 55, 51, 50, 50, 50},
	{ 62, 60, 58, 52, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
};

void KyokumenKomagumi::InitKanagomaValue()
{
	for(int kingSdan=1;kingSdan<=9;kingSdan++) {
		for(int kingSsuji=0x10;kingSsuji<=0x90;kingSsuji+=0x10) {
			for(int kingEdan=1;kingEdan<=9;kingEdan++) {
				for(int kingEsuji=0x10;kingEsuji<=0x90;kingEsuji+=0x10) {
					for(int suji=0x10;suji<=0x90;suji+=0x10) {
						for(int dan=1;dan<=9;dan++) {
							int DiffSujiS=abs(kingSsuji-suji)/0x10;
							int DiffSujiE=abs(kingEsuji-suji)/0x10;
							int DiffDanSS=8+(dan-kingSdan);
							int DiffDanES=8+(dan-kingEdan);
							int DiffDanSE=8+(-(dan-kingSdan));
							int DiffDanEE=8+(-(dan-kingEdan));
							int kingS=kingSsuji+kingSdan;
							int kingE=kingEsuji+kingEdan;

							SemegomaValueS[suji+dan][kingE]=Semegoma[DiffDanES][DiffSujiE]-100;
							MamorigomaValueS[suji+dan][kingS]=Mamorigoma[DiffDanSS][DiffSujiS]-100;
							SemegomaValueE[suji+dan][kingS]=-(Semegoma[DiffDanSE][DiffSujiS]-100);
							MamorigomaValueE[suji+dan][kingE]=-(Mamorigoma[DiffDanEE][DiffSujiE]-100);
						}
					}
				}
			}
		}
	}
}
					  //空歩香桂銀金角飛王と杏圭全金馬龍
int ShuubandoByAtack[]={0,1,1,2,3,3,3,4,4,3,3,3,3,3,4,5};
						//空歩香桂銀 金角飛王 と 杏 圭 全 金 馬 龍
int ShuubandoByDefence[]={0,0,0,0,-1,-1,0,0,0,-1,-1,-1,-1,-1,-2,0};
					//空歩香桂銀金角飛王と杏圭全金馬龍
int ShuubandoByHand[]={0,0,1,1,2,2,2,3,0,0,0,0,0,0,0,0};

void KyokumenKomagumi::InitShuubando()
{
	// 終盤度を求めると同時に、終盤度によるボーナスの付加、駒の加点も行う。
	int suji,dan;
	Shuubando[0]=0;
	Shuubando[1]=0;
	for(suji=0x10;suji<=0x90;suji+=0x10) {
		for(dan=1;dan<=4;dan++) {
			if (ban[suji+dan] & SELF) {
				Shuubando[1]+=ShuubandoByAtack[ban[suji+dan] & ~SELF];
			}
			if (ban[suji+dan] & ENEMY) {
				Shuubando[1]+=ShuubandoByDefence[ban[suji+dan] & ~ENEMY];
			}
		}
		for(dan=6;dan<=9;dan++) {
			if (ban[suji+dan] & ENEMY) {
				Shuubando[0]+=ShuubandoByAtack[ban[suji+dan] & ~ENEMY];
			}
			if (ban[suji+dan] & SELF) {
				Shuubando[0]+=ShuubandoByDefence[ban[suji+dan] & ~SELF];
			}
		}
	}
	int koma;
	for(koma=FU;koma<=HI;koma++) {
		Shuubando[0]+=ShuubandoByHand[koma]*Hand[ENEMY|koma];
		Shuubando[1]+=ShuubandoByHand[koma]*Hand[SELF|koma];
	}
}

int IsKanagoma[]={
//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,0,0,
//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void KyokumenKomagumi::InitBonus()
{
	int suji,dan;
	SemegomaBonus[0]=SemegomaBonus[1]=0;
	MamorigomaBonus[0]=MamorigomaBonus[1]=0;

	for(suji=0x10;suji<=0x90;suji+=0x10) {
		for(dan=1;dan<=9;dan++) {
			if (IsKanagoma[ban[suji+dan]]) {
				if (ban[suji+dan] & SELF) {
					SemegomaBonus[0]+=SemegomaValueS[suji+dan][kingE];
					MamorigomaBonus[0]+=MamorigomaValueS[suji+dan][kingS];
				} else {
					SemegomaBonus[1]+=SemegomaValueE[suji+dan][kingS];
					MamorigomaBonus[1]+=MamorigomaValueE[suji+dan][kingE];
				}
			}
		}
	}
}

void KyokumenKomagumi::Move(int SorE,const Te &te)
{
	int self,enemy;
	if (SorE==SELF) {
		self=0;
		enemy=1;
	} else {
		self=1;
		enemy=0;
	}
	//if (te.koma==OU || te.koma==EOU) {
	// http://lesserpyon.bbs.coocan.jp/?m=listthread&t_id=5により修正
	if (te.koma==SOU || te.koma==EOU) {
		// 駒を動かした後で、全面的にBonusの計算しなおしが必要。
		// Semegoma,Mamorigomaなど全て変わるので。
	} else {
		if (IsKanagoma[te.koma] && te.from>0) {
			if (SorE==SELF) {
				SemegomaBonus[0]-=SemegomaValueS[te.from][kingE];
				MamorigomaBonus[0]-=MamorigomaValueS[te.from][kingS];
			} else {
				SemegomaBonus[1]-=SemegomaValueE[te.from][kingS];
				MamorigomaBonus[1]-=MamorigomaValueE[te.from][kingE];
			}
		}
		if (te.capture) {
			if (IsKanagoma[te.capture]) {
				if (SorE==SELF) {
					SemegomaBonus[1]-=SemegomaValueE[te.to][kingS];
					MamorigomaBonus[1]-=MamorigomaValueE[te.to][kingE];
				} else {
					SemegomaBonus[0]-=SemegomaValueS[te.to][kingE];
					MamorigomaBonus[0]-=MamorigomaValueS[te.to][kingS];
				}
			}
		}
		if (!te.promote) {
			if (IsKanagoma[te.koma]) {
				if (SorE==SELF) {
					SemegomaBonus[0]+=SemegomaValueS[te.to][kingE];
					MamorigomaBonus[0]+=MamorigomaValueS[te.to][kingS];
				} else {
					SemegomaBonus[1]+=SemegomaValueE[te.to][kingS];
					MamorigomaBonus[1]+=MamorigomaValueE[te.to][kingE];
				}
			}
		} else {
			if (IsKanagoma[te.koma|PROMOTED]) {
				if (SorE==SELF) {
					SemegomaBonus[0]+=SemegomaValueS[te.to][kingE];
					MamorigomaBonus[0]+=MamorigomaValueS[te.to][kingS];
				} else {
					SemegomaBonus[1]+=SemegomaValueE[te.to][kingS];
					MamorigomaBonus[1]+=MamorigomaValueE[te.to][kingE];
				}
			}
		}
	}
	KomagumiBonus[self]-=KomagumiValue[te.koma][te.from];
	if (te.from>0 && (te.from&0x0f)<=4) {
		// ４段目以下・終盤度の計算
		if (SorE==SELF) {
			Shuubando[1]-=ShuubandoByAtack[te.koma & ~SELF];
		} else {
			Shuubando[1]-=ShuubandoByDefence[te.koma & ~ENEMY];
		}
	}
	if (te.from>0 && (te.from&0x0f)>=6) {
		// ６段目以上・終盤度の計算
		if (SorE==SELF) {
			Shuubando[0]-=ShuubandoByDefence[te.koma & ~SELF];
		} else {
			Shuubando[0]-=ShuubandoByAtack[te.koma & ~ENEMY];
		}
	}
	if (te.capture) {
		if ((te.to&0x0f)<=4) {
			// ４段目以下・終盤度の計算
			if (SorE==SELF) {
				Shuubando[1]-=ShuubandoByDefence[te.capture & ~ENEMY];
			}
		}
		if ((te.to&0x0f)>=6) {
			// ６段目以上・終盤度の計算
			if (SorE==SELF) {
				Shuubando[0]-=ShuubandoByDefence[te.capture & ~SELF];
			}
		}
		// Handに入ったことによる終盤度の計算
		KomagumiBonus[enemy]-=KomagumiValue[te.capture][te.to];
	}
	if (!te.promote) {
		if ((te.to&0x0f)<=4) {
			// ４段目以下・終盤度の計算
			if (SorE==SELF) {
				Shuubando[1]+=ShuubandoByAtack[te.koma & ~SELF];
			} else {
				Shuubando[1]+=ShuubandoByDefence[te.koma & ~ENEMY];
			}
		}
		if ((te.to&0x0f)>=6) {
			// ６段目以上・終盤度の計算
			if (SorE==SELF) {
				Shuubando[0]+=ShuubandoByDefence[te.koma & ~SELF];
			} else {
				Shuubando[0]+=ShuubandoByAtack[te.koma & ~ENEMY];
			}
		}
		KomagumiBonus[self]+=KomagumiValue[te.koma][te.to];
	} else {
		if ((te.to&0x0f)<=4) {
			// ４段目以下・終盤度の計算
			if (SorE==SELF) {
				Shuubando[1]+=ShuubandoByAtack[(te.koma|PROMOTED) & ~SELF];
			} else {
				Shuubando[1]+=ShuubandoByDefence[(te.koma|PROMOTED) & ~ENEMY];
			}
		}
		if ((te.to&0x0f)>=6) {
			// ６段目以上・終盤度の計算
			if (SorE==SELF) {
				Shuubando[0]+=ShuubandoByDefence[(te.koma|PROMOTED) & ~SELF];
			} else {
				Shuubando[0]+=ShuubandoByAtack[(te.koma|PROMOTED) & ~ENEMY];
			}
		}
		KomagumiBonus[self]+=KomagumiValue[te.koma|PROMOTED][te.to];
	}
	Kyokumen::Move(SorE,te);
	if (te.koma==SOU || te.koma==EOU) {
		// 全面的に金駒のBonusの計算しなおし。
		InitBonus();
	}
}

extern bool isPonderhitReceived; // ponderhitコマンドを受信したか
extern bool canThrow; // 思考中断が可能かどうか
extern bool isPonderThink; // 先読み思考中か
extern unsigned long thinkStartTime; // 思考を開始した時刻
extern unsigned long ponderhitReceiveTime; // ponderhitを受信した時刻
extern unsigned long evaluatedNodes; // KyokumenKomagumi::Evaluate()が呼ばれた回数
extern unsigned long hashCount; // ハッシュに追加された数
extern unsigned long remainTime; // 残り時間
extern unsigned long byoyomiTime; // 秒読みの時間
extern bool isInfinite; // 思考時間が無制限かどうか

unsigned long prevTime; // 前回、infoコマンドでhashfullを返した時刻
int thinkLimitTime; // 思考の制限時間

#define SHOWHASH 1

int KyokumenKomagumi::Evaluate()
{
	if (evaluatedNodes == 0) {
		prevTime = timeGetTime();
		if (byoyomiTime == 0) { // 秒読みがない場合
			if (remainTime > 60 * 1000) {
				// 1分以上時間があるなら、10秒まで考えてもいいことにする。
				thinkLimitTime = 10 * 1000;
			} else if (remainTime > 30 * 1000) {
				// 30秒以上1分以内の時間があるなら、3秒まで考えてもいいことにする。
				thinkLimitTime = 3 * 1000;
			} else {
				// そうでなければ1秒までとする。
				thinkLimitTime = 1000;
			}
		} else if (remainTime == 0) { // 持ち時間を使い切って秒読みだけが残っている場合
			// 秒読みいっぱいまで考えるとする。
			thinkLimitTime = byoyomiTime;
		} else { // 持ち時間が残っていて秒読みがある場合
			// 10秒、もしくは（残り時間＋秒読み）の少ない方とする。
			thinkLimitTime = min((unsigned long)(10 * 1000), remainTime + byoyomiTime);
		}
	}
	unsigned long currTime = timeGetTime();
	unsigned long diffTime = currTime - prevTime;
	if (diffTime >= 1000) {
		unsigned long nps = (uint64)(evaluatedNodes * 1000) / (currTime - thinkStartTime); // evaluatedNodes * 1000がintの範囲を超えることがあるので、uint64でキャストする。
		unsigned long hashfull = hashCount * 1000 / (1024 * 1024);
#if SHOWHASH
		printf("info nodes %lu nps %lu hashfull %lu\n", evaluatedNodes, nps, hashfull);
#endif
		prevTime = currTime;
	}
	if (canThrow && evaluatedNodes > 0 && !isInfinite) {
		if (isPonderThink) {
			// 先読み思考なら
			if (ponderhitReceiveTime != 0) {
				// ponderhitを受信していたら
				int thinkTime = currTime - ponderhitReceiveTime;
				if (thinkTime > thinkLimitTime) {
					throw 0; // 戻り先はSikou::ITDeep()
				}
			}
		} else {
			// 通常の思考なら
			int thinkTime = currTime - thinkStartTime;
			if (thinkTime > thinkLimitTime) {
				throw 0; // 戻り先はSikou::ITDeep()
			}
		}
	}
	++evaluatedNodes;

	// 終盤度を０〜１６の範囲に補正する。
	int Shuubando0,Shuubando1;

	if (Shuubando[0]<0) {
		Shuubando0=0;
	} else if (Shuubando[0]>16) {
		Shuubando0=16;
	} else {
		Shuubando0=Shuubando[0];
	}
	if (Shuubando[1]<0) {
		Shuubando1=0;
	} else if (Shuubando[1]>16) {
		Shuubando1=16;
	} else {
		Shuubando1=Shuubando[1];
	}

	// 終盤度の差を評価する。とりあえず、１違うと２００点違うことにする。
	int ret=(Shuubando1-Shuubando0)*200;

	ret+=SemegomaBonus[0]*Shuubando1/16;
	ret+=MamorigomaBonus[0]*Shuubando0/16;
	ret+=SemegomaBonus[1]*Shuubando0/16;
	ret+=MamorigomaBonus[1]*Shuubando1/16;

	ret+=KomagumiBonus[0]+KomagumiBonus[1];

//	printf("ret=%d\n",ret);
	//if (abs(ret)>10000) {
	//	Print();
	//	printf("ret=%d\n",ret);
	//}

	// 最後に、駒得の点数と合わせて評価値とする。
	return ret+value;
//	return value;
}

// 手番から見て敵の駒
int IsEnemy(int SorE,KomaInf koma)
{
	return koma!=WALL && !(SorE&koma);
}

// 手番から見て味方の駒
int IsSelf(int SorE,KomaInf koma)
{
	return koma!=WALL && (SorE&koma);
}

// 手の価値の比較（qsort用）大きい順に並ぶようにする。
int teValueComp(const void *p1,const void *p2)
{
	Te *te1=(Te *)p1;
	Te *te2=(Te *)p2;
	return te2->value-te1->value;
}

static const int Tbl[256]={
	0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
	4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};

// xは、何ビット１が立っているか
int bitnum(int x)
{
	return Tbl[((unsigned char*)&x)[0]]+Tbl[((unsigned char*)&x)[1]]+Tbl[((unsigned char*)&x)[2]]+Tbl[((unsigned char*)&x)[3]];
}


void KyokumenKomagumi::EvaluateTe(int SorE,int teNum,Te *te)
{
	int i;
	int nowEval=Evaluate();
//	Print();
	for(i=0;i<teNum;i++) {
		int LossS,LossE,GainS,GainE;
		LossS=LossE=GainS=GainE=0;
		KyokumenKomagumi _new(*this);
		// 手番から見て敵の玉
		KomaInf EnemyKing=SorE==SELF? EOU:SOU;
		// 手番から見て敵の利き
		unsigned int *_newControlE=SorE==SELF?_new.controlE:_new.controlS;
		// 手番から見て味方の利き
		unsigned int *_newControlS=SorE==SELF?_new.controlS:_new.controlE;
		// 手番から見て味方の利き
		unsigned int *_nowControlS=SorE==SELF?controlS:controlE;

		KomaInf NewKoma=te[i].promote? te[i].koma|PROMOTED:te[i].koma;
		// 実際に一手動かして、評価値の変動をみる。
		_new.Move(SorE,te[i]);
		te[i].value=_new.Evaluate()-nowEval;
		if (SorE==ENEMY) {
			// 敵の番の時には手の価値をひっくり返して置く
			te[i].value=-te[i].value;
		}
		if (te[i].from!=0) {
			// 駒が居た場所の脅威がなくなる
			LossS-=Eval(te[i].from);
		}
		// 新しく移動した先での脅威が加わる
		LossS+=_new.Eval(te[i].to);
		// 相手に与える脅威と、新しく自分の駒にヒモをつけることで、減る脅威を計算
		int dir;
		for(dir=0;dir<12;dir++) {
			if (CanMove[dir][NewKoma]) {
				int p=te[i].to+Direct[dir];
				if (_new.ban[p]!=EnemyKing) {
					// 玉以外の駒に対する脅威
					if (IsEnemy(SorE,_new.ban[p])) {
						LossE+=_new.Eval(p);
					} else if (IsSelf(SorE,_new.ban[p])) {
						GainS+=Eval(p)-_new.Eval(p);
					}
				} else {
					// 玉に与える脅威はそのまま計算すると大きすぎになるので調整する。
					if (_newControlE[te[i].to]) {
						if (_newControlS[te[i].to]) {
							LossE+=1000;
						} else {
							LossE+=500;
						}
					} else {
						LossE+=1500;
					}
				}
			}
		}
		// 移動したことで、別の駒の飛び利きを通す（かも知れない）
		if (te[i].from>OU) {
			for(dir=0;dir<8;dir++) {
				if ((_nowControlS[te[i].from]& (1<<(dir+16))) !=0) {
					int p=_new.search(te[i].from,Direct[dir]);
					if (_new.ban[p]!=WALL) {
						// 飛び利きの通った先での交換値の再計算
						if (IsEnemy(SorE,_new.ban[p])) {
							LossE+=_new.Eval(p);
						} else if (IsSelf(SorE,_new.ban[p])) {
							GainS+=Eval(p)-_new.Eval(p);
						}
					}
				}
			}
		}
		// 移動した駒自身の飛び利きによる脅威の増減を計算する
		for(dir=0;dir<8;dir++) {
			if (CanJump[dir][NewKoma]) {
				int p=_new.search(te[i].to,Direct[dir]);
				// 例によって、玉に対する脅威は大きく評価されすぎるので調整する。
				if (_new.ban[p]!=EnemyKing) {
					if (IsEnemy(SorE,_new.ban[p])) {
						LossE+=_new.Eval(p);
					} else if (IsSelf(SorE,_new.ban[p])) {
						GainS+=Eval(p)-_new.Eval(p);
					}
				} else {
					if (_newControlE[te[i].to]) {
						if (_newControlS[te[i].to]) {
							LossE+=1000;
						} else {
							LossE+=500;
						}
					} else {
						LossE+=1500;
					}
				}
			}
		}
		te[i].value+=GainS-LossS;
		if ((te[i].capture!=EMPTY && te[i].capture!=FU)) {
			// 歩以外の駒を取る手は無条件に1500点プラスして、読みに入れるようにする
			te[i].value+=1500;
		}
		if (te[i].from==FU && bitnum(_newControlE[0xac-te[i].to])>1) {
			// 焦点の歩…かも。
			te[i].value+=50;
		}
		// 攻撃の価値はこれくらいの値（１０分の１）を加えると、実験ではちょうど良い
		te[i].value+=LossE*1/10;

//		te[i].Print();
//		printf("val:%5d LossS:%5d LossE:%5d GainS:%5d GainE:%5d\n",te[i].value,LossS,LossE,GainS,GainE);
	}
//	char p[20];
//	gets(p);
	// 並び替え。
	qsort(te,teNum,sizeof(te[0]),teValueComp);
}

int Kyokumen::MakeChecks(int SorE,Te *teBuf,int *pin)
{
	Kyokumen kk(*this);			// 実際に動かしてみる局面
	unsigned int *selfControl;	// 手番側から見て、自分の利き
	int enemyKing;				// 手番側から見て、相手の玉の位置

	if (SorE==SELF) {
		selfControl=kk.controlS;
		enemyKing=kingE;
	} else {
		selfControl=kk.controlE;
		enemyKing=kingS;
	}
	int teNum=MakeLegalMoves(SorE,teBuf,pin);
	// 実際に動かしてみて、王手だけを残す。
	int outeNum=0;
	for(int i=0;i<teNum;i++) {
		kk=*this;
		kk.Move(SorE,teBuf[i]);
		// 相手の玉に利きが付いていれば王手。
		if (selfControl[enemyKing]) {
			teBuf[outeNum++]=teBuf[i];
		}
	}
	return outeNum;
}


TsumeVal TsumeHash::HashTbl[TSUME_HASH_SIZE];


uint64 TsumeHash::FU_BIT_TBL[19]={
	0x0000000000000000,
	0x0000000000000001,
	0x0000000000000003,
	0x0000000000000007,
	0x000000000000000f,
	0x000000000000001f,
	0x000000000000003f,
	0x000000000000007f,
	0x00000000000001ff,
	0x00000000000003ff,
	0x00000000000007ff,
	0x0000000000000fff,
	0x0000000000001fff,
	0x0000000000003fff,
	0x0000000000007fff,
	0x000000000000ffff,
	0x000000000001ffff,
	0x000000000003ffff,
	0x000000000007ffff
};

uint64 TsumeHash::KY_BIT_TBL[5]={
	0x0000000000000000,
	0x0000000000100000,
	0x0000000000300000,
	0x0000000000700000,
	0x0000000000f00000,
};

uint64 TsumeHash::KE_BIT_TBL[5]={
	0x0000000000000000,
	0x0000000001000000,
	0x0000000003000000,
	0x0000000007000000,
	0x000000000f000000,
};

uint64 TsumeHash::GI_BIT_TBL[5]={
	0x0000000000000000,
	0x0000000010000000,
	0x0000000030000000,
	0x0000000070000000,
	0x00000000f0000000,
};

uint64 TsumeHash::KI_BIT_TBL[5]={
	0x0000000000000000ULL,
	0x0000000100000000ULL,
	0x0000000300000000ULL,
	0x0000000700000000ULL,
	0x0000000f00000000ULL,
};
/*
//	これは間違い
uint64 TsumeHash::KA_BIT_TBL[3]={
	0x0000000000000000,
	0x0000000100000000,
	0x0000000300000000,
};

uint64 TsumeHash::HI_BIT_TBL[3]={
	0x0000000000000000,
	0x0000000400000000,
	0x0000000c00000000,
};
*/

// こちらが正しい
// http://lesserpyon.bbs.coocan.jp/?m=listthread&t_id=4により修正
uint64 TsumeHash::KA_BIT_TBL[3]={ 
	0x0000000000000000ULL,
	0x0000001000000000ULL,
	0x0000003000000000ULL,
};

uint64 TsumeHash::HI_BIT_TBL[3]={ 
	0x0000000000000000ULL,
	0x0000004000000000ULL,
	0x000000c000000000ULL,
};


uint64 TsumeHash::CalcHand(int Motigoma[])
{
	return 
		FU_BIT_TBL[Motigoma[FU]] |
		KY_BIT_TBL[Motigoma[KY]] |
		KE_BIT_TBL[Motigoma[KE]] |
		GI_BIT_TBL[Motigoma[GI]] |
		KI_BIT_TBL[Motigoma[KI]] |
		KA_BIT_TBL[Motigoma[KA]] |
		HI_BIT_TBL[Motigoma[HI]];
}

void TsumeHash::Add(uint64 KyokumenHashVal,uint64 HandHashVal,int Motigoma[],int mate,Te te)
{
	uint64 NowHashVal=KyokumenHashVal;
	uint64 Hand=CalcHand(Motigoma);
	int i;
	for(i=0;i<RETRY_MAX;i++) {
		if (HashTbl[NowHashVal&TSUME_HASH_AND].HashVal==0) {
			TsumeVal &t=HashTbl[NowHashVal&TSUME_HASH_AND];
			t.HashVal=KyokumenHashVal;
			t.Motigoma=Hand;
			t.mate=mate;
			t.NextEntry=0;
			t.te=te;
			break;
		} else if (HashTbl[NowHashVal&TSUME_HASH_AND].HashVal==KyokumenHashVal) {
			if (HashTbl[NowHashVal&TSUME_HASH_AND].Motigoma==Hand) {
				// 持ち駒も含めて同一局面
			} else {
				// 盤上が同一で、持ち駒が違うものが登録済み
				while(HashTbl[NowHashVal&TSUME_HASH_AND].NextEntry!=0) {
					NowHashVal=HashTbl[NowHashVal&TSUME_HASH_AND].NextEntry;
					if (HashTbl[NowHashVal&TSUME_HASH_AND].Motigoma==Hand) {
						// 持ち駒も含めて同一局面
						break;
					}
				}
			}
			if (HashTbl[NowHashVal&TSUME_HASH_AND].Motigoma==Hand) {
				TsumeVal &t=HashTbl[NowHashVal&TSUME_HASH_AND];
				t.HashVal=KyokumenHashVal;
				t.Motigoma=Hand;
				t.mate=mate;
				t.te=te;
				break;
			}
			TsumeVal &pre=HashTbl[NowHashVal&TSUME_HASH_AND];
			NowHashVal=KyokumenHashVal^HandHashVal;
			for (int j=0;j<RETRY_MAX;j++) {
				if (HashTbl[NowHashVal&TSUME_HASH_AND].HashVal==0) {
					TsumeVal &t=HashTbl[NowHashVal&TSUME_HASH_AND];
					t.HashVal=KyokumenHashVal;
					t.Motigoma=Hand;
					t.mate=mate;
					t.NextEntry=0;
					t.te=te;
					pre.NextEntry=NowHashVal&TSUME_HASH_AND;
					break;
				}
				NowHashVal+=TSUME_HASH_SIZE/11;
			}
			break;
		}
		NowHashVal+=TSUME_HASH_SIZE/11;
	}
	if (i==RETRY_MAX) {
		printf("OVER MAX\n");
	}
}

void TsumeHash::Clear()
{
	memset(HashTbl,0,sizeof(HashTbl));
}

TsumeVal* TsumeHash::FindFirst(uint64 KyokumenHashVal)
{
	uint64 NowHashVal=KyokumenHashVal;
	for(int i=0;i<RETRY_MAX;i++) {
		if (HashTbl[NowHashVal&TSUME_HASH_AND].HashVal==KyokumenHashVal) {
			return HashTbl+(NowHashVal&TSUME_HASH_AND);
		}
		NowHashVal+=TSUME_HASH_SIZE/11;
	}
	return NULL;
}

TsumeVal* TsumeHash::FindNext(TsumeVal *Now) 
{
	if (Now->NextEntry==0) return NULL;
	return HashTbl+Now->NextEntry;
}

TsumeVal *TsumeHash::Find(uint64 KyokumenHashVal,uint64 HandHashVal,int Motigoma[])
{
	uint64 CalcMotigoma=CalcHand(Motigoma);
	TsumeVal *ret=FindFirst(KyokumenHashVal);
	while(ret!=NULL) {
		if (ret->Motigoma==CalcMotigoma) return ret;
		ret=FindNext(ret);
	}
	return NULL;
}

// ハッシュに登録された局面で、持ち駒がより少ない局面で詰んでいる局面を探す
TsumeVal *TsumeHash::DomSearchCheckMate(uint64 KyokumenHashVal,int Motigoma[])
{
	uint64 CalcMotigoma=CalcHand(Motigoma);
	TsumeVal *ret=FindFirst(KyokumenHashVal);
	while(ret!=NULL) {
		if (ret->Motigoma==CalcMotigoma) {
			if (ret->mate!=1) {
				return NULL;
			}
		}
		// ハッシュに登録されているより持ち駒が多く、ハッシュで詰みなら詰み
		if ((ret->Motigoma & CalcMotigoma)==ret->Motigoma && ret->mate==1) {
			return ret;
		}
		ret=FindNext(ret);
	}
	return NULL;
}

extern bool isTsumeThink; // 詰将棋解答思考中か
extern bool isStopReceived; // stopコマンドを受信したか
extern unsigned long tsumeLimitTime; // 詰将棋解答の制限時間
unsigned long mateNodes; // 詰将棋解答で調べた局面数（CheckMate()とAntiCheckMate()が呼ばれた数にしているが、間違ってるかも。）

int Kyokumen::Mate(int SorE,int maxDepth,Te &te)
{
	Te teBuf[10000];	// 深さ30程度までなら十分すぎる大きさ

	TsumeVal *p;
	if ((p=TsumeHash::Find(KyokumenHashVal,HandHashVal,Hand+SorE))!=NULL) {
		if (p->mate==1) {
			te=p->te;
		}
		return p->mate;
	}
	mateNodes = 0;
	int ret = 0;
	for(int i=1;i<=maxDepth;i+=2) {
		if (isTsumeThink) {
			// 詰将棋解答の場合だけ表示する。（対局中に呼ばれた場合は表示しない。）
			printf("info depth %d\n",i);
		}
		ret = CheckMate(SorE,0,i,teBuf,te);
		if (ret != 0) {
			break;
		}
	}
	//  0:不明
	//  1:詰んだ
	// -1:不詰み
	return ret;
}

int Kyokumen::CheckMate(int SorE,int depth, int depthMax, Te *checks,Te &te)
{
	if (!isTsumeThink) {
		// 対局中ならここに入る。
		if (isStopReceived) {
			// 詰みを読んでいるときにstopが送られたらすぐに抜け出す。
			// （詰みを読むのはかなり長時間になることがある。）
			throw 0; // 戻り先はSikou::NegaAlphaBeta()
		}
		unsigned long diffTime = timeGetTime() - thinkStartTime;
		if (diffTime > 500) {
			throw 0; // 対局では、詰みを読むのは0.5秒までにする。
		}
	} else {
		// 詰将棋解答の場合だけここに入る。（対局中なら入らない。）
		if (!isInfinite) {
			unsigned long diffTime = timeGetTime() - thinkStartTime;
			if (diffTime > tsumeLimitTime) {
				throw 0; // 戻り先はLesserkai::main()の"go mate"のところ。
			}
		}
		if (mateNodes == 0) {
			prevTime = timeGetTime();
		}
		unsigned long currTime = timeGetTime();
		unsigned long diffTime = currTime - prevTime;
		if (diffTime > 1000) {
			// 探索局面数などの情報表示（とりあえず、CheckMate()とAntiCheckMate()が呼ばれた回数を探索局面数とする。）
			unsigned long nps = (uint64)(mateNodes * 1000) / (currTime - thinkStartTime); // evaluatedNodes * 1000がintの範囲を超えることがあるので、uint64でキャストする。
			printf("info nodes %lu nps %lu\n", mateNodes, nps);
			prevTime = currTime;
		}
		++mateNodes;
	}
	int teNum = MakeChecks(SorE,checks);
	if (teNum == 0) {
		TsumeHash::Add(KyokumenHashVal,HandHashVal,Hand+SorE,-1,0);
		return -1;	//詰まない
	}
	TsumeVal *p;
	if ((p=TsumeHash::DomSearchCheckMate(KyokumenHashVal,Hand+SorE))!=NULL) {
		te=p->te;
		return 1; //詰んだ
	}
	int valmax = -1;
	for (int i = 0; i < teNum; i++) {
		Kyokumen kk(*this);
		kk.Move(SorE,checks[i]);
		int val = kk.AntiCheckMate(SorE^0x30, depth+1, depthMax, checks+teNum);
		if (val > valmax) valmax = val;
		if (valmax == 1) {
			te=checks[i];
			break; //詰んだ
		}
	}
	if (valmax == 1) { //詰んだ
		TsumeHash::Add(KyokumenHashVal,HandHashVal,Hand+SorE,1,te);
	} else if (valmax == -1) { //本当に詰まなかった
		TsumeHash::Add(KyokumenHashVal,HandHashVal,Hand+SorE,-1,0);
	}
	return valmax;
}

int Kyokumen::AntiCheckMate(int SorE,int depth, int depthMax, Te *antichecks)
{
	if (isTsumeThink) {
		++mateNodes;
	}
	Te te;
	int teNum=MakeLegalMoves(SorE,antichecks);

	int i=0;
	int valmin = 1;
	if (teNum == 0) {
		return 1;
	}
	if (depth >= depthMax+1) return 0; //長さの限界詰みは不明

	for (i = 0; i < teNum; i++) {
		Kyokumen k(*this);
		k.Move(SorE,antichecks[i]);
		int val = k.CheckMate(SorE^0x30, depth+1, depthMax, antichecks+teNum,te);
		if (val < valmin) valmin = val;
		if (valmin == -1) break; // 詰まなかった
		if (valmin == 0) break; // 詰まなかった
	}
	return valmin;
}

// これは必ず詰みが見つかったあとで呼ぶこと。
Te Kyokumen::GetTsumeTe(int SorE)
{
	TsumeVal *p;
	if ((p=TsumeHash::Find(KyokumenHashVal,HandHashVal,Hand+SorE))!=NULL) {
		return p->te;
	}
	return Te(0); // 詰みが見つかったあとなら、ここに来ることはない。
}

bool Kyokumen::IsNyugyokuWin(int SorE)
{
	// 入玉勝ちかどうか、以下に基づいて判定する。
	// http://www.computer-shogi.org/protocol/tcp_ip_1on1_11.html
	if (SorE == SELF) {
		if (controlE[kingS] != 0) {
			return false; // 先手玉に王手がかかっていたら勝ちではない。
		}
		if (kingS % 0x10 > 3) {
			return false; // 玉が敵陣３段目以内になければ勝ちではない。
		}
		int komaCount = 0; // 敵陣３段目以内にある（玉を除く）駒の枚数。
		int komaPoint = 0; // 得点の対象となる駒の得点（大駒５点、小駒１点）。
		for (int suji = 1; suji <= 9; suji++) {
			for (int dan = 1; dan <= 3; dan++) {
				KomaInf koma = ban[suji * 0x10 + dan];
				if ((koma & SELF) != 0 && koma != SOU) {
					++komaCount;
					if (koma == SHI || koma == SKA || koma == SUM || koma == SRY) {
						komaPoint += 5;
					} else {
						komaPoint += 1;
					}
				}
			}
		}
		if (komaCount < 10) {
			return false; // 敵陣３段目以内にある駒が１０枚未満なら勝ちではない。
		}
		for (int i = SFU; i <= SHI; i++) {
			if (i == SHI || i == SKA) {
				komaPoint += Hand[i] * 5;
			} else {
				komaPoint += Hand[i];
			}
		}
		if (komaPoint < 28) {
			return false; // 先手の場合、得点が２８点未満なら勝ちではない。
		}
		return true; // 全ての条件を満たしていれば勝ち。
	} else {
		if (controlS[kingE] != 0) {
			return false; // 後手玉に王手がかかっていたら勝ちではない。
		}
		if (kingE % 0x10 < 7) {
			return false; // 玉が敵陣３段目以内になければ勝ちではない。
		}
		int komaCount = 0; // 敵陣３段目以内にある（玉を除く）駒の枚数。
		int komaPoint = 0; // 得点の対象となる駒の得点（大駒５点、小駒１点）。
		for (int suji = 1; suji <= 9; suji++) {
			for (int dan = 7; dan <= 9; dan++) {
				KomaInf koma = ban[suji * 0x10 + dan];
				if ((koma & ENEMY) != 0 && koma != EOU) {
					++komaCount;
					if (koma == EHI || koma == EKA || koma == EUM || koma == ERY) {
						komaPoint += 5;
					} else {
						komaPoint += 1;
					}
				}
			}
		}
		if (komaCount < 10) {
			return false; // 敵陣３段目以内にある駒が１０枚未満なら勝ちではない。
		}
		for (int i = EFU; i <= EHI; i++) {
			if (i == EHI || i == EKA) {
				komaPoint += Hand[i] * 5;
			} else {
				komaPoint += Hand[i];
			}
		}
		if (komaPoint < 27) {
			return false; // 後手の場合、得点が２７点未満なら勝ちではない。
		}
		return true; // 全ての条件を満たしていれば勝ち。
	}
}
