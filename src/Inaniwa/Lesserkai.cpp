#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include "WinAPI.h"
#endif

#include <time.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <memory>
#include <queue>

#include "Sikou.h"
#include "USIUtil.h"

using namespace std;

extern auto_ptr<Joseki> joseki;
extern Kyokumen *shoki;

extern bool isStopReceived; // stop�R�}���h����M������
extern bool canPonder; // ��ǂ݉\��
extern bool isPonderThink; // ��ǂݎv�l����
extern bool isTsumeThink; // �l�����𓚎v�l����
extern unsigned long thinkStartTime; // �v�l���J�n��������
extern unsigned long hashCount; // �n�b�V���ɒǉ����ꂽ��
extern unsigned long ponderhitReceiveTime; // ponderhit����M��������
extern bool isInfinite; // �v�l���Ԃ����������ǂ���
extern int thinkDepthMax; // �ǂ݂̍ő�[��
extern int InaniwaTimeTesu;     //ina//
extern int InaniwaKomagumiTesu; //ina//
extern Te  InaniwaLastTe;       //ina//

// �G���W���ݒ�t�@�C���̃p�X
#ifdef _WIN32
//ina// const char* iniFilePath = ".\\Lesserkai.ini";
const char* iniFilePath = ".\\inaniwa.ini"; //ina//
#else
//ina// const char* iniFilePath = "./Lesserkai.ini";
const char* iniFilePath = "./inaniwa.ini";  //ina//
#endif

#ifdef __GNUC__

#define CRITICAL_SECTION pthread_mutex_t
#define InitializeCriticalSection(a) pthread_mutex_init(a, NULL)
#define EnterCriticalSection(a) pthread_mutex_lock(a)
#define LeaveCriticalSection(a) pthread_mutex_unlock(a)
#define DeleteCriticalSection(a) pthread_mutex_destroy(a)
#define Sleep(a) usleep((a) * 1000)

#endif

queue<string> commandQueue; // ��M�����R�}���h��ǉ����邽�߂̃L���[

CRITICAL_SECTION cs; // �N���e�B�J���Z�N�V�����I�u�W�F�N�g
// commandQueue�ɑ΂��āAReceiveThread()��WaitCommand()����
// �����ɃA�N�Z�X�ł��Ȃ��悤�AcommandQueue�ɃA�N�Z�X���镔����
// �N���e�B�J���Z�N�V�����ɂ���B

// �R�}���h��M�̂��߂̊֐��B�v�l���ł���M�ł���悤�ɁA�X���b�h���炱�̊֐����Ăяo���B
#ifdef _WIN32
void ReceiveThread(void *)
#else
void *ReceiveThread(void *)
#endif
{
	char buf[10000];
	while (true) {
		fgets(buf, 10000, stdin);
		buf[strlen(buf) - 1] = 0; // ���s�R�[�h�������B

		if (strncmp(buf, "quit", strlen("quit")) == 0) {
			while (commandQueue.size() > 0) {
				// �G���W���ݒ�_�C�A���O��OK�������ƁAsetoption��quit�������đ����Ă���B
				// ���̎��Asetoption�̓��e�����s����O��exit()���Ă΂Ȃ��悤�Aquit�R�}���h���
				// �O�Ɏ�M�����R�}���h���S�Ď��s����Ă���I������悤�ɂ���B
				// �{���́AcommandQueue�̃T�C�Y��0������Ƃ����āA����ȑO�̃R�}���h���{����
				// ���s���ꂽ�Ƃ����ۏ؂͂Ȃ����ipop���������ŁA�܂����s�O�̉\��������̂Łj�A
				// setoption�Ɋւ��Ă͎��s��200ms�������邱�Ƃ͂Ȃ��̂ŁA���̑΍􂾂��ōς܂���
				// ���Ƃɂ���B
				Sleep(200);
			}
			// �I���O�ɃN���e�B�J���Z�N�V�����I�u�W�F�N�g��j���B
			DeleteCriticalSection(&cs);
			// quit����M������G���W�����I������B
			exit(0);
		}
		EnterCriticalSection(&cs); // �N���e�B�J���Z�N�V�����ɓ���B
		if (strncmp(buf, "stop", strlen("stop")) == 0) {
			isStopReceived = true;
		}
		if (strncmp(buf, "ponderhit", strlen("ponderhit")) == 0) {
			ponderhitReceiveTime = timeGetTime();
		}
		if (strncmp(buf, "gameover", strlen("gameover")) == 0) {
			isStopReceived = true; // ��ǂݒ���gameover�������Ă����ꍇ�A�����v�l��ł��؂�B
		}
		string commandStr = buf;
		commandQueue.push(commandStr); // �R�}���h���L���[�ɒǉ��B
		LeaveCriticalSection(&cs); // �N���e�B�J���Z�N�V�������o��B
	}
}

