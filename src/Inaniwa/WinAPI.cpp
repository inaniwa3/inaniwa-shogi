#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

// Windows�ɌŗL�̊֐���Linux�Ŏg����悤�Ɏ��������B

// ���ݎ������~���b�P�ʂŎ擾����B
// tv.tv_sec * 1000�̒l���I�[�o�[�t���[���Ă��܂����A�����Ǝ����̍������悤��
// �g����������Ȃ���͂Ȃ��͂��B
unsigned long timeGetTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long currTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return currTime;
}

// �����ݒ�t�@�C������L�[�ɑΉ�������������擾����B
// ���̎����ł�lpAppName�i�Z�N�V�������j���g�p���Ă��Ȃ����AWindows�Ŏg���Ƃ���
// �K���w�肷��K�v�����邱�Ƃɒ��ӁB
unsigned long GetPrivateProfileString
(
	const char* /* lpAppName */,	// �Z�N�V�������i���g�p�j
	const char* lpKeyName,	// �L�[�̖��O
	const char* lpDefault,	// �L�[�����݂��Ȃ��ꍇ�Ɏg���f�t�H���g�̕�����
	char* lpReturnedString,	// �R�s�[��̃o�b�t�@
	unsigned long nSize,	// �o�b�t�@�̃T�C�Y
	const char* lpFileName	// �����ݒ�t�@�C���̃p�X��
)
{
	if (lpKeyName == NULL || lpDefault == NULL || lpReturnedString == NULL || lpFileName == NULL) {
		return 0;
	}
	if (strlen(lpKeyName) == 0 || strlen(lpFileName) == 0) {
		return 0;
	}
	// �ŏ��Ƀf�t�H���g�̕�������R�s�[����B
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
			// �L�[����n�܂�s������������A���̒l���擾����B
			string val = line.substr(keyStr.length());
			len = min((unsigned long)strlen(val.c_str()), nSize - 1);
			strncpy(lpReturnedString, val.c_str(), len);
			lpReturnedString[len] = 0;
			break;
		}
	}
	return len;
}

// �����ݒ�t�@�C���ɃL�[�ƑΉ����镶�����ۑ�����B
// ���̎����ł�lpAppName�i�Z�N�V�������j���g�p���Ă��Ȃ����AWindows�Ŏg���Ƃ���
// �K���w�肷��K�v�����邱�Ƃɒ��ӁB
bool WritePrivateProfileString
(
	const char* /* lpAppName */,	// �Z�N�V�������i���g�p�j
	const char* lpKeyName,	// �L�[�̖��O
	const char* lpString,	// �L�[�ɑΉ����镶����
	const char* lpFileName	// �����ݒ�t�@�C���̃p�X��
)
{
	if (lpKeyName == NULL || lpString == NULL || lpFileName == NULL) {
		return false;
	}
	if (strlen(lpKeyName) == 0 || strlen(lpFileName) == 0) {
		return false;
	}

	// �X�V����s���쐬���Ă����B
	string keyStr = lpKeyName;
	keyStr += "=";
	string valStr = lpString;
	string newLine = keyStr + valStr;

	vector<string> lineVector; // �t�@�C���̍s��ۑ����邽�߂�Vector
	bool keyFound = false;
	{	// �t�@�C���ɏ������ޑO��ifstream�̃f�X�g���N�^���Ă΂��悤�A�u���[�X�ň͂��Ă����B
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
						// �L�[����n�܂�s������������A���̍s��u��������B
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
