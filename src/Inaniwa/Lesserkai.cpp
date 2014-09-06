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

extern bool isStopReceived; // stopコマンドを受信したか
extern bool canPonder; // 先読み可能か
extern bool isPonderThink; // 先読み思考中か
extern bool isTsumeThink; // 詰将棋解答思考中か
extern unsigned long thinkStartTime; // 思考を開始した時刻
extern unsigned long hashCount; // ハッシュに追加された数
extern unsigned long ponderhitReceiveTime; // ponderhitを受信した時刻
extern bool isInfinite; // 思考時間が無制限かどうか
extern int thinkDepthMax; // 読みの最大深さ
extern int InaniwaTimeTesu;     //ina//
extern int InaniwaKomagumiTesu; //ina//
extern Te  InaniwaLastTe;       //ina//

// エンジン設定ファイルのパス
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

queue<string> commandQueue; // 受信したコマンドを追加するためのキュー

CRITICAL_SECTION cs; // クリティカルセクションオブジェクト
// commandQueueに対して、ReceiveThread()とWaitCommand()から
// 同時にアクセスできないよう、commandQueueにアクセスする部分は
// クリティカルセクションにする。

// コマンド受信のための関数。思考中でも受信できるように、スレッドからこの関数を呼び出す。
#ifdef _WIN32
void ReceiveThread(void *)
#else
void *ReceiveThread(void *)
#endif
{
	char buf[10000];
	while (true) {
		fgets(buf, 10000, stdin);
		buf[strlen(buf) - 1] = 0; // 改行コードを消す。

		if (strncmp(buf, "quit", strlen("quit")) == 0) {
			while (commandQueue.size() > 0) {
				// エンジン設定ダイアログでOKを押すと、setoptionとquitが続けて送られてくる。
				// その時、setoptionの内容を実行する前にexit()を呼ばないよう、quitコマンドより
				// 前に受信したコマンドが全て実行されてから終了するようにする。
				// 本当は、commandQueueのサイズが0だからといって、それ以前のコマンドが本当に
				// 実行されたという保証はないが（popしただけで、まだ実行前の可能性もあるので）、
				// setoptionに関しては実行に200msもかかることはないので、この対策だけで済ませる
				// ことにする。
				Sleep(200);
			}
			// 終了前にクリティカルセクションオブジェクトを破棄。
			DeleteCriticalSection(&cs);
			// quitを受信したらエンジンを終了する。
			exit(0);
		}
		EnterCriticalSection(&cs); // クリティカルセクションに入る。
		if (strncmp(buf, "stop", strlen("stop")) == 0) {
			isStopReceived = true;
		}
		if (strncmp(buf, "ponderhit", strlen("ponderhit")) == 0) {
			ponderhitReceiveTime = timeGetTime();
		}
		if (strncmp(buf, "gameover", strlen("gameover")) == 0) {
			isStopReceived = true; // 先読み中にgameoverが送られてきた場合、すぐ思考を打ち切る。
		}
		string commandStr = buf;
		commandQueue.push(commandStr); // コマンドをキューに追加。
		LeaveCriticalSection(&cs); // クリティカルセクションを出る。
	}
}

// 受信したコマンドを返す。
string WaitCommand()
{
	while (commandQueue.empty()) {
		Sleep(100);
	}
	EnterCriticalSection(&cs); // クリティカルセクションに入る。
	string commandStr = commandQueue.front(); // キューからコマンドを取得。
	commandQueue.pop(); // キューからコマンドを削除。
	LeaveCriticalSection(&cs); // クリティカルセクションを出る。
	return commandStr;
}

// コマンド受信スレッドを開始する。
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
	setvbuf(stdout, NULL, _IONBF, 0); // これが必要（標準出力のバッファリングを無効にする）

	InitializeCriticalSection(&cs); // クリティカルセクションオブジェクトの初期化。
	bool isSuccess = BeginThread(); // 受信スレッド開始。
	if (!isSuccess) {
		printf("Failed to begin thread\n");
		return -1;
	}

	// Lesserkai.iniファイルがあるなら、エンジン設定のデフォルト値を取得。