// ��M�����R�}���h��Ԃ��B
string WaitCommand()
{
	while (commandQueue.empty()) {
		Sleep(100);
	}
	EnterCriticalSection(&cs); // �N���e�B�J���Z�N�V�����ɓ���B
	string commandStr = commandQueue.front(); // �L���[����R�}���h���擾�B
	commandQueue.pop(); // �L���[����R�}���h���폜�B
	LeaveCriticalSection(&cs); // �N���e�B�J���Z�N�V�������o��B
	return commandStr;
}

// �R�}���h��M�X���b�h���J�n����B
bool BeginThread()
{
#ifdef _WIN32
	uintptr_t hdl = _beginthread(ReceiveThread, 0, NULL);
	return (hdl != -1);
#else
	pthread_t thread;
	int err = pthread_create(&thread , NULL , ReceiveThread , NULL);
	return (err == 0);
#endif
}

int main()
{
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0); // ���ꂪ�K�v�i�W���o�͂̃o�b�t�@�����O�𖳌��ɂ���j

	InitializeCriticalSection(&cs); // �N���e�B�J���Z�N�V�����I�u�W�F�N�g�̏������B
	bool isSuccess = BeginThread(); // ��M�X���b�h�J�n�B
	if (!isSuccess) {
		printf("Failed to begin thread\n");
		return -1;
	}

	// Lesserkai.ini�t�@�C��������Ȃ�A�G���W���ݒ�̃f�t�H���g�l���擾�B
