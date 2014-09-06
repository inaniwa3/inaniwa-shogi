#include <string.h>
#include <string>

#include "USIUtil.h"

using namespace std;

extern unsigned long remainTime; // �c�莞��
extern unsigned long byoyomiTime; // �b�ǂ݂̎���
extern unsigned long tsumeLimitTime; // �l�����𓚂̐�������
extern bool isInfinite; // �l�����𓚂̎v�l���Ԃ����������ǂ���
extern Te InaniwaLastTe; //ina//

static char komaSFENNameArray[] = { ' ', 'P', 'L', 'N', 'S', 'G', 'B', 'R', 'K' }; // SFEN�`���Ŏg��������̕�����i��葤�A������j
static char komaSFENNameArray2[] = { ' ', 'p', 'l', 'n', 's', 'g', 'b', 'r', 'k' }; // SFEN�`���Ŏg��������̕�����i��葤�j

// �񍇖@�Ȏ肩�ǂ������肷��֐��ł��B
bool IsIllegal(Te te,int teNum,Te *teBuf)
{
	// �v����ɁA��̈ꗗ�̒��ɂ�������A
	for(int i=0;i<teNum;i++) {
		if (te==teBuf[i]) {
			// Illegal�ł͂Ȃ��A�Ƃ������Ƃ�false��Ԃ��܂��B
			return false;
		}
	}
	// ��̈ꗗ�̒��ɂȂ���́A��@�Ȏ聁�w���Ă͂����Ȃ���ł��B
	return true;
}

KomaInf USIUtil::MochigomaSFENKomaNameToValue(char komaName, int SorE)
{
	for (int i = 1; i <= 7; i++) // �ʂ͎�����ɂȂ�Ȃ��̂�7�܂łŏ\��
	{
		if (komaName == komaSFENNameArray[i])
		{
			return SorE | (KomaInf)(i);
		}
	}
	return 0;
}

KomaInf USIUtil::SFENKomaNameToValue(char komaName)
{
	for (int i = 1; i <= 8; i++) // �ʂ��܂߂�8�܂�
	{
		if (komaName == komaSFENNameArray[i])
		{
			return SELF | (KomaInf)(i);
		}
		if (komaName == komaSFENNameArray2[i])
		{
			return ENEMY | (KomaInf)(i);
		}
	}
	return 0;
}

void USIUtil::ClearMochigoma(int motigoma[])
{
	for (int i = 0; i < EHI + 1; i++) {
		motigoma[i] = 0;
	}
}

void USIUtil::MakeCustomKyokumen(const char *buf, KomaInf customBan[9][9], int motigoma[])
{
	int pos = strlen("position sfen "); // �Ō�̃X�y�[�X�ɒ���
	const char* ptr = buf + pos;
	int num;
	for (int dan = 1; dan <= 9; dan++) {
		int suji = 1; // ���́u�؁v�Ƃ����̂́A���ʂƂ͔��]�������̂ɂȂ�B
		while (suji <= 9) {
			if (*ptr == '/') {
				++ptr;
			} else if (*ptr >= '1' && *ptr <= '9') {
				sscanf(ptr, "%1d", &num);
				for (int i = 0; i < num; i++) {
					customBan[dan - 1][suji - 1] = EMPTY;
					++suji;
				}
				++ptr;
			} else if (*ptr == '+') {
				++ptr;
				KomaInf koma = SFENKomaNameToValue(*ptr) | PROMOTED;
				customBan[dan - 1][suji - 1] = koma;
				++ptr;
				++suji;
			} else {
				KomaInf koma = SFENKomaNameToValue(*ptr);
				customBan[dan - 1][suji - 1] = koma;
				++ptr;
				++suji;
			}
		}
	}
	
	ptr += 3;
	while (*ptr != ' ') {
		num = 1;
		if (*ptr >= '1' && *ptr <= '9') {
			if (*(ptr + 1) >= '1' && *(ptr + 1) <= '9') {
				sscanf(ptr, "%2d", &num); // �Q���̐����Ȃ�
				ptr += 2;
			} else {
				sscanf(ptr, "%1d", &num);
				ptr += 1;
			}
		}
		KomaInf koma = SFENKomaNameToValue(*ptr);
		motigoma[koma] += num;
		++ptr;
	}
}

