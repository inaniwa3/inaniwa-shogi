#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

// Windowsに固有の関数をLinuxで使えるように実装した。

// 現在時刻をミリ秒単位で取得する。
// tv.tv_sec * 1000の値がオーバーフローしてしまうが、時刻と時刻の差を取るような
// 使い方をするなら問題はないはず。
unsigned long timeGetTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long currTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return currTime;
}

// 初期設定ファイルからキーに対応した文字列を取得する。
// この実装ではlpAppName（セクション名）を使用していないが、Windowsで使うときは
// 必ず指定する必要があることに注意。
unsigned long GetPrivateProfileString
(
	const char* /* lpAppName */,	// セクション名（未使用）
	const char* lpKeyName,	// キーの名前
	const char* lpDefault,	// キーが存在しない場合に使うデフォルトの文字列
	char* lpReturnedString,	// コピー先のバッファ
	unsigned long nSize,	// バッファのサイズ
	const char* lpFileName	// 初期設定ファイルのパス名
)
{
	if (lpKeyName == NULL || lpDefault == NULL || lpReturnedString == NULL || lpFileName == NULL) {
		return 0;
	}
	if (strlen(lpKeyName) == 0 || strlen(lpFileName) == 0) {
		return 0;
	}
	// 最初にデフォルトの文字列をコピーする。
	unsigned long len = min((unsigned long)strlen(lpDefault), nSize - 1);
	strncpy(lpReturnedString, lpDefault, len);
	lpReturnedString[len] = 0;

	ifstream ifs(lpFileName);
	if (ifs.fail()) {
		return len;
	}
	string keyStr = lpKeyName;
	keyStr += "=";
	while (!ifs.eof()) {
		string line;
		ifs >> line;
		if (line == "") {
			continue;
		}
		if (line.find(keyStr) == 0) {
			// キーから始まる行が見つかったら、その値を取得する。
			string val = line.substr(keyStr.length());
			len = min((unsigned long)strlen(val.c_str()), nSize - 1);
			strncpy(lpReturnedString, val.c_str(), len);
			lpReturnedString[len] = 0;
			break;
		}
	}
	return len;
}

// 初期設定ファイルにキーと対応する文字列を保存する。
// この実装ではlpAppName（セクション名）を使用していないが、Windowsで使うときは
// 必ず指定する必要があることに注意。
bool WritePrivateProfileString
(
	const char* /* lpAppName */,	// セクション名（未使用）
	const char* lpKeyName,	// キーの名前
	const char* lpString,	// キーに対応する文字列
	const char* lpFileName	// 初期設定ファイルのパス名
)
{
	if (lpKeyName == NULL || lpString == NULL || lpFileName == NULL) {
		return false;
	}
	if (strlen(lpKeyName) == 0 || strlen(lpFileName) == 0) {
		return false;
	}

	// 更新する行を作成しておく。
	string keyStr = lpKeyName;
	keyStr += "=";
	string valStr = lpString;
	string newLine = keyStr + valStr;

	vector<string> lineVector; // ファイルの行を保存するためのVector
	bool keyFound = false;
	{	// ファイルに書き込む前にifstreamのデストラクタが呼ばれるよう、ブレースで囲っておく。
		ifstream ifs(lpFileName);
		if (!ifs.fail()) {
			while (!ifs.eof()) {
				string line;
				ifs >> line;
				if (line == "") {
					continue;
				}
				if (line.find(keyStr) == 0) {
					if (!keyFound) {
						// キーから始まる行が見つかったら、その行を置き換える。
						lineVector.push_back(newLine);
						keyFound = true;
					}
				} else {
					lineVector.push_back(line);
				}
			}
		}
	}
	if (!keyFound) {
		lineVector.push_back(newLine);
	}

	ofstream ofs(lpFileName);
	if (ofs.fail()) {
        return false;
	}
	for (vector<string>::iterator iter = lineVector.begin(); iter != lineVector.end(); iter++) {
		ofs << *iter << endl;
	}
	return true;
}
