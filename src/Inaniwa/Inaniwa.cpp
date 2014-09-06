#include "Sikou.h"

extern int InaniwaTimeTesu;
extern int InaniwaKomagumiTesu;
extern Te  InaniwaLastTe;

// 思考部メイン
Te Sikou::InaniwaTime(int SorE, KyokumenKomagumi &k)
{
	Te te(0);

	// ---------- 0. 稲庭タイムの判定 ---------- //
	if(!InaniwaAlgorithm0(SorE,k)) {                   // 稲庭タイムの判定 //
		// printf("info string InaniwaAlgorithm0\n");
		return te;
	}

	// ---------- 1. 受ける ---------- //
	te = InaniwaAlgorithmE(SorE,k);                    // 角が覗いてくる //
	if(te.IsNull()==0) {
		// printf("info string InaniwaAlgorithmE\n");
		return te;
	}

	te = InaniwaAlgorithmF(SorE,k,0x07);               // 七〜九段目に敵の駒があれば取る //
	if(te.IsNull()==0) {
		// printf("info string InaniwaAlgorithmF\n");
		return te;
	}

	te = InaniwaAlgorithmA(SorE,k);                    // 敵が六段目にいる //
	if(te.IsNull()==0) {
		// printf("info string InaniwaAlgorithmA\n");
		return te;
	}

	// ---------- 2. 駒組みをする ---------- //
	te = InaniwaAlgorithm3(SorE,k);                    // 駒組みをする //
	if(te.IsNull()==0) {
		// printf("info string InaniwaAlgorithm3\n");
		return te;
	}
	
	te = InaniwaAlgorithm1(SorE,k);                    // 七段目に歩が打てたら打つ //
	if(te.IsNull()==0) {
		// printf("info string InaniwaAlgorithm1\n");
		return te;
	}

	te = InaniwaAlgorithm4(SorE,k);                    // 定位置駒を定位置に戻す //
	if(te.IsNull()==0) {
		// printf("info string InaniwaAlgorithm4\n");
		return te;
	}
	
	te = InaniwaAlgorithmG(SorE,k);                    // 敵が五段目にいる //
	if(te.IsNull()==0) {
		// printf("info string InaniwaAlgorithmG\n");
		return te;
	}

	// ---------- 3. ウロウロする ---------- //
	te = InaniwaAlgorithmD(SorE,k);                    // ウロウロする //
	if(te.IsNull()==0) {
		// printf("info string InaniwaAlgorithmD\n");
		return te;
	}

	return te;
}

// ------------------------------ 0. 稲庭タイムの判定 ------------------------------ //

// 相手の持ち駒に歩超があれば稲庭タイム終了
bool Sikou::InaniwaAlgorithm0(int SorE, KyokumenKomagumi &k)
{
	if(SorE==SELF) {
		for(int i=EKY;i<=EHI;i++) {
			if(k.Hand[i]>0) {
				InaniwaTimeTesu = -1;
				return false;
			}
		}
	}
	if(SorE==ENEMY) {
		for(int i=SKY;i<=SHI;i++) {
			if(k.Hand[i]>0) {
				InaniwaTimeTesu = -1;
				return false;
			}
		}
	}

	return true;
}

// ------------------------------ 1. 受ける ------------------------------ //

// 角が覗いてくる
Te Sikou::InaniwaAlgorithmE(int SorE, KyokumenKomagumi &k)
{
	int teNum = 0;
	Te teBuf[600];

	if(SorE==SELF) {
		if((k.CountControlE(0x37)>>16&1)==1 && k.ban[0x37]==EMPTY) {
			teBuf[teNum++] = Te(0x00,0x37,SFU,k.ban[0x37]);  // まず七段目に歩を打ってみる
			if(k.ban[0x48]==SKI || k.ban[0x48]==SHI || k.ban[0x59]==SHI) {
				teBuf[teNum++] = InaniwaAlgorithmD(SorE,k);  // きっと避けてくれる
			}
		}
		if((k.CountControlE(0x37)>>18&1)==1 && k.ban[0x37]==EMPTY) {
			teBuf[teNum++] = Te(0x00,0x37,SFU,k.ban[0x37]);  // まず七段目に歩を打ってみる
			teBuf[teNum++] = Te(0x17,0x28,SGI,k.ban[0x28]);  // ダメもとで銀を戻してみる
			if(k.ban[0x38]!=SKI) {
				teBuf[teNum++] = InaniwaAlgorithmD(SorE,k);  // きっと避けてくれる
			}
		}
		if((k.CountControlE(0x57)>>18&1)==1 && k.ban[0x57]==EMPTY) {
			teBuf[teNum++] = Te(0x00,0x57,SFU,k.ban[0x57]);  // まず七段目に歩を打ってみる
			if(k.ban[0x48]==SKI || k.ban[0x48]==SHI || k.ban[0x39]==SHI) {
				teBuf[teNum++] = InaniwaAlgorithmD(SorE,k);  // きっと避けてくれる
			}
		}
	}
	if(SorE==ENEMY) {
		if((k.CountControlS(0x73)>>23&1)==1 && k.ban[0x73]==EMPTY) {
			teBuf[teNum++] = Te(0x00,0x73,EFU,k.ban[0x73]);  // まず三段目に歩を打ってみる
			if(k.ban[0x62]==EKI || k.ban[0x62]==EHI || k.ban[0x51]==EHI) {
				teBuf[teNum++] = InaniwaAlgorithmD(SorE,k);  // きっと避けてくれる
			}
		}
		if((k.CountControlS(0x73)>>21&1)==1 && k.ban[0x73]==EMPTY) {
			teBuf[teNum++] = Te(0x00,0x73,EFU,k.ban[0x73]);  // まず三段目に歩を打ってみる
			teBuf[teNum++] = Te(0x93,0x82,EGI,k.ban[0x82]);  // ダメもとで銀を戻してみる
			if(k.ban[0x72]!=EKI) {
				teBuf[teNum++] = InaniwaAlgorithmD(SorE,k);  // きっと避けてくれる
			}
		}
		if((k.CountControlS(0x53)>>21&1)==1 && k.ban[0x53]==EMPTY) {
			teBuf[teNum++] = Te(0x00,0x53,EFU,k.ban[0x53]);  // まず三段目に歩を打ってみる
			if(k.ban[0x62]==EKI || k.ban[0x62]==EHI || k.ban[0x71]==EHI) {
				teBuf[teNum++] = InaniwaAlgorithmD(SorE,k);  // きっと避けてくれる
			}
		}
	}

	for(int i=0;i<teNum;i++) {
		if(k.IsLegalMove(SorE,teBuf[i])) {
			return teBuf[i];
		}
	}

	Te te(0);
	return te;
}

