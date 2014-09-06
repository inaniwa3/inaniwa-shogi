// USIに対応するためのグローバル変数を追加しているうちに数が増えてわかりにくくなったので、
// グローバル変数は全部このファイルで定義することにする。

#include "Kyokumen.h" //ina//

bool isStopReceived = false; // stopコマンドを受信したか
bool canPonder = false; // 先読み可能か
bool canThrow = false; // 思考中断が可能かどうか
bool isPonderThink = false; // 先読み思考中か
bool isTsumeThink = false; // 詰将棋解答思考中か

unsigned long thinkStartTime; // 思考を開始した時刻
unsigned long ponderhitReceiveTime = 0; // ponderhitを受信した時刻

unsigned long evaluatedNodes; // KyokumenKomagumi::Evaluate()が呼ばれた回数
unsigned long hashCount = 0; // ハッシュに追加された数

unsigned long remainTime = 0; // 残り時間
unsigned long byoyomiTime = 0; // 秒読みの時間
unsigned long tsumeLimitTime = 0; // 詰将棋解答の制限時間
bool isInfinite = false; // 思考時間が無制限かどうか
//ina// int thinkDepthMax = 4; // 読みの最大深さ
int thinkDepthMax = 5; // 読みの最大深さ //ina//

int InaniwaTimeTesu;     //ina//
int InaniwaKomagumiTesu; //ina//
Te  InaniwaLastTe;       //ina//
