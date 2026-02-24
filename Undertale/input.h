//=========================================================
// 
// "入力処理"	[input.h]
// Author : KikuchiMina
// 
//=========================================================
#ifndef _INPUT_H_		// このマクロ定義がされていなければ
#define _INPUT_H_		// 2重インクルード防止のマクロを定義

#include "main.h"

//=========================================================
// マクロ定義
//=========================================================
#define NUM_KEY_MAX		(256)			// キーボードの最大数
#define NUM_JOY_MAX		(JOYKEY_MAX)	// ジョイパッドの最大数
#define MAX_PLAYER		(2)				// 最大人数
#define DEADZONE_LEFTSTICK	(300)			// 左スティックのデッドゾーン
#define DEADZONE_RIGHTSTICK	(300)			// 右スティックのデッドゾーン


//=========================================================
// ジョイパッド構造体の定義
//=========================================================
typedef enum
{
	JOYKEY_UP = 0,				// [00]十字キー(上)
	JOYKEY_DOWN,				// [01]十字キー(下)
	JOYKEY_LEFT,				// [02]十字キー(左)
	JOYKEY_RIGHT,				// [04]十字キー(右)
	JOYKEY_START,				// [05]ボタンキー(START)
	JOYKEY_BACK,				// [06]ボタンキー(BACK)
	JOYKEY_LEFT_PUSH,			// [07]スティックキー(左押し込み)
	JOYKEY_RIGHT_PUSH,			// [08]スティックキー(右押し込み)
	JOYKEY_LB,					// [09]ボタンキー(LB)
	JOYKEY_RB,					// [10]ボタンキー(RB)
	JOYKEY_NONE1,				// [10]ボタンキー(RB)
	JOYKEY_NONE2,				// [10]ボタンキー(RB)
	JOYKEY_A,					// [13]ボタンキー(A)
	JOYKEY_B,					// [14]ボタンキー(B)
	JOYKEY_X,					// [15]ボタンキー(X)
	JOYKEY_Y,					// [16]ボタンキー(Y)
	JOYKEY_LEFT_STICK_UP,		// [17]左スティック(上)
	JOYKEY_LEFT_STICK_DOWN,		// [18]左スティック(下)
	JOYKEY_LEFT_STICK_LEFT,		// [19]左スティック(左)
	JOYKEY_LEFT_STICK_RIGHT,	// [20]左スティック(右)
	JOYKEY_RIGHT_STICK_UP,		// [21]右スティック(上)
	JOYKEY_RIGHT_STICK_DOWN,	// [22]右スティック(下)
	JOYKEY_RIGHT_STICK_LEFT,	// [23]右スティック(左)
	JOYKEY_RIGHT_STICK_RIGHT,	// [24]右スティック(右)
	JOYKEY_MAX
}JOYKEY;

//=========================================================
// マウスのボタンの種類 
//=========================================================
typedef enum
{
	MOUSEKEY_LEFT = 0,		// 左クリック
	MOUSEKEY_RIGHT,			// 右クリック
	MOUSEKEY_WHEEL,			// 中クリック
	MOUSEKEY_MAX
}MOUSEKEY;

//=========================================================
// マウスの傾きの種類
//=========================================================
typedef enum
{
	MOUSESLOPE_LR = 0,		// 左右
	MOUSESLOPE_FB,			// 前後
	MOUSESLOPE_WHEEL,		// マウスホイール
	MOUSESLOPE_MAX
}MOUSESLOPE;

//=========================================================
// 複数人同時使用
//=========================================================
typedef struct
{
	XINPUT_STATE		joykeyState;						// ジョイパッドのプレス情報
	XINPUT_STATE		joykeyStateTrigger;					// ジョイパッドのトリガー情報
	int					nJoykeyStateRepeat[NUM_JOY_MAX];	// ジョイパッドのリピート情報
	XINPUT_STATE 		joykeyStateRelease[NUM_KEY_MAX];	// ジョイパッドのリリース情報
	XINPUT_VIBRATION	vibration;							// ジョイパッドの振動
}XINPUT_PAD;

//=========================================================
// プロトタイプ宣言
//=========================================================
HRESULT InitKeyboard(HINSTANCE hInstance, HWND hWnd);	// キーボードの初期化処理
void UninitKeyboard(void);								// キーボードの終了処理
void UpdateKeyboard(void);								// キーボードの更新処理
bool GetKeyboardPress(int nKey);						// キーボードのプレス情報を取得
bool GetKeyboardTrigger(int nKey);						// キーボードのトリガー情報を取得
bool GetKeyboardRepeat(int nKey);						// キーボードのリピート情報を取得
bool GetKeyboardRelease(int nKey);						// キーボードのリリース情報を取得
														   
HRESULT InitJoypad(void);								// ジョイパッドの初期化処理
void UninitJoypad(void);								// ジョイパッドの終了処理
void UpdateJoypad(void);								// ジョイパッドの更新処理
bool GetJoypadPress(int nPlayer, JOYKEY key);			// ジョイパッドのプレス情報を取得
bool GetJoypadTrigger(int nPlayer, JOYKEY key);			// ジョイパッドのトリガー情報を取得
bool GetJoypadRepeat(int nPlayer, JOYKEY key);			// ジョイパッドのリピート情報を取得
bool GetJoypadRelease(int nPlayer, JOYKEY key);			// ジョイパッドのリリース情報を取得
bool GetJoypadStickLeft(int nPlayer, JOYKEY key);		// ジョイパッドの左スティック情報を取得

bool GetJoypadLeftStick(int nPlayer, D3DXVECTOR3* value);			// ジョイパッドの左スティック情報を取得
bool GetJoypadRightStick(int nPlayer, D3DXVECTOR3* value);			// ジョイパッドの右スティック情報を取得

														   
XINPUT_VIBRATION* GetXInput(void);						// 振動の情報
void SetVibration(int nPlayer, int nLeftMotor, int nRightMotor, int nTime);
void StopVibration(int nPlayer);

HRESULT InitMouse(HWND hWnd);
void UninitMouse(void);
void UpdateMouse(void);
bool GetMousePress(int nButton);
bool GetMouseTrigger(int nButton);
bool GetMouseRelease(int nButton);
POINT GetMousePos(void);
LONG GetMouseMove(MOUSESLOPE slope);

#endif