// 七〜九段目に敵の駒があれば取る
Te Sikou::InaniwaAlgorithmF(int SorE, KyokumenKomagumi &k, int danTH)
{
	int teNum;
	Te teBuf[600];
	Te teBufBest(0);

	teNum = k.MakeLegalMoves(SorE,teBuf);

	// ---------- 自分の飛の筋と段を求める ---------- //
	// ---------- 相手の飛の筋　　を求める ---------- //
	int sujiSHI = 0, danSHI = 0;
	int sujiEHI = 0, danEHI = 0;
	if(SorE==SELF){
		for(int suji=0x10;suji<=0x90;suji+=0x10) {
			for(int dan=0x01;dan<=0x09;dan+=0x01) {
				if(k.ban[suji+dan]==SHI) sujiSHI = suji, danSHI = dan;
				if(k.ban[suji+dan]==EHI) sujiEHI = suji, danEHI = dan;
			}
		}
	}
	if(SorE==ENEMY){
		for(int suji=0x90;suji>=0x10;suji-=0x10) {
			for(int dan=0x09;dan>=0x01;dan-=0x01) {
				if(k.ban[suji+dan]==EHI) sujiEHI = suji, danEHI = dan;
				if(k.ban[suji+dan]==SHI) sujiSHI = suji, danSHI = dan;
			}
		}
	}

	if(SorE==SELF) {
		for(int i=0;i<teNum;i++) {
			if((teBuf[i].capture&ENEMY) && (teBuf[i].to&0x0f)>=danTH) {
				if(k.CountControlE(teBuf[i].to)) {
					if((teBuf[i].koma<teBufBest.koma || teBufBest.koma==0) && !(teBuf[i].to==0x57 && sujiEHI>=0x70)) {  // ５七はとらない
						teBufBest = teBuf[i];
					}
				} else {
					if(teBuf[i].koma>teBufBest.koma && teBufBest.koma!=SFU) {  // 取りにいく駒の優先度は歩＞王＞…＞香
						teBufBest = teBuf[i];
					}
				}
			}
		}
	}
	if(SorE==ENEMY) {
		for(int i=0;i<teNum;i++) {
			if((teBuf[i].capture&SELF) && (teBuf[i].to&0x0f)<=(0x0A-danTH)) {
				if(k.CountControlS(teBuf[i].to)) {
					if((teBuf[i].koma<teBufBest.koma || teBufBest.koma==0) && !(teBuf[i].to==0x53 && sujiSHI<=0x30)) {  // ５三はとらない
						teBufBest = teBuf[i];
					}
				} else {
					if(teBuf[i].koma>teBufBest.koma && teBufBest.koma!=EFU) {  // 取りにいく駒の優先度は歩＞王＞…＞香
						teBufBest = teBuf[i];
					}
				}
			}
		}
	}

	// ---------- ここから例外 ---------- // 

	if(SorE==SELF) {
		if(k.ban[0x88]==SKI && k.ban[0x99]==SKY && sujiEHI==0x70 && k.ban[0x98]&ENEMY) {
			teBufBest = Te(0x99,0x98,SKY,k.ban[0x98]);
		}
		if(k.ban[0x88]==SKI && k.ban[0x99]==SKY && sujiEHI==0x70 && k.ban[0x97]&ENEMY) {
			teBufBest = Te(0x99,0x97,SKY,k.ban[0x97]);
		}
		if(k.ban[0x88]==SKI && k.ban[0x98]==SKY && k.ban[0x97]&ENEMY) {
			teBufBest = Te(0x98,0x97,SKY,k.ban[0x97]);
		}
		if(k.ban[0x28]==SGI && k.ban[0x19]==SKY && k.ban[0x17]&ENEMY) {
			if(((k.CountControlE(0x37)>>18&1)==1 || (k.CountControlE(0x46)>>18&1)==1 || (k.CountControlE(0x55)>>18&1)==1) && (k.ban[0x35]==EFU || k.Hand[EFU]>0 || k.ban[0x37]==EMPTY)) {
				teBufBest = Te(0x19,0x17,SKY,k.ban[0x17]);
			}
		}
		if(k.ban[0x28]==SGI && k.ban[0x18]==SKY && k.ban[0x17]&ENEMY) {
			if(((k.CountControlE(0x37)>>18&1)==1 || (k.CountControlE(0x46)>>18&1)==1 || (k.CountControlE(0x55)>>18&1)==1) && (k.ban[0x35]==EFU || k.Hand[EFU]>0 || k.ban[0x37]==EMPTY)) {
				teBufBest = Te(0x18,0x17,SKY,k.ban[0x17]);
			}
		}
		if(k.ban[0x57]&ENEMY && k.CountControlE(0x57)) {
			if(k.ban[0x59]==SHI || k.ban[0x48]==SHI) {
				;
			} else {
				if(sujiEHI>=0x70) {
					teBufBest = InaniwaAlgorithmD(SorE,k);  // きっとなんとかしてくれる
				} else {
					;
				}
			}
		}
		if(teBufBest.to==0x57 && teBufBest.koma==SKI) {
			Te teTmp;
			teTmp = Te(0x68,0x57,SGI,k.ban[0x57]);
			if(k.IsLegalMove(SorE,teTmp)) {
				teBufBest = teTmp;
			}
		}
	}
	if(SorE==ENEMY) {
		if(k.ban[0x22]==EKI && k.ban[0x11]==EKY && sujiSHI==0x30 && k.ban[0x12]&SELF) {
			teBufBest = Te(0x11,0x12,EKY,k.ban[0x12]);
		}
		if(k.ban[0x22]==EKI && k.ban[0x11]==EKY && sujiSHI==0x30 && k.ban[0x13]&SELF) {
			teBufBest = Te(0x11,0x13,EKY,k.ban[0x13]);
		}
		if(k.ban[0x22]==EKI && k.ban[0x12]==EKY && k.ban[0x13]&SELF) {
			teBufBest = Te(0x12,0x13,EKY,k.ban[0x13]);
		}
		if(k.ban[0x82]==EGI && k.ban[0x91]==EKY && k.ban[0x93]&SELF) {
			if(((k.CountControlS(0x73)>>21&1)==1 || (k.CountControlS(0x64)>>21&1)==1 || (k.CountControlS(0x55)>>21&1)==1) && (k.ban[0x75]==SFU || k.Hand[SFU]>0 || k.ban[0x73]==EMPTY)) {
				teBufBest = Te(0x91,0x93,EKY,k.ban[0x93]);
			}
		}
		if(k.ban[0x82]==EGI && k.ban[0x92]==EKY && k.ban[0x93]&SELF) {
			if(((k.CountControlS(0x73)>>21&1)==1 || (k.CountControlS(0x64)>>21&1)==1 || (k.CountControlS(0x55)>>21&1)==1) && (k.ban[0x75]==SFU || k.Hand[SFU]>0 || k.ban[0x73]==EMPTY)) {
				teBufBest = Te(0x92,0x93,EKY,k.ban[0x93]);
			}
		}
		if(k.ban[0x53]&SELF && k.CountControlS(0x53)) {
			if(k.ban[0x51]==EHI || k.ban[0x62]==EHI) {
				;
			} else {
				if(sujiSHI<=0x30) {
					teBufBest = InaniwaAlgorithmD(SorE,k);  // きっとなんとかしてくれる
				} else {
					;
				}
			}
		}
		if(teBufBest.to==0x53 && teBufBest.koma==EKI) {
			Te teTmp;
			teTmp = Te(0x42,0x53,EGI,k.ban[0x53]);
			if(k.IsLegalMove(SorE,teTmp)) {
				teBufBest = teTmp;
			}
		}
	}

	return teBufBest;
}