int USIUtil::AddMove(const char *buf, Kyokumen *k, int SorE, int teNum, Te *teBuf)
{
	unsigned char from,to;
	KomaInf koma,capture;
	unsigned char promote = 0;
	int pos = 0;
	if (buf[1] == '*') {
		// �������ł�̏ꍇ
		from = 0;
		char komaName[2];
		char dummy[2];
		int toSuji;
		int toDan;
		char toDanBuf[2];
		sscanf(buf, "%1s%1s%1d%1s", komaName, dummy, &toSuji, toDanBuf);
		koma = MochigomaSFENKomaNameToValue(komaName[0], SorE);
		toDan = toDanBuf[0] - 'a' + 1;
		to = toSuji * 0x10 + toDan;
		capture = 0;
		promote = 0;
		pos += 5;
	} else {
		int fromSuji;
		int toSuji;
		char fromDanBuf[2];
		char toDanBuf[2];
		sscanf(buf, "%1d%1s%1d%1s", &fromSuji, fromDanBuf, &toSuji, toDanBuf);
		int fromDan = fromDanBuf[0] - 'a' + 1;
		int toDan = toDanBuf[0] - 'a' + 1;
		from = fromSuji * 0x10 + fromDan;
		to = toSuji * 0x10 + toDan;
		koma = k->ban[from];
		capture = k->ban[to];
		promote = buf[4] == '+' ? 1 : 0;
		pos += promote ? 6 : 5;
	}
	Te te=Te(from,to,koma,capture,promote);
	// ���͂��ꂽ�肪�A��������������A
	if (IsIllegal(te,teNum,teBuf)) {
		// �������Ֆʂ�\�����ē��͂��Ȃ����ł��B
		//printf("���͂��ꂽ�肪�ُ�ł��B���͂��Ȃ����Ă��������B\n");
		char illBuf[10];
		if (promote) {
			strncpy(illBuf, buf, 5);
			illBuf[5] = 0;
		} else {
			strncpy(illBuf, buf, 4);
			illBuf[4] = 0;
		}
		printf("illegal move %s\n", illBuf); // �����USI�̃R�}���h�ł͂Ȃ�
		return 0;
	}
	k->Move(SorE, te);
	InaniwaLastTe = te; //ina//
	return pos;
}

int USIUtil::AddAllMoves(const char *buf, int len, Kyokumen *k, int& SorE, int& teNum, Te *teBuf)
{
	string moveStr = buf;
	int pos = moveStr.find(" moves ") + 7; // 7 = strlen(" moves ")
	while (pos < len) {
		int delta = AddMove(buf + pos, k, SorE, teNum, teBuf);
		if (delta == 0) {
			break;
		}
		pos += delta;
		SorE = SorE == SELF ? ENEMY : SELF;
		teNum = k->MakeLegalMoves(SorE, teBuf);
	}
	return SorE;
}

int USIUtil::ParseTime(const char *buf)
{
	int time = 0;
	sscanf(buf, "%d", &time);
	return time;
}

void USIUtil::ParseAllTimes(const char *buf, int SorE)
{
	string goStr = buf;
	if (goStr.find("infinite") != string::npos) {
		isInfinite = true;
	} else {
		isInfinite = false;
		int pos = goStr.find(" btime ") + 7; // 7 = strlen(" btime ")
		int senteTime = ParseTime(buf + pos);
		pos = goStr.find(" wtime ") + 7; // 7 = strlen(" wtime ")
		int goteTime = ParseTime(buf + pos);
		remainTime = SorE == SELF ? senteTime : goteTime;
		pos = goStr.find(" byoyomi ") + 9; // 9 = strlen(" byoyomi ")
		byoyomiTime = ParseTime(buf + pos);
	}
}

void USIUtil::ParseLimitTimes(const char *buf)
{
	string goStr = buf;
	if (goStr.find("infinite") != string::npos) {
		isInfinite = true;
	} else {
		isInfinite = false;
		int pos = goStr.find(" mate ") + 6; // 6 = strlen(" mate ");
		tsumeLimitTime = ParseTime(buf + pos);
	}
}
