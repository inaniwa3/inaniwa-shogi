#include "Kyokumen.h"

void Te::FPrint(FILE *fp)
{
	if (from != 0) {
		int fromSuji = from / 0x10;
		int fromDan = from % 0x10;
		fprintf(fp, "%d", fromSuji);
		fprintf(fp, "%s", danSFENNameArray[fromDan]);
	} else {
		fprintf(fp, "%s", mochiGomaSFENNameArray[koma]);
		fprintf(fp, "*");
	}
	int toSuji = to / 0x10;
	int toDan = to % 0x10;
	fprintf(fp, "%d", toSuji);
	fprintf(fp, "%s", danSFENNameArray[toDan]);
	if (promote) {
		fprintf(fp, "+");
	}
}

Te::Te(int SorE,unsigned char f,unsigned char t,const Kyokumen &k)
{
	if (f>100) {
		koma=(f-100)+SorE;
		from=0;
	} else {
        int fd = (f + 8) / 9;
        int fs = (f - 1) % 9 + 1;
		from=fs*16+fd;
		koma=k.ban[from];
	}
	if (t>100) {
		t=t-100;
		promote=1;
	} else {
		promote=0;
	}
    int td = (t + 8) / 9;
    int ts = (t - 1) % 9 + 1;
	to=ts*16+td;
	capture=k.ban[to];
	Kind=0;
	value=0;
}