// 敵が六段目にいる
Te Sikou::InaniwaAlgorithmA(int SorE, KyokumenKomagumi &k)
{
	int teNum = 0;
	Te teBuf[600];

	int sujiA[10];
	sujiA[0] = InaniwaLastTe.to&0xf0;  // 一手前

	if(SorE==SELF) {
		for(int i=1,suji=0x10;suji<=0x90;suji+=0x10) {
			sujiA[i++] = suji;
		}
	}
	if(SorE==ENEMY) {
		for(int i=1,suji=0x90;suji>=0x10;suji-=0x10) {
			sujiA[i++] = suji;
		}
	}

	if(SorE==SELF) {
		for(int i=0;i<=9;++i) {  // 優先順高い筋
			int suji = sujiA[i];
			if(k.ban[suji+0x06]&ENEMY) {  // 敵が六段目にいるか
				if(k.ban[suji+0x07]==SFU) {  // 己歩が七段目にいるか
					teBuf[teNum++] = Te(suji+0x07,suji+0x06,SFU,k.ban[suji+0x06]);  // いたら取る
				} else if(k.ban[suji+0x07]==SKY) {  // 己香が七段目にいるか
					teBuf[teNum++] = Te(suji+0x07,suji+0x06,SKY,k.ban[suji+0x06]);  // いたら取る
				} else if(k.ban[suji+0x07]==EMPTY) {  // 七段目になにもなければ
					switch(k.ban[suji+0x06]) {
						case EFU: case EKY:  // 歩か香なら
							if(suji==0x10 || suji==0x90) {  // １筋か９筋なら
								teBuf[teNum++] = Te(0x00,suji+0x08,SFU,k.ban[suji+0x08]);  // まず八段目に歩を打ってみる
							}
							teBuf[teNum++] = Te(0x00,suji+0x07,SFU,k.ban[suji+0x07]);  // 七段目に歩を打つ
							break;
						default:  // それ以外なら // （桂），銀，（金），角，飛，（王），（成）
							teBuf[teNum++] = Te(0x00,suji+0x07,SFU,k.ban[suji+0x07]);  // 七段目に歩を打つ
							break;
					}
				} else {  // 七段目に歩以外がいたら（叩かれたら）
					teBuf[teNum++] = InaniwaAlgorithm4(SorE,k);
					teBuf[teNum++] = InaniwaAlgorithmF(SorE,k,0x06);
				}
			}
		}
	}
	if(SorE==ENEMY) {
		for(int i=0;i<=9;++i) {  // 優先順高い筋
			int suji = sujiA[i];
			if(k.ban[suji+0x04]&SELF) {  // 敵が四段目にいるか
				if(k.ban[suji+0x03]==EFU) {  // 己歩が三段目にいるか
					teBuf[teNum++] = Te(suji+0x03,suji+0x04,EFU,k.ban[suji+0x04]);  // いたら取る
				} else if(k.ban[suji+0x03]==EKY) {  // 己香が三段目にいるか
					teBuf[teNum++] = Te(suji+0x03,suji+0x04,EKY,k.ban[suji+0x04]);  // いたら取る
				} else if(k.ban[suji+0x03]==EMPTY) {  // 三段目になにもなければ
					switch(k.ban[suji+0x04]) {
						case SFU: case SKY:  // 歩か香なら
							if(suji==0x90 || suji==0x10) {  // ９筋か１筋なら
								teBuf[teNum++] = Te(0x00,suji+0x02,EFU,k.ban[suji+0x02]);  // まず二段目に歩を打ってみる
							}
							teBuf[teNum++] = Te(0x00,suji+0x03,EFU,k.ban[suji+0x03]);  // 三段目に歩を打つ
							break;
						default:  // それ以外なら // （桂），銀，（金），角，飛，（王），（成）
							teBuf[teNum++] = Te(0x00,suji+0x03,EFU,k.ban[suji+0x03]);  // 三段目に歩を打つ
							break;
					}
				} else {  // 三段目に歩以外がいたら（叩かれたら）
					teBuf[teNum++] = InaniwaAlgorithm4(SorE,k);
					teBuf[teNum++] = InaniwaAlgorithmF(SorE,k,0x06);  // 注意：0x04ではない
				}
			}
		}
	}

	for(int i=0;i<teNum;i++) {
		if(k.IsLegalMove(SorE,teBuf[i])) {
			return teBuf[i];
		}
	}

	Te te(0);
	return te;
}

// ------------------------------ 2. 駒組みをする ------------------------------ //

