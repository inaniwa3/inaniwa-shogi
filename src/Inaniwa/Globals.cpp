// USI�ɑΉ����邽�߂̃O���[�o���ϐ���ǉ����Ă��邤���ɐ��������Ă킩��ɂ����Ȃ����̂ŁA
// �O���[�o���ϐ��͑S�����̃t�@�C���Œ�`���邱�Ƃɂ���B

#include "Kyokumen.h" //ina//

bool isStopReceived = false; // stop�R�}���h����M������
bool canPonder = false; // ��ǂ݉\��
bool canThrow = false; // �v�l���f���\���ǂ���
bool isPonderThink = false; // ��ǂݎv�l����
bool isTsumeThink = false; // �l�����𓚎v�l����

unsigned long thinkStartTime; // �v�l���J�n��������
unsigned long ponderhitReceiveTime = 0; // ponderhit����M��������

unsigned long evaluatedNodes; // KyokumenKomagumi::Evaluate()���Ă΂ꂽ��
unsigned long hashCount = 0; // �n�b�V���ɒǉ����ꂽ��

unsigned long remainTime = 0; // �c�莞��
unsigned long byoyomiTime = 0; // �b�ǂ݂̎���
unsigned long tsumeLimitTime = 0; // �l�����𓚂̐�������
bool isInfinite = false; // �v�l���Ԃ����������ǂ���
//ina// int thinkDepthMax = 4; // �ǂ݂̍ő�[��
int thinkDepthMax = 5; // �ǂ݂̍ő�[�� //ina//

int InaniwaTimeTesu;     //ina//
int InaniwaKomagumiTesu; //ina//
Te  InaniwaLastTe;       //ina//
