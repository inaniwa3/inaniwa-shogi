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
		// http://lesserpyon.bbs.coocan.jp/?m=listthread&t_id=3�ɂ��C��
		for(int maisuu=0;maisuu<=18;maisuu++) {
			HandHashSeed[i][maisuu]=(((uint64)rand())<<49)|
								 (((uint64)rand())<<34)|
								 (((uint64)rand())<<19)|
								 (((uint64)rand())<<4)|
								  (rand() & 0x07);
		}
	}
}



// �ǖʂ̃R���X�g���N�^�F�Ֆʂ̏�ԂƎ萔�A������琶��
Kyokumen::Kyokumen(int tesu,KomaInf board[9][9],int Motigoma[])
{
	// �Ֆʂ�WALL�i�ǁj�Ŗ��߂Ă����܂��B
	memset(banpadding,WALL,sizeof(banpadding));
	memset(ban,WALL,sizeof(ban));
	// ������
	value=0;
	kingS=0;
	kingE=0;
	Tesu=tesu;

	HashVal=0;
	HandHashVal=0;
	KyokumenHashVal=0;
	// board�ŗ^����ꂽ�ǖʂ�ݒ肵�܂��B
	for(int dan=1;dan<=9;dan++) {
		for(int suji=0x10;suji<=0x90;suji+=0x10) {
			// �����̋؂͍�����E�Ȃ̂ŁA�z��̐錾�Ƌt�ɂȂ邽�߁A�؂͂Ђ�����Ԃ��Ȃ��ƂȂ�܂���B
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
	// ������͂��̂܂ܗ��p���܂��B
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
	// controlS/controlE�����������܂��B
	InitControl();
}

// �ǖʂ̏������B�R���X�g���N�^�Ɠ������Ƃ���点��B�i�R���X�g���N�^�ȊO����ł��������ł���悤�ɁB�j
void Kyokumen::InitKyokumen(int tesu, KomaInf board[9][9], int Motigoma[])
{
	// �Ֆʂ�WALL�i�ǁj�Ŗ��߂Ă����܂��B
	memset(banpadding,WALL,sizeof(banpadding));
	memset(ban,WALL,sizeof(ban));
	// ������
	value=0;
	kingS=0;
	kingE=0;
	Tesu=tesu;

	HashVal=0;
	HandHashVal=0;
	KyokumenHashVal=0;
	// board�ŗ^����ꂽ�ǖʂ�ݒ肵�܂��B
	for(int dan=1;dan<=9;dan++) {
		for(int suji=0x10;suji<=0x90;suji+=0x10) {
			// �����̋؂͍�����E�Ȃ̂ŁA�z��̐錾�Ƌt�ɂȂ邽�߁A�؂͂Ђ�����Ԃ��Ȃ��ƂȂ�܂���B
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
	// ������͂��̂܂ܗ��p���܂��B
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
	// controlS/controlE�����������܂��B
	InitControl();
}

// controlS,controlE�̏�����
void Kyokumen::InitControl()
{
	int dan, suji;
	int i, j, b, bj;

	memset(controlS,0,sizeof(controlS));
	memset(controlE,0,sizeof(controlE));

	for (suji = 0x10; suji <= 0x90; suji += 0x10) {
		for (dan = 1 ; dan <= 9 ; dan++) {
			if (ban[suji + dan] & ENEMY) { //�G�̋�
				//��̌�����ǉ�����
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
			} else if (ban[suji + dan] & SELF) { //�����̋�L��
				//��̌�����ǉ�����
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

// ��ŋǖʂ�i�߂�
void Kyokumen::Move(int SorE,const Te &te)
{
	int i,j,b,bj;
	if (te.from>0x10) {
		// ��������̃R���g���[��������
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
		// �������ʒu�͋󔒂ɂȂ�
		ban[te.from]=EMPTY;
		// �󔒂ɂȂ������Ƃŕς��n�b�V���l
		KyokumenHashVal^=HashSeed[te.koma][te.from];
		KyokumenHashVal^=HashSeed[EMPTY][te.from];
		// ��ї�����L�΂�
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
		// �������ꖇ���炷
		HandHashVal^=HandHashSeed[te.koma][Hand[te.koma]];
		Hand[te.koma]--;
		value-=HandValue[te.koma];
		value+=KomaValue[te.koma];
	}
	if (ban[te.to]!=EMPTY) {
		// ����̋��������ɂ���B
		// ������ɂ��鎞�́A�����Ă������s����ɖ߂��B�i&~PROMOTED�j
		value-=KomaValue[ban[te.to]];
		value+=HandValue[SorE|(ban[te.to]&~PROMOTED&~SELF&~ENEMY)];
		int koma=SorE|(ban[te.to]&~PROMOTED&~SELF&~ENEMY);
		Hand[koma]++;
		// �n�b�V���Ɏ�������������
		HandHashVal^=HandHashSeed[koma][Hand[koma]];
		//�������̌���������
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
		// �ړ���ŎՂ�����ї���������
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
	// ban[te.to]�ɂ��������̂��g�������������
	KyokumenHashVal^=HashSeed[ban[te.to]][te.to];
	if (te.promote) {
		value-=KomaValue[te.koma];
		value+=KomaValue[te.koma|PROMOTED];
		ban[te.to]=te.koma|PROMOTED;
	} else {
		ban[te.to]=te.koma;
	}
	// �V��������g�������ɉ�����
	KyokumenHashVal^=HashSeed[ban[te.to]][te.to];
	// �ړ���̗���������
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
	// ���l�̈ʒu�͊o���Ă����B
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

// �s���i�������Ɖ�������Ă��܂��̂œ���������������j�̏�Ԃ�ݒ肷��
void Kyokumen::MakePinInf(int *pin) const
{
	int i;
	// �s������ݒ肷��
	for (i = 0x11; i <= 0x99; i++) {
		// 0�̓s������Ă��Ȃ��A�Ƃ����Ӗ�
		pin[i] = 0;
	}
	if (kingS) {	//���ʂ��Ֆʂɂ��鎞�̂ݗL��
		for (i = 0; i < 8; i++) {
			int p;
			p = search(kingS, -Direct[i]); 
			if ((ban[p] != WALL) && !(ban[p] & ENEMY)) { //�����̋�L��
				if (controlE[p]&(1<<(16+i))) {
					pin[p]=Direct[i];
				}
			}
		}
	}
	if (kingE) {	//�G�ʂ��Ֆʂɂ��鎞�̂ݗL��
		for (i = 0; i < 8; i++) {
			int p;
			p = search(kingE, -Direct[i]);
			if ((ban[p] != WALL) && (ban[p] & ENEMY)) { //�G�̋�L��
				if (controlS[p]&(1<<(16+i))) {
					pin[p]=Direct[i];
				}
			}
		}
	}
}

// ��̓����Ƃ��Đ�����������S�Đ�������B
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
		// ���ʂ����邩�ǂ����Ƃ���������ǉ��B�i�l�����ȂǂŐ��ʂ��Ȃ��Ȃ����Ȃ��j
		return AntiCheck(SorE,teBuf,pin,controlE[kingS]);
	}
	if (SorE==ENEMY && kingE != 0 && controlS[kingE]!=0) {
		// ���ʂ����邩�ǂ����Ƃ���������ǉ��B�i�l�����ȂǂŌ��ʂ��Ȃ��Ȃ����Ȃ��j
		return AntiCheck(SorE,teBuf,pin,controlS[kingE]);
	}

	int suji,dan;
	int StartDan,EndDan;
	// �Տ�̋�𓮂���
	for(suji=0x10;suji<=0x90;suji+=0x10) {
		for(dan=1;dan<=9;dan++) {
			if (ban[suji+dan]&SorE) {
				AddMoves(SorE,teNum,teBuf,suji+dan,pin[suji+dan]);
			}
		}
	}
	// ����ł�
	if (Hand[SorE|FU]>0) {
		for(suji=0x10;suji<=0x90;suji+=0x10) {
			// ����`�F�b�N
			int nifu=0;
			for(dan=1;dan<=9;dan++) {
				if (ban[suji+dan]==(SorE|FU)) {
					nifu=true;
					break;
				}
			}
			if (nifu) continue;
			//(���Ȃ�Q�i�ڂ�艺�ɁA���Ȃ�W�i�ڂ���ɑłj
			if (SorE==SELF) {
				StartDan=2;
				EndDan=9;
			} else {
				StartDan=1;
				EndDan=8;
			}
			for(dan=StartDan;dan<=EndDan;dan++) {
				// �ł����l�߂��`�F�b�N
				if (ban[dan+suji]==EMPTY && !Utifudume(SorE,dan+suji,pin)) {
					teBuf[teNum++]=Te(0,suji+dan,SorE|FU,EMPTY);
				}
			}
		}
	}
	// ����ł�
	if (Hand[SorE|KY]>0) {
		for(suji=0x10;suji<=0x90;suji+=0x10) {
			//(���Ȃ�Q�i�ڂ�艺�ɁA���Ȃ�W�i�ڂ���ɑłj
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
	//�j��ł�
	if (Hand[SorE|KE]>0) {
		//(���Ȃ�R�i�ڂ�艺�ɁA���Ȃ�V�i�ڂ���ɑłj
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
	// ��`��Ԃ́A�ǂ��ɂł��łĂ�
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

// �Ֆʂ�from�ɂ����𓮂�����𐶐�����B
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
		MoveKing(SorE,teNum,teTop,0);	// ���肪�������Ă��鎞�ɂ́AAntiCheck�̕����Ă΂�邩��AKiki�͂O�ł��B
	}
}

// ����ꏊ�̗��������쐬���ĕԂ��B���i�͎g��Ȃ��֐��i�����v�Z���Ă��邩��j�����A
// �ł����l�߂̃`�F�b�N�ȂǁA������ɒu���Ă݂ĉ�������悤�ȂƂ��Ɏg�p����B
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

// ����ꏊ�̗��������쐬���ĕԂ��B���i�͎g��Ȃ��֐��i�����v�Z���Ă��邩��j�����A
// �ł����l�߂̃`�F�b�N�ȂǁA������ɒu���Ă݂ĉ�������悤�ȂƂ��Ɏg�p����B
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

// ����ꏊ�Ɉړ��ł�����S���W�߂āAKiki���ɂ��ĕԂ��B
// ���̂Ƃ��Apin����Ă�����pin�̕����ɂ��������Ȃ��B
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

// �ł����l�߂̔���
int Kyokumen::Utifudume(int SorE,int to,int *pin)
{
	if (SorE==SELF) {
		// �܂��A�ʂ̓��ɕ���ł肶��Ȃ���Αł����l�߂̐S�z�͂Ȃ��B
		if (kingE+1!=to) {
			return 0;
		}
	} else {
		// �܂��A�ʂ̓��ɕ���ł肶��Ȃ���Αł����l�߂̐S�z�͂Ȃ��B
		if (kingS-1!=to) {
			return 0;
		}
	}
	//���ۂɕ���ł��Ċm���߂Ă݂�B
	ban[to]=FU|SorE;
	if (SorE==SELF) {
		// �����̗������������瑊��͋ʂŎ��Ȃ��@���@��铮����񋓂��Ă݂���ʂŎ��肵���Ȃ�
		if (controlS[to] && (CountMove(ENEMY,to,pin)==1<<1)) {
			// �ʂɓ����������邩�ǂ������`�F�b�N
			for(int i=0;i<8;i++) {
				KomaInf koma=ban[kingE+Direct[i]];
				if (!(koma & ENEMY) && !CountControlS(kingE+Direct[i])) {
					// ���������������̂ŁA�Ֆʂ����̏�Ԃɖ߂��āA
					ban[to]=EMPTY;
					// �ł����l�߂ł͂Ȃ������B
					return 0;
				}
			}
			// �ʂ̓��������Ȃ��̂Ȃ�A�ł����l�߁B�Ֆʂ̏�Ԃ͌��ɖ߂��B
			ban[to]=EMPTY;
			return 1;
		}
		// �ʈȊO�Ŏ���肪���邩�A�ʂŎ���B
		ban[to]=EMPTY;
		return 0;
	} else {
		// �����̗������������瑊��͋ʂŎ��Ȃ��@���@��铮����񋓂��Ă݂���ʂŎ��肵���Ȃ�
		if (controlE[to] && (CountMove(SELF,to,pin)==1<<6)) {
			// �ʂɓ����������邩�ǂ������`�F�b�N
			for(int i=0;i<8;i++) {
				KomaInf koma=ban[kingS+Direct[i]];
				if (!(koma & SELF) && !CountControlE(kingS+Direct[i])) {
					// ���������������̂ŁA�Ֆʂ����̏�Ԃɖ߂��āA
					ban[to]=EMPTY;
					// �ł����l�߂ł͂Ȃ������B
					return 0;
				}
			}
			// �ʂ̓��������Ȃ��̂Ȃ�A�ł����l�߁B�Ֆʂ̏�Ԃ͌��ɖ߂��B
			ban[to]=EMPTY;
			return 1;
		}
		// �ʈȊO�Ŏ���肪���邩�A�ʂŎ���B
		ban[to]=EMPTY;
		return 0;
	}
}

// ����ꏊ�ito�j�ɋ��ł�̐���
void Kyokumen::PutTo(int SorE,int &teNum,Te *teTop,int to,int *pin)
{
	int dan=to &0x0f;
	if (SorE==ENEMY) {
		dan=10-dan;
	}
	if (Hand[SorE|FU]>0 && dan>1) {
		// ����ł�𐶐�
		// ����`�F�b�N
		int suji=to & 0xf0;
		int nifu=0;
		for(int d=1;d<=9;d++) {
			if (ban[suji+d]==(SorE|FU)) {
				nifu=1;
				break;
			}
		}
		// �ł����l�߂��`�F�b�N
		if (!nifu && !Utifudume(SorE,to,pin)) {
			teTop[teNum++]=Te(0,to,SorE|FU,EMPTY);
		}
	}
	if (Hand[SorE|KY]>0 && dan>1) {
		// ����ł�𐶐�
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

// ������󂯂��̐���
int Kyokumen::AntiCheck(int SorE,Te *teBuf,int *pin,Kiki kiki)
{
	int king;
	int teNum=0;
	if ((kiki & (kiki-1))!=0) {
        //������͋ʂ𓮂��������Ȃ�
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
        //���������
        MoveTo(SorE,teNum, teBuf, check, pin);
		
        //�ʂ𓮂���
        MoveKing(SorE,teNum, teBuf, kiki);

		if (id >= 16) {
            //����������𐶐�����
            int i;
            for (i = king - Direct[id-16]; ban[i] == EMPTY; i -= Direct[id-16]) {
				MoveTo(SorE,teNum, teBuf, i, pin); //�ړ���
            }
            for (i = king - Direct[id-16]; ban[i] == EMPTY; i -= Direct[id-16]) {
				PutTo(SorE,teNum, teBuf, i, pin);  //���ł�
            }
        } 
	}
	return teNum;
}

// �ʂ𓮂�����̐���
// ���ʂ̋�ƈႢ�A����̗����̂���Ƃ���ɂ͓����Ȃ��̂ŁA���̂��߂̓���ȏ��������Ă��܂��B
void Kyokumen::MoveKing(int SorE,int &teNum,Te *teTop,Kiki kiki)
{
	int i;
	int id = -1;	//�אډ����̈ʒu��id
	// ������łȂ��Ȃ牤���̈ʒu��T��
	for (i = 0; i < 8; i++) {
		if (kiki & (1 << i)) {
			id = i;
			break;
		}
	}
	if (id >= 0) {
		// �אڂ̉��� �ŏ��Ɏ���𐶐�����̂�
		if (SorE==SELF) {
			KomaInf koma=ban[kingS-Direct[id]];
			if (( koma==EMPTY || (koma & ENEMY))
				&& !CountControlE(kingS - Direct[id]) //�G�̋�����Ă��Ȃ�
				&& !(kiki & (1 << (23-id))))  //�G�̔��Ŋт���Ă��Ȃ�
			AddMove(SorE,teNum, teTop, kingS, -Direct[id], 0);
		} else {
			KomaInf koma=ban[kingE-Direct[id]];
			if (( koma==EMPTY || (koma & SELF))
				&& !CountControlS(kingE - Direct[id]) //�G�̋�����Ă��Ȃ�
				&& !(kiki & (1 << (23-id))))  //�G�̔��Ŋт���Ă��Ȃ�
			AddMove(SorE,teNum, teTop, kingE, -Direct[id], 0);
		}
	}
	for (i = 0; i < 8; i++) {
		if (i == id) continue;
		if (SorE==SELF) {
			KomaInf koma=ban[kingS-Direct[i]];
			if (( koma==EMPTY || (koma & ENEMY))
				&& !CountControlE(kingS - Direct[i]) //�G�̋�����Ă��Ȃ�
				&& !(kiki & (1 << (23-i))))  //�G�̔��Ŋт���Ă��Ȃ�
			AddMove(SorE,teNum, teTop, kingS, -Direct[i], 0);
		} else {
			KomaInf koma=ban[kingE-Direct[i]];
			if (( koma==EMPTY || (koma & SELF))
				&& !CountControlS(kingE - Direct[i]) //�G�̋�����Ă��Ȃ�
				&& !(kiki & (1 << (23-i))))  //�G�̔��Ŋт���Ă��Ȃ�
			AddMove(SorE,teNum, teTop, kingE, -Direct[i], 0);
		}
	}
}

// ��̐����F����E�s������ӎ����āA��̓�����𐶐�����B
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
			// �K������
			teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
		} else if ((ban[from]==SFU || ban[from]==SKY) && dan<=1) {
			// �K������
			teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
		} else if (ban[from]==EKE && dan>=8) {
			// �K������
			teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
		} else if ((ban[from]==EFU || ban[from]==EKY) && dan>=9) {
			// �K������
			teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
		} else {
			if (SorE==SELF && (fromDan<=3 || dan <=3) && CanPromote[ban[from]]) {
				teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
			} else if (SorE==ENEMY && (fromDan>=7 || dan>=7) && CanPromote[ban[from]]){
				teTop[teNum++]=Te(from,to,ban[from],ban[to],1);
			}
			// ����Ȃ������������B
			teTop[teNum++]=Te(from,to,ban[from],ban[to],0);
		}
	}
}

// ��Ԋp���Ԃ��܂������ɐi�ގ�̐���
void Kyokumen::AddStraight(int SorE,int &teNum,Te *teTop,int from,int dir,int pin,int Rpin)
{
	if (dir==Rpin || dir==-Rpin) {
		return;
	}
	int i;
	if (pin==0 || pin==dir || pin==-dir) {
		// �󔒂̊ԁA������𐶐�����
		for(i=dir;ban[from+i]==EMPTY;i+=dir) {
			AddMove(SorE,teNum,teTop,from,i,0);
		}
		// �����̋�łȂ��Ȃ�A�����֓���
		if (!(ban[from+i] & SorE)) {
			AddMove(SorE,teNum,teTop,from,i,0);
		}
	}
}

//to�ɓ�����̐���
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

// ������ۂ��\������B
void Kyokumen::FPrint(FILE *fp)
{
	int x,y;
	y = 0;
	fprintf(fp,"Hash:%016llu Hand:%016llu Kyokumen:%016llu\n",HashVal,HandHashVal,KyokumenHashVal);

	//fprintf(fp,"������F");
	fprintf(fp,"Mochigoma:");
	for (x = EHI; x >=EFU; x--) {
		if (Hand[x] > 1) {
			y = 1;
			//fprintf(fp,"%s%2.2s", komaStr2[x], "���O�l�ܘZ������101112131415161718"+2*Hand[x]-2);
			fprintf(fp,"%s%2.2s", komaStr2[x], "010203040506070809101112131415161718"+2*Hand[x]-2);
		} else if (Hand[x] == 1) {
			y = 1;
			fprintf(fp,"%s", komaStr2[x]);
		}
	}
	if (y) {
		fprintf(fp,"\n");
	} else {
		//fprintf(fp,"�Ȃ�\n");
		fprintf(fp,"Nothing\n");
	}
	//fprintf(fp,"  �X �W �V �U �T �S �R �Q �P \n");
	fprintf(fp,"  09 08 07 06 05 04 03 02 01 \n");
	fprintf(fp,"+---------------------------+\n");
	for(y=1;y<=9;y++) {
		fprintf(fp,"|");
		for(x=9;x>=1;x--) {
			fprintf(fp,"%s", komaStr[ban[x*16+y]]);
		}
		//fprintf(fp,"|%2.2s","���O�l�ܘZ������" + y*2-2);
		fprintf(fp,"|%2.2s","010203040506070809" + y*2-2);
		fprintf(fp,"\n");
	}
	fprintf(fp,"+---------------------------+\n");
	//fprintf(fp,"������F");
	fprintf(fp,"Mochigoma:");
	y = 0;
	for (x = SHI; x >= SFU; x--) {
		if (Hand[x] > 1) {
			y = 1;
			//fprintf(fp,"%s%2.2s", komaStr2[x], "���O�l�ܘZ������101112131415161718"+2*Hand[x]-2);
			fprintf(fp,"%s%2.2s", komaStr2[x], "010203040506070809101112131415161718"+2*Hand[x]-2);
		} else if (Hand[x] == 1) {
			y = 1;
			fprintf(fp,"%s", komaStr2[x]);
		}
	}
	if (y) {
		fprintf(fp,"\n");
	} else {
		//fprintf(fp,"�Ȃ�\n");
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

// ��R�͂Œǉ��B�����l�����߂�B
int kyori(int p1,int p2)
{
	return max(abs(p1/16-p2/16),abs((p1 & 0x0f)-(p2 &0x0f)));
}

int Kyokumen::IsCorrectMove(Te &te)
{
	// ��5�͂Œǉ��B��ł��̏ꍇ�ɁA�������肩�ǂ����`�F�b�N����B
	if (te.from==0) {
		if (ban[te.to]!=EMPTY) return 0;
		if (te.koma==SFU) {
			// ����Ƒł����l�߂̃`�F�b�N
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
			// ����Ƒł����l�߂̃`�F�b�N
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
	// �W�����v�Ȃ̂ŁA�r���Ɏז��ȋ���Ȃ����ǂ����`�F�b�N����
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
		// �����̋�łȂ���𓮂����Ă���
		return 0;
	}
	if (te.from<OU) {
		if (Hand[te.koma]==0) return 0;
	} else {
		if (ban[te.from]!=te.koma) return 0;
	}
	if (ban[te.to] & SorE) {
		// �����̋������Ă���
		return 0;
	}
	if (IsCorrectMove(te)) {
		// ���ʂɉ���������Ă��Ȃ����A���ۂɓ������Ē��ׂ�
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
			// ���Ɏ肪�Ȃ������Ȃ��B
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
		// �ז���̏���
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
			// ���Ɏ肪�Ȃ������Ȃ��B
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
		// �����S�z���Ȃ�
		return 0;
	}
	if ((ban[position]&ENEMY) && !controlS[position]) {
		// �����S�z���Ȃ�
		return 0;
	}

	int ret;
	Te teTop[40];
	int ToPos=position;

	// AtackCount�𓾂�悤�ɁA��̃��X�g�𓾂�
	Te *AtackS=teTop;
	// ����ւ̗����́A�ő�א�8+�j�n2+���p�p��������=18������B
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

	// �j�n�̗����͕ʂɐ�����
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
		// http://lesserpyon.bbs.coocan.jp/?m=listthread&t_id=5�ɂ��C��
		if (ban[pos-Direct[i]]!=SOU && ban[pos-Direct[i]]!=EOU) {
			while((controlS[pos2] & bj) || (controlE[pos2] & bj)) {
				pos2-=Direct[i];
				while(ban[pos2]==EMPTY) {
					pos2-=Direct[i];
				}
//				if (ban[pos2]==WALL) continue;//�s�v�Ȃ͂��Ȃ̂�
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
	// �j�n�̗���
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
	// ��̉��l�Ń\�[�g�B
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
		//�ő�l�Ƃ̌���
		if (i!=max_id) {
			swap(AtackS[i],AtackS[max_id]);
		}
	}
	// ��̉��l�Ń\�[�g�B
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
		//�ő�l�Ƃ̌���
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
	// SorE�̗����̂���G�̋�i&SorE==0�̋�j�ɂ��āAEval���Ăяo���āA
	// ��Ԃ��������l��T���B
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
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{0,0,0,0,0,0,0,0,0,0},
//��
	{ 0,  0,15,15,15,3,1, 0, 0, 0},
//��
	{ 0, 1,2,3,4,5,6,7,8,9},
//�j
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//�p
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0,10,10,10, 0, 0, 0,  -5, 0, 0},
//��
	{ 0,1200,1200,900,600,300,-10,0,0,0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//����
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//���j
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//����
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//�n
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, -1, -3,-15,-15,-15, 0},
//��
	{ 0,-9,-8,-7, -6, -5, -4, -3, -2,-1},
//�j
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//�p
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 5, 0, 0, 0,-10,-10,-10},
//��
	{ 0, 0, 0, 0,10,-300,-600,-900,-1200,-1200},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//����
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//���j
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//����
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//�n
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//��
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

enum {
	IvsFURI,		// ����ԑΐU����
	IvsNAKA,		// ����ԑΒ����
	FURIvsFURI,		// ���U����
	FURIvsI,		// �U���ԑ΋����
	NAKAvsI,		// ����ԑ΋����
	KAKUGAWARI,		// �p����
	AIGAKARI,		// ���|����i�܂��͋���Ԃ̑΍R�n�j
	HUMEI			// ��`�s��
};

int JosekiKomagumiSGI[HUMEI+1][9][9]=
{
	{	// IvsFURI �M�͂��A���Z�A�⊥
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10, -7,-10,-10,-10,-10,-10,  7,-10},
		{-10,  7, -8, -7, 10,-10, 10,  6,-10},
		{-10, -2, -6, -5,-10,  6,-10,-10,-10},
		{-10, -7,  0,-10,-10,-10,-10,-10,-10}
	},{	// IvsNAKA�@�M�͂�
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10, -7,-10,-10, -7,-10,-10,  7,-10},
		{-10, -5, -8, -7, 10,-10, 10,  6,-10},
		{-10, -2, -3,  0,-10,  6,-10,-10,-10},
		{-10, -7, -5,-10,-10,-10,-10,-10,-10}
	},{ // FURIvsFURI�@��q�i�t�j�A���Z�A�⊥
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10, -7, -7,-10},
		{-10,-10,-10,-10,-10,  5, 10, 10,-10},
		{-10,-10,-10,-10,-10,-10,  0,-10,-10},
		{-10,-10,-10,-10,-10,-10, -5,-10,-10}
	},{ // FURIvsI ���Z�͂��A�⊥
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10, -3, -7,-10,-10,-10,-10,-10},
		{-10, -7,  4,  6,-10,-10,-10,  6,-10},
		{-10,  2,  3,  3,-10,-10,  4,-10,-10},
		{-10,-10,-10,  0,-10,-10,  0,-10,-10}
	},{ // NAKAvsI �����
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
					  //������j����p�򉤂ƈǌ\�S���n��
int ShuubandoByAtack[]={0,1,1,2,3,3,3,4,4,3,3,3,3,3,4,5};
						//������j�� ���p�� �� �� �\ �S �� �n ��
int ShuubandoByDefence[]={0,0,0,0,-1,-1,0,0,0,-1,-1,-1,-1,-1,-2,0};
					//������j����p�򉤂ƈǌ\�S���n��
int ShuubandoByHand[]={0,0,1,1,2,2,2,3,0,0,0,0,0,0,0,0};

void KyokumenKomagumi::InitShuubando()
{
	// �I�Փx�����߂�Ɠ����ɁA�I�Փx�ɂ��{�[�i�X�̕t���A��̉��_���s���B
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
//  ����������������������j����p�򉤂ƈǌ\�S���n��
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,0,0,
//	������j����p�򉤂ƈǌ\�S���n���ǋ���������������
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
	// http://lesserpyon.bbs.coocan.jp/?m=listthread&t_id=5�ɂ��C��
	if (te.koma==SOU || te.koma==EOU) {
		// ��𓮂�������ŁA�S�ʓI��Bonus�̌v�Z���Ȃ������K�v�B
		// Semegoma,Mamorigoma�ȂǑS�ĕς��̂ŁB
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
		// �S�i�ڈȉ��E�I�Փx�̌v�Z
		if (SorE==SELF) {
			Shuubando[1]-=ShuubandoByAtack[te.koma & ~SELF];
		} else {
			Shuubando[1]-=ShuubandoByDefence[te.koma & ~ENEMY];
		}
	}
	if (te.from>0 && (te.from&0x0f)>=6) {
		// �U�i�ڈȏ�E�I�Փx�̌v�Z
		if (SorE==SELF) {
			Shuubando[0]-=ShuubandoByDefence[te.koma & ~SELF];
		} else {
			Shuubando[0]-=ShuubandoByAtack[te.koma & ~ENEMY];
		}
	}
	if (te.capture) {
		if ((te.to&0x0f)<=4) {
			// �S�i�ڈȉ��E�I�Փx�̌v�Z
			if (SorE==SELF) {
				Shuubando[1]-=ShuubandoByDefence[te.capture & ~ENEMY];
			}
		}
		if ((te.to&0x0f)>=6) {
			// �U�i�ڈȏ�E�I�Փx�̌v�Z
			if (SorE==SELF) {
				Shuubando[0]-=ShuubandoByDefence[te.capture & ~SELF];
			}
		}
		// Hand�ɓ��������Ƃɂ��I�Փx�̌v�Z
		KomagumiBonus[enemy]-=KomagumiValue[te.capture][te.to];
	}
	if (!te.promote) {
		if ((te.to&0x0f)<=4) {
			// �S�i�ڈȉ��E�I�Փx�̌v�Z
			if (SorE==SELF) {
				Shuubando[1]+=ShuubandoByAtack[te.koma & ~SELF];
			} else {
				Shuubando[1]+=ShuubandoByDefence[te.koma & ~ENEMY];
			}
		}
		if ((te.to&0x0f)>=6) {
			// �U�i�ڈȏ�E�I�Փx�̌v�Z
			if (SorE==SELF) {
				Shuubando[0]+=ShuubandoByDefence[te.koma & ~SELF];
			} else {
				Shuubando[0]+=ShuubandoByAtack[te.koma & ~ENEMY];
			}
		}
		KomagumiBonus[self]+=KomagumiValue[te.koma][te.to];
	} else {
		if ((te.to&0x0f)<=4) {
			// �S�i�ڈȉ��E�I�Փx�̌v�Z
			if (SorE==SELF) {
				Shuubando[1]+=ShuubandoByAtack[(te.koma|PROMOTED) & ~SELF];
			} else {
				Shuubando[1]+=ShuubandoByDefence[(te.koma|PROMOTED) & ~ENEMY];
			}
		}
		if ((te.to&0x0f)>=6) {
			// �U�i�ڈȏ�E�I�Փx�̌v�Z
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
		// �S�ʓI�ɋ����Bonus�̌v�Z���Ȃ����B
		InitBonus();
	}
}

extern bool isPonderhitReceived; // ponderhit�R�}���h����M������
extern bool canThrow; // �v�l���f���\���ǂ���
extern bool isPonderThink; // ��ǂݎv�l����
extern unsigned long thinkStartTime; // �v�l���J�n��������
extern unsigned long ponderhitReceiveTime; // ponderhit����M��������
extern unsigned long evaluatedNodes; // KyokumenKomagumi::Evaluate()���Ă΂ꂽ��
extern unsigned long hashCount; // �n�b�V���ɒǉ����ꂽ��
extern unsigned long remainTime; // �c�莞��
extern unsigned long byoyomiTime; // �b�ǂ݂̎���
extern bool isInfinite; // �v�l���Ԃ����������ǂ���

unsigned long prevTime; // �O��Ainfo�R�}���h��hashfull��Ԃ�������
int thinkLimitTime; // �v�l�̐�������

#define SHOWHASH 1

int KyokumenKomagumi::Evaluate()
{
	if (evaluatedNodes == 0) {
		prevTime = timeGetTime();
		if (byoyomiTime == 0) { // �b�ǂ݂��Ȃ��ꍇ
			if (remainTime > 60 * 1000) {
				// 1���ȏ㎞�Ԃ�����Ȃ�A10�b�܂ōl���Ă��������Ƃɂ���B
				thinkLimitTime = 10 * 1000;
			} else if (remainTime > 30 * 1000) {
				// 30�b�ȏ�1���ȓ��̎��Ԃ�����Ȃ�A3�b�܂ōl���Ă��������Ƃɂ���B
				thinkLimitTime = 3 * 1000;
			} else {
				// �����łȂ����1�b�܂łƂ���B
				thinkLimitTime = 1000;
			}
		} else if (remainTime == 0) { // �������Ԃ��g���؂��ĕb�ǂ݂������c���Ă���ꍇ
			// �b�ǂ݂����ς��܂ōl����Ƃ���B
			thinkLimitTime = byoyomiTime;
		} else { // �������Ԃ��c���Ă��ĕb�ǂ݂�����ꍇ
			// 10�b�A�������́i�c�莞�ԁ{�b�ǂ݁j�̏��Ȃ����Ƃ���B
			thinkLimitTime = min((unsigned long)(10 * 1000), remainTime + byoyomiTime);
		}
	}
	unsigned long currTime = timeGetTime();
	unsigned long diffTime = currTime - prevTime;
	if (diffTime >= 1000) {
		unsigned long nps = (uint64)(evaluatedNodes * 1000) / (currTime - thinkStartTime); // evaluatedNodes * 1000��int�͈̔͂𒴂��邱�Ƃ�����̂ŁAuint64�ŃL���X�g����B
		unsigned long hashfull = hashCount * 1000 / (1024 * 1024);
#if SHOWHASH
		printf("info nodes %lu nps %lu hashfull %lu\n", evaluatedNodes, nps, hashfull);
#endif
		prevTime = currTime;
	}
	if (canThrow && evaluatedNodes > 0 && !isInfinite) {
		if (isPonderThink) {
			// ��ǂݎv�l�Ȃ�
			if (ponderhitReceiveTime != 0) {
				// ponderhit����M���Ă�����
				int thinkTime = currTime - ponderhitReceiveTime;
				if (thinkTime > thinkLimitTime) {
					throw 0; // �߂���Sikou::ITDeep()
				}
			}
		} else {
			// �ʏ�̎v�l�Ȃ�
			int thinkTime = currTime - thinkStartTime;
			if (thinkTime > thinkLimitTime) {
				throw 0; // �߂���Sikou::ITDeep()
			}
		}
	}
	++evaluatedNodes;

	// �I�Փx���O�`�P�U�͈̔͂ɕ␳����B
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

	// �I�Փx�̍���]������B�Ƃ肠�����A�P�Ⴄ�ƂQ�O�O�_�Ⴄ���Ƃɂ���B
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

	// �Ō�ɁA��̓_���ƍ��킹�ĕ]���l�Ƃ���B
	return ret+value;
//	return value;
}

// ��Ԃ��猩�ēG�̋�
int IsEnemy(int SorE,KomaInf koma)
{
	return koma!=WALL && !(SorE&koma);
}

// ��Ԃ��猩�Ė����̋�
int IsSelf(int SorE,KomaInf koma)
{
	return koma!=WALL && (SorE&koma);
}

// ��̉��l�̔�r�iqsort�p�j�傫�����ɕ��Ԃ悤�ɂ���B
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

// x�́A���r�b�g�P�������Ă��邩
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
		// ��Ԃ��猩�ēG�̋�
		KomaInf EnemyKing=SorE==SELF? EOU:SOU;
		// ��Ԃ��猩�ēG�̗���
		unsigned int *_newControlE=SorE==SELF?_new.controlE:_new.controlS;
		// ��Ԃ��猩�Ė����̗���
		unsigned int *_newControlS=SorE==SELF?_new.controlS:_new.controlE;
		// ��Ԃ��猩�Ė����̗���
		unsigned int *_nowControlS=SorE==SELF?controlS:controlE;

		KomaInf NewKoma=te[i].promote? te[i].koma|PROMOTED:te[i].koma;
		// ���ۂɈ�蓮�����āA�]���l�̕ϓ����݂�B
		_new.Move(SorE,te[i]);
		te[i].value=_new.Evaluate()-nowEval;
		if (SorE==ENEMY) {
			// �G�̔Ԃ̎��ɂ͎�̉��l���Ђ�����Ԃ��Ēu��
			te[i].value=-te[i].value;
		}
		if (te[i].from!=0) {
			// ������ꏊ�̋��Ђ��Ȃ��Ȃ�
			LossS-=Eval(te[i].from);
		}
		// �V�����ړ�������ł̋��Ђ������
		LossS+=_new.Eval(te[i].to);
		// ����ɗ^���鋺�ЂƁA�V���������̋�Ƀq�������邱�ƂŁA���鋺�Ђ��v�Z
		int dir;
		for(dir=0;dir<12;dir++) {
			if (CanMove[dir][NewKoma]) {
				int p=te[i].to+Direct[dir];
				if (_new.ban[p]!=EnemyKing) {
					// �ʈȊO�̋�ɑ΂��鋺��
					if (IsEnemy(SorE,_new.ban[p])) {
						LossE+=_new.Eval(p);
					} else if (IsSelf(SorE,_new.ban[p])) {
						GainS+=Eval(p)-_new.Eval(p);
					}
				} else {
					// �ʂɗ^���鋺�Ђ͂��̂܂܌v�Z����Ƒ傫�����ɂȂ�̂Œ�������B
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
		// �ړ��������ƂŁA�ʂ̋�̔�ї�����ʂ��i�����m��Ȃ��j
		if (te[i].from>OU) {
			for(dir=0;dir<8;dir++) {
				if ((_nowControlS[te[i].from]& (1<<(dir+16))) !=0) {
					int p=_new.search(te[i].from,Direct[dir]);
					if (_new.ban[p]!=WALL) {
						// ��ї����̒ʂ�����ł̌����l�̍Čv�Z
						if (IsEnemy(SorE,_new.ban[p])) {
							LossE+=_new.Eval(p);
						} else if (IsSelf(SorE,_new.ban[p])) {
							GainS+=Eval(p)-_new.Eval(p);
						}
					}
				}
			}
		}
		// �ړ�������g�̔�ї����ɂ�鋺�Ђ̑������v�Z����
		for(dir=0;dir<8;dir++) {
			if (CanJump[dir][NewKoma]) {
				int p=_new.search(te[i].to,Direct[dir]);
				// ��ɂ���āA�ʂɑ΂��鋺�Ђ͑傫���]�����ꂷ����̂Œ�������B
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
			// ���ȊO�̋������͖�������1500�_�v���X���āA�ǂ݂ɓ����悤�ɂ���
			te[i].value+=1500;
		}
		if (te[i].from==FU && bitnum(_newControlE[0xac-te[i].to])>1) {
			// �œ_�̕��c�����B
			te[i].value+=50;
		}
		// �U���̉��l�͂��ꂭ�炢�̒l�i�P�O���̂P�j��������ƁA�����ł͂��傤�Ǘǂ�
		te[i].value+=LossE*1/10;

//		te[i].Print();
//		printf("val:%5d LossS:%5d LossE:%5d GainS:%5d GainE:%5d\n",te[i].value,LossS,LossE,GainS,GainE);
	}
//	char p[20];
//	gets(p);
	// ���ёւ��B
	qsort(te,teNum,sizeof(te[0]),teValueComp);
}

int Kyokumen::MakeChecks(int SorE,Te *teBuf,int *pin)
{
	Kyokumen kk(*this);			// ���ۂɓ������Ă݂�ǖ�
	unsigned int *selfControl;	// ��ԑ����猩�āA�����̗���
	int enemyKing;				// ��ԑ����猩�āA����̋ʂ̈ʒu

	if (SorE==SELF) {
		selfControl=kk.controlS;
		enemyKing=kingE;
	} else {
		selfControl=kk.controlE;
		enemyKing=kingS;
	}
	int teNum=MakeLegalMoves(SorE,teBuf,pin);
	// ���ۂɓ������Ă݂āA���肾�����c���B
	int outeNum=0;
	for(int i=0;i<teNum;i++) {
		kk=*this;
		kk.Move(SorE,teBuf[i]);
		// ����̋ʂɗ������t���Ă���Ή���B
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
//	����͊ԈႢ
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

// �����炪������
// http://lesserpyon.bbs.coocan.jp/?m=listthread&t_id=4�ɂ��C��
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
				// ��������܂߂ē���ǖ�
			} else {
				// �Տオ����ŁA������Ⴄ���̂��o�^�ς�
				while(HashTbl[NowHashVal&TSUME_HASH_AND].NextEntry!=0) {
					NowHashVal=HashTbl[NowHashVal&TSUME_HASH_AND].NextEntry;
					if (HashTbl[NowHashVal&TSUME_HASH_AND].Motigoma==Hand) {
						// ��������܂߂ē���ǖ�
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

// �n�b�V���ɓo�^���ꂽ�ǖʂŁA�������菭�Ȃ��ǖʂŋl��ł���ǖʂ�T��
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
		// �n�b�V���ɓo�^����Ă����莝��������A�n�b�V���ŋl�݂Ȃ�l��
		if ((ret->Motigoma & CalcMotigoma)==ret->Motigoma && ret->mate==1) {
			return ret;
		}
		ret=FindNext(ret);
	}
	return NULL;
}

extern bool isTsumeThink; // �l�����𓚎v�l����
extern bool isStopReceived; // stop�R�}���h����M������
extern unsigned long tsumeLimitTime; // �l�����𓚂̐�������
unsigned long mateNodes; // �l�����𓚂Œ��ׂ��ǖʐ��iCheckMate()��AntiCheckMate()���Ă΂ꂽ���ɂ��Ă��邪�A�Ԉ���Ă邩���B�j

int Kyokumen::Mate(int SorE,int maxDepth,Te &te)
{
	Te teBuf[10000];	// �[��30���x�܂łȂ�\��������傫��

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
			// �l�����𓚂̏ꍇ�����\������B�i�΋ǒ��ɌĂ΂ꂽ�ꍇ�͕\�����Ȃ��B�j
			printf("info depth %d\n",i);
		}
		ret = CheckMate(SorE,0,i,teBuf,te);
		if (ret != 0) {
			break;
		}
	}
	//  0:�s��
	//  1:�l��
	// -1:�s�l��
	return ret;
}

int Kyokumen::CheckMate(int SorE,int depth, int depthMax, Te *checks,Te &te)
{
	if (!isTsumeThink) {
		// �΋ǒ��Ȃ炱���ɓ���B
		if (isStopReceived) {
			// �l�݂�ǂ�ł���Ƃ���stop������ꂽ�炷���ɔ����o���B
			// �i�l�݂�ǂނ̂͂��Ȃ蒷���ԂɂȂ邱�Ƃ�����B�j
			throw 0; // �߂���Sikou::NegaAlphaBeta()
		}
		unsigned long diffTime = timeGetTime() - thinkStartTime;
		if (diffTime > 500) {
			throw 0; // �΋ǂł́A�l�݂�ǂނ̂�0.5�b�܂łɂ���B
		}
	} else {
		// �l�����𓚂̏ꍇ���������ɓ���B�i�΋ǒ��Ȃ����Ȃ��B�j
		if (!isInfinite) {
			unsigned long diffTime = timeGetTime() - thinkStartTime;
			if (diffTime > tsumeLimitTime) {
				throw 0; // �߂���Lesserkai::main()��"go mate"�̂Ƃ���B
			}
		}
		if (mateNodes == 0) {
			prevTime = timeGetTime();
		}
		unsigned long currTime = timeGetTime();
		unsigned long diffTime = currTime - prevTime;
		if (diffTime > 1000) {
			// �T���ǖʐ��Ȃǂ̏��\���i�Ƃ肠�����ACheckMate()��AntiCheckMate()���Ă΂ꂽ�񐔂�T���ǖʐ��Ƃ���B�j
			unsigned long nps = (uint64)(mateNodes * 1000) / (currTime - thinkStartTime); // evaluatedNodes * 1000��int�͈̔͂𒴂��邱�Ƃ�����̂ŁAuint64�ŃL���X�g����B
			printf("info nodes %lu nps %lu\n", mateNodes, nps);
			prevTime = currTime;
		}
		++mateNodes;
	}
	int teNum = MakeChecks(SorE,checks);
	if (teNum == 0) {
		TsumeHash::Add(KyokumenHashVal,HandHashVal,Hand+SorE,-1,0);
		return -1;	//�l�܂Ȃ�
	}
	TsumeVal *p;
	if ((p=TsumeHash::DomSearchCheckMate(KyokumenHashVal,Hand+SorE))!=NULL) {
		te=p->te;
		return 1; //�l��
	}
	int valmax = -1;
	for (int i = 0; i < teNum; i++) {
		Kyokumen kk(*this);
		kk.Move(SorE,checks[i]);
		int val = kk.AntiCheckMate(SorE^0x30, depth+1, depthMax, checks+teNum);
		if (val > valmax) valmax = val;
		if (valmax == 1) {
			te=checks[i];
			break; //�l��
		}
	}
	if (valmax == 1) { //�l��
		TsumeHash::Add(KyokumenHashVal,HandHashVal,Hand+SorE,1,te);
	} else if (valmax == -1) { //�{���ɋl�܂Ȃ�����
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
	if (depth >= depthMax+1) return 0; //�����̌��E�l�݂͕s��

	for (i = 0; i < teNum; i++) {
		Kyokumen k(*this);
		k.Move(SorE,antichecks[i]);
		int val = k.CheckMate(SorE^0x30, depth+1, depthMax, antichecks+teNum,te);
		if (val < valmin) valmin = val;
		if (valmin == -1) break; // �l�܂Ȃ�����
		if (valmin == 0) break; // �l�܂Ȃ�����
	}
	return valmin;
}

// ����͕K���l�݂������������ƂŌĂԂ��ƁB
Te Kyokumen::GetTsumeTe(int SorE)
{
	TsumeVal *p;
	if ((p=TsumeHash::Find(KyokumenHashVal,HandHashVal,Hand+SorE))!=NULL) {
		return p->te;
	}
	return Te(0); // �l�݂������������ƂȂ�A�����ɗ��邱�Ƃ͂Ȃ��B
}

bool Kyokumen::IsNyugyokuWin(int SorE)
{
	// ���ʏ������ǂ����A�ȉ��Ɋ�Â��Ĕ��肷��B
	// http://www.computer-shogi.org/protocol/tcp_ip_1on1_11.html
	if (SorE == SELF) {
		if (controlE[kingS] != 0) {
			return false; // ���ʂɉ��肪�������Ă����珟���ł͂Ȃ��B
		}
		if (kingS % 0x10 > 3) {
			return false; // �ʂ��G�w�R�i�ڈȓ��ɂȂ���Ώ����ł͂Ȃ��B
		}
		int komaCount = 0; // �G�w�R�i�ڈȓ��ɂ���i�ʂ������j��̖����B
		int komaPoint = 0; // ���_�̑ΏۂƂȂ��̓��_�i���T�_�A����P�_�j�B
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
			return false; // �G�w�R�i�ڈȓ��ɂ����P�O�������Ȃ珟���ł͂Ȃ��B
		}
		for (int i = SFU; i <= SHI; i++) {
			if (i == SHI || i == SKA) {
				komaPoint += Hand[i] * 5;
			} else {
				komaPoint += Hand[i];
			}
		}
		if (komaPoint < 28) {
			return false; // ���̏ꍇ�A���_���Q�W�_�����Ȃ珟���ł͂Ȃ��B
		}
		return true; // �S�Ă̏����𖞂����Ă���Ώ����B
	} else {
		if (controlS[kingE] != 0) {
			return false; // ���ʂɉ��肪�������Ă����珟���ł͂Ȃ��B
		}
		if (kingE % 0x10 < 7) {
			return false; // �ʂ��G�w�R�i�ڈȓ��ɂȂ���Ώ����ł͂Ȃ��B
		}
		int komaCount = 0; // �G�w�R�i�ڈȓ��ɂ���i�ʂ������j��̖����B
		int komaPoint = 0; // ���_�̑ΏۂƂȂ��̓��_�i���T�_�A����P�_�j�B
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
			return false; // �G�w�R�i�ڈȓ��ɂ����P�O�������Ȃ珟���ł͂Ȃ��B
		}
		for (int i = EFU; i <= EHI; i++) {
			if (i == EHI || i == EKA) {
				komaPoint += Hand[i] * 5;
			} else {
				komaPoint += Hand[i];
			}
		}
		if (komaPoint < 27) {
			return false; // ���̏ꍇ�A���_���Q�V�_�����Ȃ珟���ł͂Ȃ��B
		}
		return true; // �S�Ă̏����𖞂����Ă���Ώ����B
	}
}