// 駒組みをする
// 駒組み手数が0になるまで列記された手を順に指す
Te Sikou::InaniwaAlgorithm3(int SorE, KyokumenKomagumi &k)
{
	int teNum = 0;
	Te teBuf[600];

	if(SorE==SELF) {
		//if(k.ban[0x33]==EFU || k.ban[0x34]==EFU) {
		if((k.ban[0x33]==EFU || k.ban[0x34]==EFU) && k.ban[0x42]!=EHI) {
			teBuf[teNum++] = Te(0x69,0x78,SKI,EMP);
			teBuf[teNum++] = Te(0x79,0x68,SGI,EMP);
			teBuf[teNum++] = Te(0x88,0x79,SKA,EMP);
			teBuf[teNum++] = Te(0x78,0x88,SKI,EMP);
			teBuf[teNum++] = Te(0x59,0x69,SOU,EMP);
			teBuf[teNum++] = Te(0x69,0x78,SOU,EMP);
			teBuf[teNum++] = Te(0x28,0x48,SHI,EMP);
			teBuf[teNum++] = Te(0x39,0x28,SGI,EMP);
			teBuf[teNum++] = Te(0x49,0x38,SKI,EMP);
		} else {
			teBuf[teNum++] = Te(0x28,0x48,SHI,EMP);
			teBuf[teNum++] = Te(0x39,0x28,SGI,EMP);
			teBuf[teNum++] = Te(0x49,0x38,SKI,EMP);
			teBuf[teNum++] = Te(0x69,0x78,SKI,EMP);
			teBuf[teNum++] = Te(0x79,0x68,SGI,EMP);
			teBuf[teNum++] = Te(0x88,0x79,SKA,EMP);
			teBuf[teNum++] = Te(0x78,0x88,SKI,EMP);
			teBuf[teNum++] = Te(0x59,0x69,SOU,EMP);
			teBuf[teNum++] = Te(0x69,0x78,SOU,EMP);
		}
	}
	if(SorE==ENEMY) {
		//if(k.ban[0x77]==SFU || k.ban[0x76]==SFU) {
		if((k.ban[0x77]==SFU || k.ban[0x76]==SFU) && k.ban[0x68]!=SHI) {
			teBuf[teNum++] = Te(0x41,0x32,EKI,EMP);
			teBuf[teNum++] = Te(0x31,0x42,EGI,EMP);
			teBuf[teNum++] = Te(0x22,0x31,EKA,EMP);
			teBuf[teNum++] = Te(0x32,0x22,EKI,EMP);
			teBuf[teNum++] = Te(0x51,0x41,EOU,EMP);
			teBuf[teNum++] = Te(0x41,0x32,EOU,EMP);
			teBuf[teNum++] = Te(0x82,0x62,EHI,EMP);
			teBuf[teNum++] = Te(0x71,0x82,EGI,EMP);
			teBuf[teNum++] = Te(0x61,0x72,EKI,EMP);
		} else {
			teBuf[teNum++] = Te(0x82,0x62,EHI,EMP);
			teBuf[teNum++] = Te(0x71,0x82,EGI,EMP);
			teBuf[teNum++] = Te(0x61,0x72,EKI,EMP);
			teBuf[teNum++] = Te(0x41,0x32,EKI,EMP);
			teBuf[teNum++] = Te(0x31,0x42,EGI,EMP);
			teBuf[teNum++] = Te(0x22,0x31,EKA,EMP);
			teBuf[teNum++] = Te(0x32,0x22,EKI,EMP);
			teBuf[teNum++] = Te(0x51,0x41,EOU,EMP);
			teBuf[teNum++] = Te(0x41,0x32,EOU,EMP);
		}
	}

	if(InaniwaKomagumiTesu>0) {
		for(int i=0;i<teNum;i++) {
			if(k.IsLegalMove(SorE,teBuf[i])) {
				InaniwaKomagumiTesu--;
				return teBuf[i];
			}
		}
	}

	Te te(0);
	return te;
}

// 七段目に歩が打てたら打つ
Te Sikou::InaniwaAlgorithm1(int SorE, KyokumenKomagumi &k)
{
	if(SorE==SELF) {
		int dan = 0x07;
		for(int suji=0x10;suji<=0x90;suji+=0x10) {
			Te te(0x00,suji+dan,SFU,k.ban[suji+dan]);
			if(k.IsLegalMove(SorE,te)) return te;
		}
	}
	if(SorE==ENEMY) {
		int dan = 0x03;
		for(int suji=0x90;suji>=0x10;suji-=0x10) {
			Te te(0x00,suji+dan,EFU,k.ban[suji+dan]);
			if(k.IsLegalMove(SorE,te)) return te;
		}
	}

	Te te(0);
	return te;
}