//ina// 	char iniValueBuf[256];
//ina// 	::GetPrivateProfileString("CustomSettings", "BookFile", "public.bin", iniValueBuf, 256, iniFilePath);
//ina// 	string bookFileStr = iniValueBuf;
//ina// 	::GetPrivateProfileString("CustomSettings", "UseBook", "true", iniValueBuf, 256, iniFilePath);
//ina// 	string useBookStr = iniValueBuf;
//ina// 	bool isUseJoseki = useBookStr == "true";
	bool isUseJoseki = false; //ina//

	srand((unsigned)time(NULL)); // Sikou::Think()で定跡をランダムに選べるようにするため。
	Kyokumen::HashInit();
	
	// 平手の初期配置です。見やすいでしょ？変換はその分複雑ですけど。
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
	// こちらは面倒でもEHIまで0を並べないといけません。
	int HirateMotigoma[EHI+1]={
	// 空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍空歩香桂銀金角飛
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
	// 平手初期局面以外から開始する時に使う。
	int customMotigoma[EHI+1]={
	// 空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍空歩香桂銀金角飛
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
	KyokumenKomagumi::InitKanagomaValue();
	shoki=new Kyokumen(0, HirateBan, HirateMotigoma);

	while (true) {
		while (true) {
			// 対局待ちのループ。setoptionコマンドが送られてきたらその値を設定する。
			// usinewgameコマンドが送られてきたらこのループを抜けて対局のループに入る。

			string commandStr = WaitCommand(); // 受信したコマンドを取得する
			const char *buf = commandStr.c_str();
			int len = strlen(buf);
			if (strncmp(buf, "usi", len) == 0) {
				// エンジン起動時、一番最初に呼ばれるコマンド。これに対してエンジンのidを返したあと、
				// エンジンで設定可能なパラメータをoptionコマンドで返し、最後にusiokを返す必要がある。
//ina// 				printf("id name Lesserkai 1.3.3\n");
				printf("id name inaniwa 1.0.0\n"); //ina//
//ina// 				printf("id author Program Writer\n");
				printf("id author inaniwa3\n"); //ina//
//ina// 				printf("option name BookFile type string default %s\n", bookFileStr.c_str());
//ina// 				printf("option name UseBook type check default %s\n", isUseJoseki ? "true" : "false");
				printf("usiok\n");
			} else if (strncmp(buf, "isready", len) == 0) {
				// 本当はメモリがちゃんと確保できたかどうか調べてからreadyokを返すべきだが
				// 面倒なので何も調べずにreadyokを返している。
				printf("readyok\n");
			} else if (strncmp(buf, "setoption", strlen("setoption")) == 0) {
				// エンジンに設定するパラメータが送られてくるのでそれを設定し、
				// エンジン固有のパラメータであれば初期設定ファイルに保存する。
				if (strncmp(buf, "setoption name USI_Ponder value true", len) == 0) {
					canPonder = true;
				} else if (strncmp(buf, "setoption name USI_Ponder value false", len) == 0) {
					canPonder = false;
				} else if (strncmp(buf, "setoption name USI_Hash value ", strlen("setoption name USI_Hash value ")) == 0) {
					string hashStr = buf + strlen("setoption name USI_Hash value ");
					// int hashSize = atoi(hashStr.c_str());
					// 本来であれば、この値を元にハッシュサイズを設定すべきである。
//ina// 				} else if (strncmp(buf, "setoption name BookFile value ", strlen("setoption name BookFile value ")) == 0) {
//ina// 					bookFileStr = buf + strlen("setoption name BookFile value ");
//ina// 					// 初期設定ファイルに保存しておく
//ina// 					::WritePrivateProfileString("CustomSettings", "BookFile", bookFileStr.c_str(), iniFilePath);
//ina// 				} else if (strncmp(buf, "setoption name UseBook value ", strlen("setoption name UseBook value ")) == 0) {
//ina// 					useBookStr = buf + strlen("setoption name UseBook value ");
//ina// 					isUseJoseki = useBookStr == "true";
//ina// 					// 初期設定ファイルに保存しておく
//ina// 					::WritePrivateProfileString("CustomSettings", "UseBook", isUseJoseki ? "true" : "false", iniFilePath);
				}
			} else if (strncmp(buf, "usinewgame", len) == 0) {
				// 新規対局開始のコマンド。これを受信したらこのループを抜ける。
				break;
			}
		}

//ina// 		if (joseki.get() == NULL) {
//ina// 			// 定跡ファイルが指定されていて、まだそれを読んでいなかったら、そのファイルを読み込む。
//ina// 			joseki.reset(new Joseki((char *)bookFileStr.c_str()));
//ina// 		}

		KyokumenKomagumi k; // 現在の局面
		KomaInf customBan[9][9]; // 平手初期局面以外の局面から開始する時に使う。
		memset(customBan, 0, sizeof(customBan));

		// これはまだ簡単な思考部なので、初期化も簡単です。
		Sikou sikou;
		sikou.ClearHash(); // ハッシュを全てクリアする。（必要かどうか不明だけど）
		hashCount = 0; // ハッシュの使用数も0に戻しておく。
		isStopReceived = false;

		// 将棋の局面で、最大の手数は５７９手だそうです。
		Te teBuf[600];
		int SorE; // 次の手番。SELF or ENEMY
		int teNum; // 着手可能な手数
		Te te(0); // エンジンが返す指し手
		Te ponderTe(0); // エンジンの指し手に対する相手の予想手
		InaniwaTimeTesu = INANIWA_MAX_TESU; //ina//
		InaniwaKomagumiTesu = 9; //ina//

		while (true) {
			// 対局のループ。対局時のコマンドのやり取りはこの中で行われる。
			// gameoverコマンドが送られてきたらこのループを抜けて対局待ちのループに戻る。

			string commandStr = WaitCommand(); // 受信したコマンドを取得する。
			const char *buf = commandStr.c_str();
			int len = strlen(buf);
			if (strncmp(buf, "position", strlen("position")) == 0) {
				// 現在局面を指定するコマンド。
				if (strncmp(buf, "position startpos", strlen("position startpos")) == 0) {
					// 平手初期局面からの開始
					k.InitKyokumen(0, HirateBan, HirateMotigoma);
					k.Initialize();
					SorE = SELF;
					teNum = k.MakeLegalMoves(SorE, teBuf);
					if (commandStr.find(" moves ") != string::npos) {
						// 開始局面後の指し手の情報がある場合、局面を進める。
						SorE = USIUtil::AddAllMoves(buf, len, &k, SorE, teNum, teBuf);
					}
				} else {
					// SFENで指定される局面からの開始
					USIUtil::ClearMochigoma(customMotigoma); // 持ち駒を初期化する必要がある。
					USIUtil::MakeCustomKyokumen(buf, customBan, customMotigoma); // 開始局面初期化
					k.InitKyokumen(0, customBan, customMotigoma);
					k.Initialize();

					SorE = commandStr.find(" w ") != string::npos ? ENEMY : SELF;
					// commandStr.find(" b ") != string::nposという条件を使うと、後手の持ち駒が角だけの
					// 場合に" b "という文字列が含まれてしまうので、必ず先手番と判断してしまう問題があった。
					// それを防ぐためにcommandStr.find(" w ") != string::nposという条件を使うように変更。

					teNum = k.MakeLegalMoves(SorE, teBuf);
					if (commandStr.find(" moves ") != string::npos) {
						// 開始局面後の指し手の情報がある場合、局面を進める。
						SorE = USIUtil::AddAllMoves(buf, len, &k, SorE, teNum, teBuf);
					}
					isUseJoseki = false; // 駒落ちの定跡がないのでfalseにしておく。
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
						printf("info string 相手の残り時間 %02d:%02d:%02d\n", tm/3600, tm/60%60, tm%60); //ina//
					} else {                                                                             //ina//
						printf("info string 相手の残り時間 %02d:%02d\n", tm/60, tm%60);                  //ina//
					}                                                                                    //ina//
				}                                                                                        //ina//
				// 現在局面からの思考開始。
				if (teNum == 0) {
					printf("bestmove resign\n");
					continue;
				}
				if (k.IsNyugyokuWin(SorE)) {
					// CSAの入玉ルールで勝ちの局面ならbestmove winを返す。
					printf("bestmove win\n");
					continue;
				}
				isPonderThink = false;
				USIUtil::ParseAllTimes(buf, SorE);
				if (strncmp(buf, "go infinite", strlen("go infinite")) == 0) {
					thinkDepthMax = 8; // go infiniteの場合だけ8にする。
				}
				k.SenkeiInit(); // sikou.Think()の前にこれが必要。
				te = sikou.Think(SorE, k, isUseJoseki, &ponderTe);

				if (isStopReceived || isInfinite) {
					// 思考中にstopが来ていた場合、もしくはgo infiniteで思考を開始した場合は、ここでは手を返さず、
					// この直後のif (strncmp(buf, "stop", strlen("stop")) == 0)の中で返す。
					continue;
				} else {
					// 指し手を返す。
					printf("bestmove ");
					te.Print();
					if (canPonder && !ponderTe.IsNull()) {
						printf(" ponder ");
						ponderTe.Print();
					}
					printf("\n");
				}

				// 局面を進める。
				k.Move(SorE,te);
				SorE = SorE == SELF ? ENEMY : SELF;
				teNum = k.MakeLegalMoves(SorE,teBuf);
				continue;
			} else if (strncmp(buf, "go ponder", strlen("go ponder")) == 0) {
				// 予想した局面からの思考開始。先読みなので、この中で手を返してはいけない。
				if (teNum == 0) {
					continue;
				}
				isPonderThink = true;
				USIUtil::ParseAllTimes(buf, SorE);
				k.SenkeiInit(); // sikou.Think()の前にこれが必要。
				te = sikou.Think(SorE, k, isUseJoseki, &ponderTe);
				continue;
			} else if (strncmp(buf, "go mate", strlen("go mate")) == 0) {
				// 詰将棋の思考開始。
				isTsumeThink = true;
				USIUtil::ParseLimitTimes(buf);
				thinkStartTime = timeGetTime();
				try {
					int val = k.Mate(SorE, 30, te); // 深さ30まで探索。
					if (val == 1) { // 詰んだ場合
						printf("checkmate");
						int startSorE = SorE;
						Te tsumeTe;
						while (teNum > 0) {
							printf(" ");
							if (SorE == startSorE) {
								// 詰める手をハッシュから取得する。
								tsumeTe = k.GetTsumeTe(SorE);
							} else {
								// 受ける手はどれが最善かわからないので、可能な手のうちの
								// 最初に生成した手にする。
								tsumeTe = teBuf[0];
							}
							tsumeTe.Print();
							k.Move(SorE, tsumeTe);
							SorE = SorE == SELF ? ENEMY : SELF;
							teNum = k.MakeLegalMoves(SorE,teBuf);
						}
					} else { // 詰まなかった場合
						printf("checkmate nomate");
					}
				} catch (int) { // 時間切れの場合
					printf("checkmate timeout");
				}
				printf("\n");
				isTsumeThink = false;
				continue;
			} else if (strncmp(buf, "stop", strlen("stop")) == 0) {
				isStopReceived = false; // falseに戻す。

				// goまたはgo ponderで既に探索済みの手を返す。
				// goに対するstopであれば既に局面が進められているし、
				// go ponderに対するstopであれば局面を進める必要はない（次のpositionで
				// 初期化されてしまう）ので、局面に関しては何も変更しなくてよい。
				printf("bestmove ");
				if (teNum == 0) {
					// 先読み開始時点で詰んでいたら投了する。
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
				// ここに入るのはbestmove ... ponder ...で返した予想手が当たった時。
				// 予想手までの局面は進んでいるが、自分が指した手に関してはここで局面を進める必要がある。

				// 次のponderhitを受信する前に0に戻してやる。bestmoveを返すと、その直後にponderhitが
				// 送られてくることがあり、それによってまたponderhitReceiveTimeを設定するので、
				// それを0にしてしまわないよう、bestmoveを返す前に0に戻さなければいけない。
				ponderhitReceiveTime = 0;

				if (isStopReceived) {
					// 人間対エンジンで、エンジンが予想した手を人間が指すとponderhitが送られ、
					// その後、まだエンジンが継続して思考している時に「すぐ指させる」ボタンを押すと、
					// stopが送られてくる。このように、ponderhitとstopが続けて送られてきた場合、
					// ここでは何も返さず、その次のstopに入ったところで手を返す。
					continue;
				}
				if (teNum == 0) {
					// teNum==0ということは、go ponderの相手の予想手によって詰んでしまったということなので、
					// ponderhitによってその通りの手が来たら投了する。
					printf("bestmove resign\n");
					continue;
				}
				k.Move(SorE,te);
				SorE = SorE == SELF ? ENEMY : SELF;
				teNum = k.MakeLegalMoves(SorE,teBuf);

				// 既に探索済みの手を返す。
				printf("bestmove ");
				te.Print();
				if (canPonder && !ponderTe.IsNull()) {
					printf(" ponder ");
					ponderTe.Print();
				}
				printf("\n");
				continue;
			} else if (strncmp(buf, "gameover", strlen("gameover")) == 0) {
				// gameoverというコマンドはUSIの原案にはないが、対局終了をエンジンに
				// 知らせるために独自に追加した。これを受信したらこのループを抜けて
				// 対局待ちのループに戻る。
				// gameover [ win | lose | draw ]というように、gameoverのあとに結果通知の
				// パラメータがあるので、自動学習するのであればそれを使える。
				break;
			}
		}
	}
	return 0;
}
