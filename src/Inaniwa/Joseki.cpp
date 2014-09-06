#include <stdlib.h>
#include <string.h>

#include "Kyokumen.h"

Joseki::Joseki(char *filenames)
{
	//char chCurrentDir[256];
	//GetCurrentDirectory( sizeof( chCurrentDir ), chCurrentDir ); 
	// filenamesは、,区切りとする。
	// JosekiDataは１エントリー５１２バイトとする。
	// JosekiSizeはファイルの長さ/512となる。
	char *filename=filenames;
	char *nextfile=strchr(filenames,',');
	if (nextfile!=NULL) {
		*nextfile='\0';	// ,を\0で置き換え
		nextfile++;		// 次のファイル名の先頭へ
		child=new Joseki(nextfile);
	} else {
		child=NULL;
	}
	FILE *fp=fopen(filename,"rb");
	JosekiSize=0;
	if (fp!=NULL) {
		for(;;) {
			char buf[512];
			if (fread(buf,1,512,fp)<=0) break;
//			JosekiData[JosekiSize]=(unsigned char*)malloc(512);
//			memcpy(JosekiData[JosekiSize],buf,512);
			JosekiSize++;
		}
		JosekiData=(unsigned char **)malloc(sizeof(unsigned char *)*JosekiSize);
		fseek(fp,0,SEEK_SET);
		for(int i=0;i<JosekiSize;i++) {
			JosekiData[i]=(unsigned char*)malloc(512);
			if (fread(JosekiData[i],1,512,fp)<=0) break;
		}
		fclose(fp);
	}
}

void Joseki::fromJoseki(Kyokumen &shoki,int shokiTeban,Kyokumen &k,int tesu,int &teNum,Te te[],int hindo[])
{
	teNum=0;
	for(int i=0;i<JosekiSize;i++) {
		Kyokumen kk(shoki);
		int teban=shokiTeban;
		int j;
		for (j=0;j<tesu;j++) {
			if (JosekiData[i][j*2]==0 || JosekiData[i][j*2]==0xff) break;
			Te te=Te(teban,JosekiData[i][j*2+1],JosekiData[i][j*2],kk);
			kk.Move(teban,te);
			if (teban==ENEMY) {
				teban=SELF;
			} else {
				teban=ENEMY;
			}
		}
		if (j==tesu && k==kk) {
			Te ret=Te(teban,JosekiData[i][j*2+1],JosekiData[i][j*2],kk);
			if (JosekiData[i][j*2]==0 || JosekiData[i][j*2]==0xff) continue;
			int l;
			for(l=0;l<teNum;l++) {
				if (ret==te[l]) {
					hindo[l]++;
					break;
				}
			}
			if (l==teNum) {
				te[l]=ret;
				hindo[l]=1;
				teNum++;
			}
		}
	}
	if (child!=NULL && teNum==0) {
		child->fromJoseki(shoki,shokiTeban,k,tesu,teNum,te,hindo);
	}
}