// 定位置駒を定位置に戻す
Te Sikou::InaniwaAlgorithm4(int SorE, KyokumenKomagumi &k)
{
	int teNum = 0;
	Te teBuf[600];

	if(SorE==SELF) {
		if(k.ban[0x79]!=SKA) {
			teBuf[teNum++] = Te(0x97,0x79,SKA,EMP);
			teBuf[teNum++] = Te(0x88,0x79,SKA,EMP);
			teBuf[teNum++] = Te(0x57,0x79,SKA,EMP);
			teBuf[teNum++] = Te(0x68,0x79,SKA,EMP);
		}
		if(k.ban[0x28]!=SGI) {
			teBuf[teNum++] = Te(0x17,0x28,SGI,EMP);
			teBuf[teNum++] = Te(0x37,0x28,SGI,EMP);
		}
		if(k.ban[0x68]!=SGI) {
			teBuf[teNum++] = Te(0x57,0x68,SGI,EMP);
			teBuf[teNum++] = Te(0x77,0x68,SGI,EMP);
		}
		if(k.ban[0x88]!=SKI && k.ban[0x98]!=SKI) {
			teBuf[teNum++] = Te(0x97,0x98,SKI,EMP);
			teBuf[teNum++] = Te(0x87,0x88,SKI,EMP);
		}
		if(k.ban[0x38]!=SKI && k.ban[0x48]!=SKI) {
			teBuf[teNum++] = Te(0x37,0x38,SKI,EMP);
			teBuf[teNum++] = Te(0x47,0x48,SKI,EMP);
			teBuf[teNum++] = Te(0x57,0x58,SKI,EMP);
			teBuf[teNum++] = Te(0x58,0x48,SKI,EMP);
		}
		if(k.ban[0x78]!=SOU) {
			teBuf[teNum++] = Te(0x67,0x78,SOU,EMP);
			teBuf[teNum++] = Te(0x77,0x78,SOU,EMP);
			teBuf[teNum++] = Te(0x87,0x78,SOU,EMP);
			teBuf[teNum++] = Te(0x88,0x78,SOU,EMP);
		}
		if(k.ban[0x57]==SHI) {
			teBuf[teNum++] = Te(0x57,0x59,SHI,EMP);
			teBuf[teNum++] = Te(0x57,0x58,SHI,EMP);
		}
	}
	if(SorE==ENEMY) {
		if(k.ban[0x31]!=EKA) {
			teBuf[teNum++] = Te(0x13,0x31,EKA,EMP);
			teBuf[teNum++] = Te(0x22,0x31,EKA,EMP);
			teBuf[teNum++] = Te(0x53,0x31,EKA,EMP);
			teBuf[teNum++] = Te(0x42,0x31,EKA,EMP);
		}
		if(k.ban[0x82]!=EGI) {
			teBuf[teNum++] = Te(0x93,0x82,EGI,EMP);
			teBuf[teNum++] = Te(0x73,0x82,EGI,EMP);
		}
		if(k.ban[0x42]!=EGI) {
			teBuf[teNum++] = Te(0x53,0x42,EGI,EMP);
			teBuf[teNum++] = Te(0x33,0x42,EGI,EMP);
		}
		if(k.ban[0x22]!=EKI && k.ban[0x12]!=EKI) {
			teBuf[teNum++] = Te(0x13,0x12,EKI,EMP);
			teBuf[teNum++] = Te(0x23,0x22,EKI,EMP);
		}
		if(k.ban[0x72]!=EKI && k.ban[0x62]!=EKI) {
			teBuf[teNum++] = Te(0x73,0x72,EKI,EMP);
			teBuf[teNum++] = Te(0x63,0x62,EKI,EMP);
			teBuf[teNum++] = Te(0x53,0x52,EKI,EMP);
			teBuf[teNum++] = Te(0x52,0x62,EKI,EMP);
		}
		if(k.ban[0x32]!=EOU) {
			teBuf[teNum++] = Te(0x43,0x32,EOU,EMP);
			teBuf[teNum++] = Te(0x33,0x32,EOU,EMP);
			teBuf[teNum++] = Te(0x23,0x32,EOU,EMP);
			teBuf[teNum++] = Te(0x22,0x32,EOU,EMP);
		}
		if(k.ban[0x53]==EHI) {
			teBuf[teNum++] = Te(0x53,0x51,EHI,EMP);
			teBuf[teNum++] = Te(0x53,0x52,EHI,EMP);
		}
	}

	for(int i=0;i<teNum;i++) {
		if(k.IsLegalMove(SorE,teBuf[i])) {
			return teBuf[i];
		}
	}

	Te te(0);
	return te;
}

// 敵が五段目にいる
Te Sikou::InaniwaAlgorithmG(int SorE, KyokumenKomagumi &k)
{
	int teNum = 0;
	Te teBuf[600];

	if(SorE==SELF) {
		for(int suji=0x10;suji<=0x90;suji+=0x10) {  // 優先順高い筋
			if(k.ban[suji+0x05]&ENEMY) {  // 敵が五段目にいるか
				if(k.ban[suji+0x06]==SFU) {  // 己歩が六段目にいるか
					teBuf[teNum++] = Te(suji+0x06,suji+0x05,SFU,k.ban[suji+0x05]);  // いたら取る
				} else if(k.ban[suji+0x06]==SKY) {  // 己香が六段目にいるか
					teBuf[teNum++] = Te(suji+0x06,suji+0x05,SKY,k.ban[suji+0x05]);  // いたら取る
				} else if(k.ban[suji+0x06]==EMPTY) {  // 六段目になにもなければ
					;
				} else {  // 六段目に歩以外がいたら（叩かれたら）
					;
				}
			}
		}
	}
	if(SorE==ENEMY) {
		for(int suji=0x90;suji>=0x10;suji-=0x10) {  // 優先順高い筋
			if(k.ban[suji+0x05]&SELF) {  // 敵が五段目にいるか
				if(k.ban[suji+0x04]==EFU) {  // 己歩が四段目にいるか
					teBuf[teNum++] = Te(suji+0x04,suji+0x05,EFU,k.ban[suji+0x05]);  // いたら取る
				} else if(k.ban[suji+0x04]==EKY) {  // 己香が四段目にいるか
					teBuf[teNum++] = Te(suji+0x04,suji+0x05,EKY,k.ban[suji+0x05]);  // いたら取る
				} else if(k.ban[suji+0x04]==EMPTY) {  // 四段目になにもなければ
					;
				} else {  // 四段目に歩以外がいたら（叩かれたら）
					;
				}
			}
		}
	}

	for(int i=0;i<teNum;i++) {
		if(k.IsLegalMove(SorE,teBuf[i])) {
			return teBuf[i];
		}
	}

	Te te(0);
	return te;
}

// ------------------------------ 3. ウロウロする ------------------------------ //