//ina// 	char iniValueBuf[256];
//ina// 	::GetPrivateProfileString("CustomSettings", "BookFile", "public.bin", iniValueBuf, 256, iniFilePath);
//ina// 	string bookFileStr = iniValueBuf;
//ina// 	::GetPrivateProfileString("CustomSettings", "UseBook", "true", iniValueBuf, 256, iniFilePath);
//ina// 	string useBookStr = iniValueBuf;
//ina// 	bool isUseJoseki = useBookStr == "true";
	bool isUseJoseki = false; //ina//

	srand((unsigned)time(NULL)); // Sikou::Think()�Œ�Ղ������_���ɑI�ׂ�悤�ɂ��邽�߁B
	Kyokumen::HashInit();
	
	// ����̏����z�u�ł��B���₷���ł���H�ϊ��͂��̕����G�ł����ǁB
	KomaInf HirateBan[9][9]={
		{EKY,EKE,EGI,EKI,EOU,EKI,EGI,EKE,EKY},
		{EMP,EHI,EMP,EMP,EMP,EMP,EMP,EKA,EMP},
		{EFU,EFU,EFU,EFU,EFU,EFU,EFU,EFU,EFU},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{SFU,SFU,SFU,SFU,SFU,SFU,SFU,SFU,SFU},
		{EMP,SKA,EMP,EMP,EMP,EMP,EMP,SHI,EMP},
		{SKY,SKE,SGI,SKI,SOU,SKI,SGI,SKE,SKY}
	};
	// ������͖ʓ|�ł�EHI�܂�0����ׂȂ��Ƃ����܂���B
	int HirateMotigoma[EHI+1]={
	// ����������������������j����p�򉤂ƈǌ\�S���n��������j����p��
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
	// ���菉���ǖʈȊO����J�n���鎞�Ɏg���B
	int customMotigoma[EHI+1]={
	// ����������������������j����p�򉤂ƈǌ\�S���n��������j����p��
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
	KyokumenKomagumi::InitKanagomaValue();
	shoki=new Kyokumen(0, HirateBan, HirateMotigoma);

	while (true) {
		while (true) {
			// �΋Ǒ҂��̃��[�v�Bsetoption�R�}���h�������Ă����炻�̒l��ݒ肷��B
			// usinewgame�R�}���h�������Ă����炱�̃��[�v�𔲂��đ΋ǂ̃��[�v�ɓ���B

			string commandStr = WaitCommand(); // ��M�����R�}���h���擾����
			const char *buf = commandStr.c_str();
			int len = strlen(buf);
			if (strncmp(buf, "usi", len) == 0) {
				// �G���W���N�����A��ԍŏ��ɌĂ΂��R�}���h�B����ɑ΂��ăG���W����id��Ԃ������ƁA
				// �G���W���Őݒ�\�ȃp�����[�^��option�R�}���h�ŕԂ��A�Ō��usiok��Ԃ��K�v������B
//ina// 				printf("id name Lesserkai 1.3.3\n");
				printf("id name inaniwa 1.0.0\n"); //ina//
//ina// 				printf("id author Program Writer\n");
				printf("id author inaniwa3\n"); //ina//
//ina// 				printf("option name BookFile type string default %s\n", bookFileStr.c_str());
//ina// 				printf("option name UseBook type check default %s\n", isUseJoseki ? "true" : "false");
				printf("usiok\n");
			} else if (strncmp(buf, "isready", len) == 0) {
				// �{���̓������������Ɗm�ۂł������ǂ������ׂĂ���readyok��Ԃ��ׂ�����
				// �ʓ|�Ȃ̂ŉ������ׂ���readyok��Ԃ��Ă���B
				printf("readyok\n");
			} else if (strncmp(buf, "setoption", strlen("setoption")) == 0) {
				// �G���W���ɐݒ肷��p�����[�^�������Ă���̂ł����ݒ肵�A
				// �G���W���ŗL�̃p�����[�^�ł���Ώ����ݒ�t�@�C���ɕۑ�����B
				if (strncmp(buf, "setoption name USI_Ponder value true", len) == 0) {
					canPonder = true;
				} else if (strncmp(buf, "setoption name USI_Ponder value false", len) == 0) {
					canPonder = false;
				} else if (strncmp(buf, "setoption name USI_Hash value ", strlen("setoption name USI_Hash value ")) == 0) {
					string hashStr = buf + strlen("setoption name USI_Hash value ");
					// int hashSize = atoi(hashStr.c_str());
					// �{���ł���΁A���̒l�����Ƀn�b�V���T�C�Y��ݒ肷�ׂ��ł���B
//ina// 				} else if (strncmp(buf, "setoption name BookFile value ", strlen("setoption name BookFile value ")) == 0) {
//ina// 					bookFileStr = buf + strlen("setoption name BookFile value ");
//ina// 					// �����ݒ�t�@�C���ɕۑ����Ă���
//ina// 					::WritePrivateProfileString("CustomSettings", "BookFile", bookFileStr.c_str(), iniFilePath);
//ina// 				} else if (strncmp(buf, "setoption name UseBook value ", strlen("setoption name UseBook value ")) == 0) {
//ina// 					useBookStr = buf + strlen("setoption name UseBook value ");
//ina// 					isUseJoseki = useBookStr == "true";
//ina// 					// �����ݒ�t�@�C���ɕۑ����Ă���
//ina// 					::WritePrivateProfileString("CustomSettings", "UseBook", isUseJoseki ? "true" : "false", iniFilePath);
				}
			} else if (strncmp(buf, "usinewgame", len) == 0) {
				// �V�K�΋ǊJ�n�̃R�}���h�B�������M�����炱�̃��[�v�𔲂���B
				break;
			}
		}

//ina// 		if (joseki.get() == NULL) {
//ina// 			// ��Ճt�@�C�����w�肳��Ă��āA�܂������ǂ�ł��Ȃ�������A���̃t�@�C����ǂݍ��ށB
//ina// 			joseki.reset(new Joseki((char *)bookFileStr.c_str()));
//ina// 		}

		KyokumenKomagumi k; // ���݂̋ǖ�
		KomaInf customBan[9][9]; // ���菉���ǖʈȊO�̋ǖʂ���J�n���鎞�Ɏg���B
		memset(customBan, 0, sizeof(customBan));

		// ����͂܂��ȒP�Ȏv�l���Ȃ̂ŁA���������ȒP�ł��B
		Sikou sikou;
		sikou.ClearHash(); // �n�b�V����S�ăN���A����B�i�K�v���ǂ����s�������ǁj
		hashCount = 0; // �n�b�V���̎g�p����0�ɖ߂��Ă����B
		isStopReceived = false;

		// �����̋ǖʂŁA�ő�̎萔�͂T�V�X�肾�����ł��B
		Te teBuf[600];
		int SorE; // ���̎�ԁBSELF or ENEMY
		int teNum; // ����\�Ȏ萔
		Te te(0); // �G���W�����Ԃ��w����
		Te ponderTe(0); // �G���W���̎w����ɑ΂��鑊��̗\�z��
		InaniwaTimeTesu = INANIWA_MAX_TESU; //ina//
		InaniwaKomagumiTesu = 9; //ina//

		while (true) {
			// �΋ǂ̃��[�v�B�΋ǎ��̃R�}���h�̂����͂��̒��ōs����B
			// gameover�R�}���h�������Ă����炱�̃��[�v�𔲂��đ΋Ǒ҂��̃��[�v�ɖ߂�B

			string commandStr = WaitCommand(); // ��M�����R�}���h���擾����B
			const char *buf = commandStr.c_str();
			int len = strlen(buf);
			if (strncmp(buf, "position", strlen("position")) == 0) {
				// ���݋ǖʂ��w�肷��R�}���h�B
				if (strncmp(buf, "position startpos", strlen("position startpos")) == 0) {
					// ���菉���ǖʂ���̊J�n
					k.InitKyokumen(0, HirateBan, HirateMotigoma);
					k.Initialize();
					SorE = SELF;
					teNum = k.MakeLegalMoves(SorE, teBuf);
					if (commandStr.find(" moves ") != string::npos) {
						// �J�n�ǖʌ�̎w����̏�񂪂���ꍇ�A�ǖʂ�i�߂�B
						SorE = USIUtil::AddAllMoves(buf, len, &k, SorE, teNum, teBuf);
					}
				} else {
					// SFEN�Ŏw�肳���ǖʂ���̊J�n
					USIUtil::ClearMochigoma(customMotigoma); // �����������������K�v������B
					USIUtil::MakeCustomKyokumen(buf, customBan, customMotigoma); // �J�n�ǖʏ�����
					k.InitKyokumen(0, customBan, customMotigoma);
					k.Initialize();

					SorE = commandStr.find(" w ") != string::npos ? ENEMY : SELF;
					// commandStr.find(" b ") != string::npos�Ƃ����������g���ƁA���̎�����p������
					// �ꍇ��" b "�Ƃ��������񂪊܂܂�Ă��܂��̂ŁA�K�����ԂƔ��f���Ă��܂���肪�������B
					// �����h�����߂�commandStr.find(" w ") != string::npos�Ƃ����������g���悤�ɕύX�B

					teNum = k.MakeLegalMoves(SorE, teBuf);
					if (commandStr.find(" moves ") != string::npos) {
						// �J�n�ǖʌ�̎w����̏�񂪂���ꍇ�A�ǖʂ�i�߂�B
						SorE = USIUtil::AddAllMoves(buf, len, &k, SorE, teNum, teBuf);
					}
					isUseJoseki = false; // ����̒�Ղ��Ȃ��̂�false�ɂ��Ă����B
				}
				continue;
			} else if (strncmp(buf, "go", strlen("go")) == 0 &&
					strncmp(buf, "go ponder", strlen("go ponder")) != 0 &&
					strncmp(buf, "go mate", strlen("go mate")) != 0) {
				const char *pbuf;                                                                        //ina//
				if ( (SorE == SELF  && (pbuf = strstr(buf, "wtime ")) != NULL) ||                        //ina//
					 (SorE == ENEMY && (pbuf = strstr(buf, "btime ")) != NULL)  ) {                      //ina//
					int tm = atoi(pbuf+6)/1000;                                                          //ina//
					if (tm >= 3600) {                                                                    //ina//
						printf("info string ����̎c�莞�� %02d:%02d:%02d\n", tm/3600, tm/60%60, tm%60); //ina//
					} else {                                                                             //ina//
						printf("info string ����̎c�莞�� %02d:%02d\n", tm/60, tm%60);                  //ina//
					}                                                                                    //ina//
				}                                                                                        //ina//
				// ���݋ǖʂ���̎v�l�J�n�B
				if (teNum == 0) {
					printf("bestmove resign\n");
					continue;
				}
				if (k.IsNyugyokuWin(SorE)) {
					// CSA�̓��ʃ��[���ŏ����̋ǖʂȂ�bestmove win��Ԃ��B
					printf("bestmove win\n");
					continue;
				}
				isPonderThink = false;
				USIUtil::ParseAllTimes(buf, SorE);
				if (strncmp(buf, "go infinite", strlen("go infinite")) == 0) {
					thinkDepthMax = 8; // go infinite�̏ꍇ����8�ɂ���B
				}
				k.SenkeiInit(); // sikou.Think()�̑O�ɂ��ꂪ�K�v�B
				te = sikou.Think(SorE, k, isUseJoseki, &ponderTe);

				if (isStopReceived || isInfinite) {
					// �v�l����stop�����Ă����ꍇ�A��������go infinite�Ŏv�l���J�n�����ꍇ�́A�����ł͎��Ԃ����A
					// ���̒����if (strncmp(buf, "stop", strlen("stop")) == 0)�̒��ŕԂ��B
					continue;
				} else {
					// �w�����Ԃ��B
					printf("bestmove ");
					te.Print();
					if (canPonder && !ponderTe.IsNull()) {
						printf(" ponder ");
						ponderTe.Print();
					}
					printf("\n");
				}

				// �ǖʂ�i�߂�B
				k.Move(SorE,te);
				SorE = SorE == SELF ? ENEMY : SELF;
				teNum = k.MakeLegalMoves(SorE,teBuf);
				continue;
			} else if (strncmp(buf, "go ponder", strlen("go ponder")) == 0) {
				// �\�z�����ǖʂ���̎v�l�J�n�B��ǂ݂Ȃ̂ŁA���̒��Ŏ��Ԃ��Ă͂����Ȃ��B
				if (teNum == 0) {
					continue;
				}
				isPonderThink = true;
				USIUtil::ParseAllTimes(buf, SorE);
				k.SenkeiInit(); // sikou.Think()�̑O�ɂ��ꂪ�K�v�B
				te = sikou.Think(SorE, k, isUseJoseki, &ponderTe);
				continue;
			} else if (strncmp(buf, "go mate", strlen("go mate")) == 0) {
				// �l�����̎v�l�J�n�B
				isTsumeThink = true;
				USIUtil::ParseLimitTimes(buf);
				thinkStartTime = timeGetTime();
				try {
					int val = k.Mate(SorE, 30, te); // �[��30�܂ŒT���B
					if (val == 1) { // �l�񂾏ꍇ
						printf("checkmate");
						int startSorE = SorE;
						Te tsumeTe;
						while (teNum > 0) {
							printf(" ");
							if (SorE == startSorE) {
								// �l�߂����n�b�V������擾����B
								tsumeTe = k.GetTsumeTe(SorE);
							} else {
								// �󂯂��͂ǂꂪ�őP���킩��Ȃ��̂ŁA�\�Ȏ�̂�����
								// �ŏ��ɐ���������ɂ���B
								tsumeTe = teBuf[0];
							}
							tsumeTe.Print();
							k.Move(SorE, tsumeTe);
							SorE = SorE == SELF ? ENEMY : SELF;
							teNum = k.MakeLegalMoves(SorE,teBuf);
						}
					} else { // �l�܂Ȃ������ꍇ
						printf("checkmate nomate");
					}
				} catch (int) { // ���Ԑ؂�̏ꍇ
					printf("checkmate timeout");
				}
				printf("\n");
				isTsumeThink = false;
				continue;
			} else if (strncmp(buf, "stop", strlen("stop")) == 0) {
				isStopReceived = false; // false�ɖ߂��B

				// go�܂���go ponder�Ŋ��ɒT���ς݂̎��Ԃ��B
				// go�ɑ΂���stop�ł���Ί��ɋǖʂ��i�߂��Ă��邵�A
				// go ponder�ɑ΂���stop�ł���΋ǖʂ�i�߂�K�v�͂Ȃ��i����position��
				// ����������Ă��܂��j�̂ŁA�ǖʂɊւ��Ă͉����ύX���Ȃ��Ă悢�B
				printf("bestmove ");
				if (teNum == 0) {
					// ��ǂ݊J�n���_�ŋl��ł����瓊������B
					printf("resign\n");
					continue;
				}
				te.Print();
				if (canPonder && !ponderTe.IsNull()) {
					printf(" ponder ");
					ponderTe.Print();
				}
				printf("\n");
				continue;
			} else if (strncmp(buf, "ponderhit", strlen("ponderhit")) == 0) {
				// �����ɓ���̂�bestmove ... ponder ...�ŕԂ����\�z�肪�����������B
				// �\�z��܂ł̋ǖʂ͐i��ł��邪�A�������w������Ɋւ��Ă͂����ŋǖʂ�i�߂�K�v������B

				// ����ponderhit����M����O��0�ɖ߂��Ă��Bbestmove��Ԃ��ƁA���̒����ponderhit��
				// �����Ă��邱�Ƃ�����A����ɂ���Ă܂�ponderhitReceiveTime��ݒ肷��̂ŁA
				// �����0�ɂ��Ă��܂�Ȃ��悤�Abestmove��Ԃ��O��0�ɖ߂��Ȃ���΂����Ȃ��B
				ponderhitReceiveTime = 0;

				if (isStopReceived) {
					// �l�ԑ΃G���W���ŁA�G���W�����\�z�������l�Ԃ��w����ponderhit�������A
					// ���̌�A�܂��G���W�����p�����Ďv�l���Ă��鎞�Ɂu�����w������v�{�^���������ƁA
					// stop�������Ă���B���̂悤�ɁAponderhit��stop�������đ����Ă����ꍇ�A
					// �����ł͉����Ԃ����A���̎���stop�ɓ������Ƃ���Ŏ��Ԃ��B
					continue;
				}
				if (teNum == 0) {
					// teNum==0�Ƃ������Ƃ́Ago ponder�̑���̗\�z��ɂ���ċl��ł��܂����Ƃ������ƂȂ̂ŁA
					// ponderhit�ɂ���Ă��̒ʂ�̎肪�����瓊������B
					printf("bestmove resign\n");
					continue;
				}
				k.Move(SorE,te);
				SorE = SorE == SELF ? ENEMY : SELF;
				teNum = k.MakeLegalMoves(SorE,teBuf);

				// ���ɒT���ς݂̎��Ԃ��B
				printf("bestmove ");
				te.Print();
				if (canPonder && !ponderTe.IsNull()) {
					printf(" ponder ");
					ponderTe.Print();
				}
				printf("\n");
				continue;
			} else if (strncmp(buf, "gameover", strlen("gameover")) == 0) {
				// gameover�Ƃ����R�}���h��USI�̌��Ăɂ͂Ȃ����A�΋ǏI�����G���W����
				// �m�点�邽�߂ɓƎ��ɒǉ������B�������M�����炱�̃��[�v�𔲂���
				// �΋Ǒ҂��̃��[�v�ɖ߂�B
				// gameover [ win | lose | draw ]�Ƃ����悤�ɁAgameover�̂��ƂɌ��ʒʒm��
				// �p�����[�^������̂ŁA�����w�K����̂ł���΂�����g����B
				break;
			}
		}
	}
	return 0;
}