// ウロウロする
Te Sikou::InaniwaAlgorithmD(int SorE, KyokumenKomagumi &k)
{
	int teNum = 0;
	Te teBuf[600];

	int teNumBest = 0;
	Te teBufBest[600];

	int numNO = 0;
	struct tag_NO {
		int no;  // 0:NG, 1:OK
		int pos;
		KomaInf koma;
	} NO[600];

	int sujiSHI = 0, danSHI = 0;
	int sujiEHI = 0, danEHI = 0;

	// ---------- 自分の飛の筋と段を求める ---------- //
	// ---------- 相手の飛の筋　　を求める ---------- //
	if(SorE==SELF){
		for(int suji=0x10;suji<=0x90;suji+=0x10) {
			for(int dan=0x01;dan<=0x09;dan+=0x01) {
				if(k.ban[suji+dan]==SHI) sujiSHI = suji, danSHI = dan;
				if(k.ban[suji+dan]==EHI) sujiEHI = suji, danEHI = dan;
			}
		}
	}
	if(SorE==ENEMY){
		for(int suji=0x90;suji>=0x10;suji-=0x10) {
			for(int dan=0x09;dan>=0x01;dan-=0x01) {
				if(k.ban[suji+dan]==EHI) sujiEHI = suji, danEHI = dan;
				if(k.ban[suji+dan]==SHI) sujiSHI = suji, danSHI = dan;
			}
		}
	}

	// ----------  ---------- //
	if(SorE==SELF){
		// ---------- 歩 ---------- //
		if(k.ban[0x58]==EFU) {
			NO[numNO].pos = 0x59;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
		}
		if(k.ban[0x57]==EFU) {
			NO[numNO].pos = 0x58;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
			NO[numNO].pos = 0x39;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
			NO[numNO].pos = 0x49;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
			NO[numNO].pos = 0x69;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
			NO[numNO].pos = 0x59;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
		}
		// ---------- 角 ---------- //
		if((k.CountControlE(0x37)>>16&1)==1 && (k.ban[0x35]==EFU || k.Hand[EFU]>0 || k.ban[0x37]==EMPTY)) {
			NO[numNO].pos = 0x48;   NO[numNO].no = 0;   NO[numNO++].koma = SKI;
			NO[numNO].pos = 0x48;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
			NO[numNO].pos = 0x59;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
			if(k.ban[0x48]==SKI && k.ban[0x59]==SHI) {  // 優先順に
				if(sujiEHI==0x40) {
					NO[numNO].pos = 0x49;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
				}
				if(sujiEHI==0x50) {
					NO[numNO].pos = 0x58;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
				}
				NO[numNO].pos = 0x49;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
				NO[numNO].pos = 0x58;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
			}
		}
		if(((k.CountControlE(0x37)>>18&1)==1 || (k.CountControlE(0x46)>>18&1)==1 || (k.CountControlE(0x55)>>18&1)==1) && (k.ban[0x35]==EFU || k.Hand[EFU]>0 || k.ban[0x37]==EMPTY)) {
			NO[numNO].pos = 0x48;   NO[numNO].no = 0;   NO[numNO++].koma = SKI;
		}
		if((k.CountControlE(0x57)>>16&1)==1 && (k.ban[0x55]==EFU || k.Hand[EFU]>0 || k.ban[0x57]==EMPTY)) {
			NO[numNO].pos = 0x48;   NO[numNO].no = 1;   NO[numNO++].koma = SKI;
		}
		if(((k.CountControlE(0x57)>>18&1)==1 || (k.CountControlE(0x66)>>18&1)==1 || (k.CountControlE(0x75)>>18&1)==1) && (k.ban[0x55]==EFU || k.Hand[EFU]>0 || k.ban[0x57]==EMPTY)) {
			NO[numNO].pos = 0x48;   NO[numNO].no = 0;   NO[numNO++].koma = SKI;
			NO[numNO].pos = 0x48;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
			NO[numNO].pos = 0x39;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
			if(k.ban[0x48]==SKI && k.ban[0x39]==SHI) {
				NO[numNO].pos = 0x49;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
			}
			NO[numNO].pos = 0x59;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
			NO[numNO].pos = 0x58;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
		}
		Te teTmp = Te(0x00,0x76,EFU,EMP);
		if(((k.CountControlE(0x77)>>16&1)==1 || (k.CountControlE(0x66)>>16&1)==1 || (k.CountControlE(0x55)>>16&1)==1) && (k.ban[0x75]==EFU || (k.Hand[EFU]>0 && k.IsLegalMove(ENEMY,teTmp)) || k.ban[0x77]==EMPTY)) {
			if(sujiEHI==0x70) {
				NO[numNO].pos = 0x98;   NO[numNO].no = 0;   NO[numNO++].koma = SKI;
			} else {
				NO[numNO].pos = 0x88;   NO[numNO].no = 0;   NO[numNO++].koma = SKI;
			}
		}
		// ---------- 飛 ---------- //
		if(sujiEHI==0x10 || sujiEHI==0x20) {
			NO[numNO].pos = 0x48;   NO[numNO].no = 0;   NO[numNO++].koma = SKI;
		}
		if(sujiEHI==0x30) {
			NO[numNO].pos = 0x39;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
		}
		if(sujiEHI==0x40) {
			NO[numNO].pos = 0x49;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;  // 九段優先
			if(k.ban[0x49]!=SHI) {
				NO[numNO].pos = 0x48;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
			}
		}
		if(sujiEHI==0x50) {
			NO[numNO].pos = 0x59;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;  // 九段優先
			if(k.ban[0x59]!=SHI) {
				NO[numNO].pos = 0x58;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
			}
		}
		if(sujiEHI==0x60) {
			if( !( ((k.CountControlE(0x57)>>18&1)==1 || (k.CountControlE(0x66)>>18&1)==1 || (k.CountControlE(0x75)>>18&1)==1) && (k.ban[0x55]==EFU || k.Hand[EFU]>0 || k.ban[0x57]==EMPTY) ) ) {
				NO[numNO].pos = 0x69;   NO[numNO].no = 1;   NO[numNO++].koma = SHI;
			}
		}
		if(sujiEHI==0x70) {
			NO[numNO].pos = 0x98;   NO[numNO].no = 0;   NO[numNO++].koma = SKI;
		}
		if(sujiEHI==0x80) {
			NO[numNO].pos = 0x69;   NO[numNO].no = 0;   NO[numNO++].koma = SHI;
		}
	}
	if(SorE==ENEMY){
		// ---------- 歩 ---------- //
		if(k.ban[0x52]==SFU) {
			NO[numNO].pos = 0x51;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
		}
		if(k.ban[0x53]==SFU) {
			NO[numNO].pos = 0x52;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
			NO[numNO].pos = 0x71;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
			NO[numNO].pos = 0x61;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
			NO[numNO].pos = 0x41;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
			NO[numNO].pos = 0x51;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
		}
		// ---------- 角 ---------- //
		if((k.CountControlS(0x73)>>23&1)==1 && (k.ban[0x75]==SFU || k.Hand[SFU]>0 || k.ban[0x73]==EMPTY)) {
			NO[numNO].pos = 0x62;   NO[numNO].no = 0;   NO[numNO++].koma = EKI;
			NO[numNO].pos = 0x62;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
			NO[numNO].pos = 0x51;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
			if(k.ban[0x62]==EKI && k.ban[0x51]==EHI) {  // 優先順に
				if(sujiSHI==0x60) {
					NO[numNO].pos = 0x61;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
				}
				if(sujiSHI==0x50) {
					NO[numNO].pos = 0x52;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
				}
				NO[numNO].pos = 0x61;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
				NO[numNO].pos = 0x52;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
			}
		}
		if(((k.CountControlS(0x73)>>21&1)==1 || (k.CountControlS(0x64)>>21&1)==1 || (k.CountControlS(0x55)>>21&1)==1) && (k.ban[0x75]==SFU || k.Hand[SFU]>0 || k.ban[0x73]==EMPTY)) {
			NO[numNO].pos = 0x62;   NO[numNO].no = 0;   NO[numNO++].koma = EKI;
		}
		if((k.CountControlS(0x53)>>23&1)==1 && (k.ban[0x55]==SFU || k.Hand[SFU]>0 || k.ban[0x53]==EMPTY)) {
			NO[numNO].pos = 0x62;   NO[numNO].no = 1;   NO[numNO++].koma = EKI;
		}
		if(((k.CountControlS(0x53)>>21&1)==1 || (k.CountControlS(0x44)>>21&1)==1 || (k.CountControlS(0x35)>>21&1)==1) && (k.ban[0x55]==SFU || k.Hand[SFU]>0 || k.ban[0x53]==EMPTY)) {
			NO[numNO].pos = 0x62;   NO[numNO].no = 0;   NO[numNO++].koma = EKI;
			NO[numNO].pos = 0x62;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
			NO[numNO].pos = 0x71;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
			if(k.ban[0x62]==EKI && k.ban[0x71]==EHI) {
				NO[numNO].pos = 0x61;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
			}
			NO[numNO].pos = 0x51;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
			NO[numNO].pos = 0x52;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
		}
		Te teTmp = Te(0x00,0x34,SFU,EMP);
		if(((k.CountControlS(0x33)>>23&1)==1 || (k.CountControlS(0x44)>>23&1)==1 || (k.CountControlS(0x55)>>23&1)==1) && (k.ban[0x35]==SFU || (k.Hand[SFU]>0 && k.IsLegalMove(SELF,teTmp)) || k.ban[0x33]==EMPTY)) {
			if(sujiSHI==0x30) {
				NO[numNO].pos = 0x12;   NO[numNO].no = 0;   NO[numNO++].koma = EKI;
			} else {
				NO[numNO].pos = 0x22;   NO[numNO].no = 0;   NO[numNO++].koma = EKI;
			}
		}
		// ---------- 飛 ---------- //
		if(sujiSHI==0x90 || sujiSHI==0x80) {
			NO[numNO].pos = 0x62;   NO[numNO].no = 0;   NO[numNO++].koma = EKI;
		}
		if(sujiSHI==0x70) {
			NO[numNO].pos = 0x71;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
		}
		if(sujiSHI==0x60) {
			NO[numNO].pos = 0x61;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;  // 一段優先
			if(k.ban[0x61]!=EHI) {
				NO[numNO].pos = 0x62;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
			}
		}
		if(sujiSHI==0x50) {
			NO[numNO].pos = 0x51;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;  // 一段優先
			if(k.ban[0x51]!=EHI) {
				NO[numNO].pos = 0x52;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
			}
		}
		if(sujiSHI==0x40) {
			if( !( ((k.CountControlS(0x53)>>21&1)==1 || (k.CountControlS(0x44)>>21&1)==1 || (k.CountControlS(0x35)>>21&1)==1) && (k.ban[0x55]==SFU || k.Hand[SFU]>0 || k.ban[0x53]==EMPTY) ) ) {
				NO[numNO].pos = 0x41;   NO[numNO].no = 1;   NO[numNO++].koma = EHI;
			}
		}
		if(sujiSHI==0x30) {
			NO[numNO].pos = 0x12;   NO[numNO].no = 0;   NO[numNO++].koma = EKI;
		}
		if(sujiSHI==0x20) {
			NO[numNO].pos = 0x41;   NO[numNO].no = 0;   NO[numNO++].koma = EHI;
		}
	}

	// ---------- ウロウロ手リスト優先順 ---------- //
	if(SorE==SELF) {
		teBuf[teNum++] = Te(0x98,0x88,SKI,EMP);
			teBuf[teNum++] = Te(0x48,0x38,SKI,EMP);
				teBuf[teNum++] = Te(0x48,0x49,SHI,EMP);
				teBuf[teNum++] = Te(0x58,0x59,SHI,EMP);
					teBuf[teNum++] = Te(0x48,0x58,SHI,EMP);
						teBuf[teNum++] = Te(0x39,0x59,SHI,EMP);
						teBuf[teNum++] = Te(0x69,0x59,SHI,EMP);
						teBuf[teNum++] = Te(0x49,0x59,SHI,EMP);
							teBuf[teNum++] = Te(0x39,0x49,SHI,EMP);
							teBuf[teNum++] = Te(0x69,0x49,SHI,EMP);
								teBuf[teNum++] = Te(0x39,0x69,SHI,EMP);
								teBuf[teNum++] = Te(0x69,0x39,SHI,EMP);
							teBuf[teNum++] = Te(0x49,0x69,SHI,EMP);
							teBuf[teNum++] = Te(0x49,0x39,SHI,EMP);
						teBuf[teNum++] = Te(0x59,0x49,SHI,EMP);
						teBuf[teNum++] = Te(0x59,0x69,SHI,EMP);
						teBuf[teNum++] = Te(0x59,0x39,SHI,EMP);
					teBuf[teNum++] = Te(0x58,0x48,SHI,EMP);
				teBuf[teNum++] = Te(0x59,0x58,SHI,EMP);
				teBuf[teNum++] = Te(0x49,0x48,SHI,EMP);
			teBuf[teNum++] = Te(0x38,0x48,SKI,EMP);
		teBuf[teNum++] = Te(0x88,0x98,SKI,EMP);
	}
	if(SorE==ENEMY) {
		teBuf[teNum++] = Te(0x12,0x22,EKI,EMP);
			teBuf[teNum++] = Te(0x62,0x72,EKI,EMP);
				teBuf[teNum++] = Te(0x62,0x61,EHI,EMP);
				teBuf[teNum++] = Te(0x52,0x51,EHI,EMP);
					teBuf[teNum++] = Te(0x62,0x52,EHI,EMP);
						teBuf[teNum++] = Te(0x71,0x51,EHI,EMP);
						teBuf[teNum++] = Te(0x41,0x51,EHI,EMP);
						teBuf[teNum++] = Te(0x61,0x51,EHI,EMP);
							teBuf[teNum++] = Te(0x71,0x61,EHI,EMP);
							teBuf[teNum++] = Te(0x41,0x61,EHI,EMP);
								teBuf[teNum++] = Te(0x71,0x41,EHI,EMP);
								teBuf[teNum++] = Te(0x41,0x71,EHI,EMP);
							teBuf[teNum++] = Te(0x61,0x41,EHI,EMP);
							teBuf[teNum++] = Te(0x61,0x71,EHI,EMP);
						teBuf[teNum++] = Te(0x51,0x61,EHI,EMP);
						teBuf[teNum++] = Te(0x51,0x41,EHI,EMP);
						teBuf[teNum++] = Te(0x51,0x71,EHI,EMP);
					teBuf[teNum++] = Te(0x52,0x62,EHI,EMP);
				teBuf[teNum++] = Te(0x51,0x52,EHI,EMP);
				teBuf[teNum++] = Te(0x61,0x62,EHI,EMP);
			teBuf[teNum++] = Te(0x72,0x62,EKI,EMP);
		teBuf[teNum++] = Te(0x22,0x12,EKI,EMP);
	}

	// ----------  ---------- //
	Te teTmp(0);
	for(int i=0;i<teNum;i++) {  //    -> NG なら 削除  // 前準備
		for(int j=0;j<numNO;j++) {
			if(teBuf[i].to==NO[j].pos && NO[j].no==0 && teBuf[i].koma==NO[j].koma) {  // teBufのto が NOのNG と合致
				teBuf[i] = teTmp;  // リストから削除
			}
		}
	}
	for(int j=0;j<numNO;j++) {  // NG -> OK なら 追加  // 優先順位:1
		for(int i=0;i<teNum;i++) {
			if(teBuf[i].from==NO[j].pos && NO[j].no==0 && teBuf[i].koma==NO[j].koma) {  // NOのNG が teBufのfrom と合致
				for(int k=0;k<numNO;k++) {
					if(teBuf[i].to==NO[k].pos && NO[k].no==1 && teBuf[i].koma==NO[k].koma) {  // teBufのto が NOのOK と合致
						teBufBest[teNumBest++] = teBuf[i];  // 追加
						// printf("info string Priority:1 %s:%2x%2x\n",komaStr2[teBuf[i].koma],teBuf[i].from,teBuf[i].to);
					}
				}
			}
		}
	}
	for(int j=0;j<numNO;j++) {  // NG -> ふつう なら 追加  // 優先順位:2
		for(int i=0;i<teNum;i++) {
			if(teBuf[i].from==NO[j].pos && NO[j].no==0 && teBuf[i].koma==NO[j].koma) {  // NOのNG が teBufのfrom と合致
				int flg = 1;
				for(int k=0;k<numNO;k++) {
					if(teBuf[i].to==NO[k].pos && teBuf[i].koma==NO[k].koma) {  // teBufのto が NOのOKもしくはNG と合致
						flg = 0;
					}
				}
				if(flg==1) {
					teBufBest[teNumBest++] = teBuf[i];  // 追加
					// printf("info string Priority:2 %s:%2x%2x\n",komaStr2[teBuf[i].koma],teBuf[i].from,teBuf[i].to);
				}
			}
		}
	}
	for(int j=0;j<numNO;j++) {  // ふつう -> OK なら 追加  // 優先順位:3
		for(int i=0;i<teNum;i++) {
			if(teBuf[i].to==NO[j].pos && NO[j].no==1 && teBuf[i].koma==NO[j].koma) {  // N0のOK が teBufのto と合致
				int flg = 1;
				for(int k=0;k<numNO;k++) {
					if(teBuf[i].from==NO[k].pos && teBuf[i].koma==NO[k].koma) {  // teBufのfrom が NOのOKもしくはNG と合致
						flg = 0;
					}
				}
				if(flg==1) {
					teBufBest[teNumBest++] = teBuf[i];  // 追加
					// printf("info string Priority:3 %s:%2x%2x\n",komaStr2[teBuf[i].koma],teBuf[i].from,teBuf[i].to);
				}
			}
		}
	}
	for(int j=0;j<numNO;j++) {  // OK -> OK なら 追加  // 優先順位:4
		for(int i=0;i<teNum;i++) {
			if(teBuf[i].from==NO[j].pos && NO[j].no==1 && teBuf[i].koma==NO[j].koma) {  // NOのOK が teBufのfrom と合致
				for(int k=0;k<numNO;k++) {
					if(teBuf[i].to==NO[k].pos && NO[k].no==1 && teBuf[i].koma==NO[k].koma) {  // teBufのto が NOのOK と合致
						teBufBest[teNumBest++] = teBuf[i];  // 追加
						// printf("info string Priority:4 %s:%2x%2x\n",komaStr2[teBuf[i].koma],teBuf[i].from,teBuf[i].to);
					}
				}
			}
		}
	}
	for(int i=0;i<teNum;i++) {  // ふつう -> ふつう なら 追加  // 優先順位:5
		int flg = 1;
		for(int j=0;j<numNO;j++) {
			if(teBuf[i].from==NO[j].pos && teBuf[i].koma==NO[j].koma) {  // teBufのfrom が NOのOKもしくはNG と合致
				flg = 0;
			}
		}
		for(int k=0;k<numNO;k++) {
			if(teBuf[i].to==NO[k].pos && teBuf[i].koma==NO[k].koma) {  // teBufのto が NOのOKもしくはNG と合致
				flg = 0;
			}
		}
		if(flg==1) {
			teBufBest[teNumBest++] = teBuf[i];  // 追加
			// printf("info string Priority:5 %s:%2x%2x\n",komaStr2[teBuf[i].koma],teBuf[i].from,teBuf[i].to);
		}
	}

	// ----------  ---------- //
	for(int i=0;i<teNumBest;i++) {
		if(k.IsLegalMove(SorE,teBufBest[i])) {
			return teBufBest[i];
		}
	}

	Te te(0);
	return te;
}
