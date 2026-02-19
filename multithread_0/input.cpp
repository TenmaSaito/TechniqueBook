//================================================================================================================
//
// DirectXの入力処理 [input.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "input.h"
#include "mathUtil.h"

using namespace MyMathUtil;

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define NUM_KEY_MAX			(256)			// キーの最大数
#define NUM_MOUSE_MAX		(3)				// ボタンの最大数
#define THUMB_SLOW			(12)			// GetJoyThumbSlowでの遅延する数
#define STROKE_KEY_MAX		(288)			// キー及びボタン、スティックの最大数
#define REPEAT_TIME			(30)			// リピートの待機期間
#define UP_DEADZONE			(5000)			// 値が正の時のデッドゾーン
#define DOWN_DEADZONE		(-5000)			// 値が負の時のデッドゾーン

//**********************************************************************************
//*** グローバル変数 ***
//**********************************************************************************
LPDIRECTINPUT8 g_pInput = NULL;					// DirectInputオブジェクトのポインタ
LPDIRECTINPUTDEVICE8 g_pDevKeyboard = NULL;		// 入力デバイス(キーボード)へのポインタ
BYTE g_aKeyState[NUM_KEY_MAX];					// キーボードのプレス情報
BYTE g_aKeyStateTrigger[NUM_KEY_MAX];			// キーボードのトリガー情報
BYTE g_aKeyStateRelease[NUM_KEY_MAX];			// キーボードのリリース処理
int g_nCounterRepeat[NUM_KEY_MAX] = {};			// リピートのカウント
bool g_bUseKeyAny;								// キーボードの押下情報

XINPUT_STATE g_joykeyState;						// ジョイパッドのプレス情報
XINPUT_STATE g_joykeyStateTrigger;				// ジョイパッドのトリガー情報
XINPUT_STATE g_joykeyStateRelease;				// ジョイパッドのリリース情報
int g_nCounterRepeatJoypad[JOYKEY_MAX] = {};	// ジョイパッドのリピートのカウント
bool g_bUseJoykeyAny;							// ジョイキーの押下情報
int g_nCounterJoyThumbRepeat[JOYTHUMB_MAX] = {};// ジョイパッドスティックのリピートカウント
int g_nCounterJoyThumb[JOYTHUMB_MAX] = {};		// ジョイパッドのスティックの遅延カウント
XINPUT_VIBRATION g_joyVibration;				// ジョイパッドのバイブレーション情報
_XINPUT_KEYSTROKE g_keyStroke;					// 仮想キーのストローク状態
int g_aStroke[STROKE_KEY_MAX];					// ストロークカウント
int g_nCounterVibretion;						// バイブレーションカウント

LPDIRECTINPUTDEVICE8 g_pDevMouse = NULL;		// 入力デバイス(マウス)へのポインタ

DIMOUSESTATE g_CurrentMouseState;				// マウスの入力情報
DIMOUSESTATE g_PrevMouseState;					// マウスの過去の入力情報
BYTE g_aMouseState[NUM_MOUSE_MAX];				// マウスのプレス情報
BYTE g_aMouseStateTrigger[NUM_MOUSE_MAX];		// マウスのトリガー情報
BYTE g_aMouseStateRelease[NUM_MOUSE_MAX];		// マウスのリリース情報
POINT g_mousePos = {};							// マウスの位置
LONG g_alMouseMoveExacLast[MOUSESLOPE_MAX];		// 過去のマウスのホイールの回転量
LONG g_alMouseMoveCurrent[MOUSESLOPE_MAX];		// 現在のマウスホイールの回転量

HWND g_InputhWnd = NULL;						// 座標変換に使用するウィンドウのハンドル

//================================================================================================================
//キーボードの初期化処理
//================================================================================================================
HRESULT InitKeyboard(HINSTANCE hInstance, HWND hWnd)
{
	// DirectInputオブジェクトの生成
	if (FAILED(DirectInput8Create(hInstance,
								  DIRECTINPUT_VERSION,
								  IID_IDirectInput8,
								  (void**)&g_pInput,
								  NULL)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "DirectInputオブジェクトの生成に失敗しました。");
		return E_FAIL;
	}

	// 入力デバイス(キーボード)の生成
	if (FAILED(g_pInput->CreateDevice(GUID_SysKeyboard,
									  &g_pDevKeyboard,
									  NULL)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "入力デバイス(キーボード)の生成に失敗しました。");
		return E_FAIL;
	}

	// データフォーマットを設定
	if (FAILED(g_pDevKeyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "データフォーマットの設定に失敗しました。");
		return E_FAIL;
	}

	// 協調モードを設定
	if (FAILED(g_pDevKeyboard->SetCooperativeLevel(hWnd,
												   (DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "協調モードの設定に失敗しました。");
		return E_FAIL;
	}

	//キーボードへのアクセス権を獲得
	g_pDevKeyboard->Acquire();

	// ウィンドウハンドルを保存
	g_InputhWnd = hWnd;

	return S_OK;
}

//================================================================================================================
//キーボードの終了処理
//================================================================================================================
void UninitKeyboard(void)
{
	// 入力デバイス(キーボード)の破棄
	if (g_pDevKeyboard != NULL)
	{
		g_pDevKeyboard->Unacquire();
		g_pDevKeyboard->Release();
		g_pDevKeyboard = NULL;
	}

	// DirectInputオブジェクトの破棄
	if (g_pInput != NULL)
	{
		g_pInput->Release();
		g_pInput = NULL;
	}
}

//================================================================================================================
//キーボードの更新処理
//================================================================================================================
void UpdateKeyboard(void)
{
	BYTE aKeyState[NUM_KEY_MAX];			// キーボードの入力情報
	int nCntKey;
	g_bUseKeyAny = false;

	// 入力デバイスからデータを取得
	if (SUCCEEDED(g_pDevKeyboard->GetDeviceState(sizeof(aKeyState), &aKeyState[0])))
	{
		for (nCntKey = 0; nCntKey < NUM_KEY_MAX; nCntKey++)
		{
			g_aKeyStateRelease[nCntKey] = (g_aKeyState[nCntKey] & (g_aKeyState[nCntKey] ^ aKeyState[nCntKey]));			// キーボードのリリース情報を保存
			g_aKeyStateTrigger[nCntKey] = ((aKeyState[nCntKey] ^ g_aKeyState[nCntKey]) & aKeyState[nCntKey]);			// キーボードのトリガー情報を保存
			if (g_aKeyState[nCntKey] != aKeyState[nCntKey])
			{
				g_nCounterRepeat[nCntKey] = 0;
			}
			g_aKeyState[nCntKey] = aKeyState[nCntKey];																	// キーボードのプレス情報を保存
			if (g_aKeyState[nCntKey] & 0x80)
			{
				g_bUseKeyAny = true;
			}
		}			
	}
	else
	{
		g_pDevKeyboard->Acquire();			// キーボードへのアクセス権を取得
	}
}                                              

//================================================================================================================
// キーボードのプレス情報を取得
//================================================================================================================
bool GetKeyboardPress(int nKey)
{
	// 3項演算子
	return (g_aKeyState[nKey] & 0x80) ? true : false;
}

//================================================================================================================
// キーボードのトリガー情報を取得
//================================================================================================================
bool GetKeyboardTrigger(int nKey)
{
	return (g_aKeyStateTrigger[nKey] & 0x80) ? true : false;
}

//================================================================================================================
// キーボードのリリース情報を取得
//================================================================================================================
bool GetKeyboardRelease(int nKey)
{
	return (g_aKeyStateRelease[nKey] & 0x80) ? true : false;
}

//================================================================================================================
// キーボードのリピート情報を取得
//================================================================================================================
bool GetKeyboardRepeat(int nKey, int nCounterRepeat)
{
	g_nCounterRepeat[nKey]++;
	if (g_nCounterRepeat[nKey] < 30)
	{
		return (g_aKeyStateTrigger[nKey] & 0x80) ? true : false;
	}
	else
	{
		if (g_nCounterRepeat[nKey] % nCounterRepeat == 0)
		{
			return (g_aKeyState[nKey] & 0x80) ? true : false;
		}
		else
		{
			return false;
		}
	}
}

//================================================================================================================
// キーボードの押下情報を取得
//================================================================================================================
bool GetKeyboardAny(void)
{
	return g_bUseKeyAny;
}

//================================================================================================================
// キーボードの基本動作情報(WASD)を取得
//================================================================================================================
bool GetKeyboardWASD(void)
{
	if ((g_aKeyState[DIK_W] & 0x80))
	{
		return true;
	}
	else if ((g_aKeyState[DIK_A] & 0x80))
	{
		return true;
	}
	else if ((g_aKeyState[DIK_S] & 0x80))
	{
		return true;
	}
	else if ((g_aKeyState[DIK_D] & 0x80))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//================================================================================================================
// キーボードの基本数字情報(0～9)を取得
//================================================================================================================
int GetKeyboardPressNumber(void)
{
	for (int nCntKey = DIK_1; nCntKey < DIK_MINUS; nCntKey++)
	{
		if (g_aKeyState[nCntKey] & 0x80)
		{
			if (nCntKey != DIK_0)
			{
				return nCntKey - 1;
			}
			else
			{
				return 0x00;
			}
		}
	}

	return false;
}

//================================================================================================================
// キーボードの基本数字情報(0～9)を取得
//================================================================================================================
bool GetKeyboardPressMove(bool *bW, bool *bS, bool *bA, bool *bD)
{
	if (bW)
	{
		*bW = (g_aKeyState[DIK_W] & 0x80) ? true : false;
	}

	if (bS)
	{
		*bS = (g_aKeyState[DIK_S] & 0x80) ? true : false;
	}

	if (bA)
	{
		*bA = (g_aKeyState[DIK_A] & 0x80) ? true : false;
	}

	if (bD)
	{
		*bD = (g_aKeyState[DIK_D] & 0x80) ? true : false;
	}

	if (*bW == false
		&& *bS == false
		&& *bA == false
		&& *bD == false)
	{
		return false;
	}

	return true;
}

//================================================================================================================
// ジョイパッドの初期化処理
//================================================================================================================
HRESULT InitJoypad(void)
{
	// メモリのクリア
	memset(&g_joykeyState, 0, sizeof(XINPUT_STATE));

	memset(&g_joyVibration, 0, sizeof(XINPUT_VIBRATION));

	g_nCounterVibretion = 0;

	// XInputのステート設定(有効)
	XInputEnable(true);

	return S_OK;
}

//================================================================================================================
// ジョイパッドの終了処理
//================================================================================================================
void UninitJoypad(void)
{
	// XInputのステート設定(無効)
	XInputEnable(false);
}

//================================================================================================================
// ジョイパッドの更新処理
//================================================================================================================
void UpdateJoypad(void)
{
	XINPUT_STATE joykeyState;			// 入力情報
	g_bUseJoykeyAny = false;

	// ジョイパッドの状態を取得
	if (XInputGetState(0, &joykeyState) == ERROR_SUCCESS)
	{
		if (g_joykeyState.Gamepad.sThumbLX <= 100.0f && g_joykeyState.Gamepad.sThumbLX >= -100.0f)
		{
			g_nCounterJoyThumb[JOYTHUMB_LX_UP] = 0;
			g_nCounterJoyThumb[JOYTHUMB_LX_DOWN] = 0;
		}

		if (g_joykeyState.Gamepad.sThumbLY <= 100.0f && g_joykeyState.Gamepad.sThumbLY >= -100.0f)
		{
			g_nCounterJoyThumb[JOYTHUMB_LY_UP] = 0;
			g_nCounterJoyThumb[JOYTHUMB_LY_DOWN] = 0;
		}

		if (g_joykeyState.Gamepad.sThumbRX <= 100.0f && g_joykeyState.Gamepad.sThumbRX >= -100.0f)
		{
			g_nCounterJoyThumb[JOYTHUMB_RX_UP] = 0;
			g_nCounterJoyThumb[JOYTHUMB_RX_DOWN] = 0;
		}

		if (g_joykeyState.Gamepad.sThumbRY <= 100.0f && g_joykeyState.Gamepad.sThumbRY >= -100.0f)
		{
			g_nCounterJoyThumb[JOYTHUMB_RY_UP] = 0;
			g_nCounterJoyThumb[JOYTHUMB_RY_DOWN] = 0;
		}

		g_joykeyStateTrigger.Gamepad.wButtons = ((joykeyState.Gamepad.wButtons ^ g_joykeyState.Gamepad.wButtons) & joykeyState.Gamepad.wButtons);
		g_joykeyStateRelease.Gamepad.wButtons = (g_joykeyState.Gamepad.wButtons & (g_joykeyState.Gamepad.wButtons ^ joykeyState.Gamepad.wButtons));
		if (joykeyState.Gamepad.bLeftTrigger > 0 && g_joykeyState.Gamepad.bLeftTrigger == 0)
		{
			g_joykeyStateTrigger.Gamepad.bLeftTrigger = 254;
		}
		else if (joykeyState.Gamepad.bLeftTrigger > 0 && g_joykeyState.Gamepad.bLeftTrigger > 0)
		{
			g_joykeyStateTrigger.Gamepad.bLeftTrigger = 0;
		}
		else if (joykeyState.Gamepad.bLeftTrigger == 0 && g_joykeyState.Gamepad.bLeftTrigger > 0)
		{
			g_joykeyStateRelease.Gamepad.bLeftTrigger = 254;
		}

		if (joykeyState.Gamepad.bRightTrigger > 0 && g_joykeyState.Gamepad.bRightTrigger == 0)
		{
			g_joykeyStateTrigger.Gamepad.bRightTrigger = 254;
		}
		else if (joykeyState.Gamepad.bRightTrigger > 0 && g_joykeyState.Gamepad.bRightTrigger > 0)
		{
			g_joykeyStateTrigger.Gamepad.bRightTrigger = 0;
		}
		else if (joykeyState.Gamepad.bRightTrigger == 0 && g_joykeyState.Gamepad.bRightTrigger > 0)
		{
			g_joykeyStateRelease.Gamepad.bRightTrigger = 254;
		}

		g_joykeyState = joykeyState;	// プレス情報を保存
		
		for (int nCntJoykey = 0; nCntJoykey < JOYKEY_MAX; nCntJoykey++)
		{
			if ((g_joykeyState.Gamepad.wButtons & (0x01 << nCntJoykey)) == false)
			{
				g_nCounterRepeatJoypad[nCntJoykey] = 0;
			}
			else
			{
				g_bUseJoykeyAny = true;
			}
		}

		
		if (g_joykeyState.Gamepad.sThumbLX >= DOWN_DEADZONE && g_joykeyState.Gamepad.sThumbLX <= UP_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[JOYTHUMB_LX_UP] = 0;
			g_nCounterJoyThumbRepeat[JOYTHUMB_LX_DOWN] = 0;
		}
		
		if (g_joykeyState.Gamepad.sThumbLY >= DOWN_DEADZONE && g_joykeyState.Gamepad.sThumbLY <= UP_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[JOYTHUMB_LY_UP] = 0;
			g_nCounterJoyThumbRepeat[JOYTHUMB_LY_DOWN] = 0;
		}
		
		if (g_joykeyState.Gamepad.sThumbRX >= DOWN_DEADZONE && g_joykeyState.Gamepad.sThumbRX <= UP_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[JOYTHUMB_RX_UP] = 0;
			g_nCounterJoyThumbRepeat[JOYTHUMB_RX_DOWN] = 0;
		}
		
		if (g_joykeyState.Gamepad.sThumbRY >= DOWN_DEADZONE && g_joykeyState.Gamepad.sThumbRY <= UP_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[JOYTHUMB_RY_UP] = 0;
			g_nCounterJoyThumbRepeat[JOYTHUMB_RY_DOWN] = 0;
		}

		if (g_nCounterVibretion > 0)
		{
			g_nCounterVibretion--;
			if (g_nCounterVibretion <= 0)
			{
				g_nCounterVibretion = 0;
				g_joyVibration.wLeftMotorSpeed = 0;
				g_joyVibration.wRightMotorSpeed = 0;

				XInputSetState(0, &g_joyVibration);
			}
		}
	}
}

//================================================================================================================
// ジョイパッドのプレス情報を取得
//================================================================================================================
bool GetJoypadPress(JOYKEY Key)
{
	if (Key <= JOYKEY_Y)
	{
		return (g_joykeyState.Gamepad.wButtons & (0x01 << Key)) ? true : false;
	}
	else
	{
		switch (Key)
		{
		case JOYKEY_LEFTTRIGGER:

			return (g_joykeyState.Gamepad.bLeftTrigger > 0.0f) ? true : false;

			break;

		case JOYKEY_RIGHTTRIGGER:

			return (g_joykeyState.Gamepad.bRightTrigger > 0.0f) ? true : false;

			break;

		default:

			return false;

			break;
		}
	}
}

//================================================================================================================
// ジョイパッドのトリガー情報を取得
//================================================================================================================
bool GetJoypadTrigger(JOYKEY Key)
{
	if (Key <= JOYKEY_Y)
	{
		return (g_joykeyStateTrigger.Gamepad.wButtons & (0x01 << Key)) ? true : false;
	}
	else
	{
		switch (Key)
		{
		case JOYKEY_LEFTTRIGGER:

			return (g_joykeyStateTrigger.Gamepad.bLeftTrigger > 0.0f) ? true : false;

			break;

		case JOYKEY_RIGHTTRIGGER:

			return (g_joykeyStateTrigger.Gamepad.bRightTrigger > 0.0f) ? true : false;

			break;

		default:

			return false;

			break;
		}
	}
}

//================================================================================================================
// ジョイパッドのリリース情報を取得
//================================================================================================================
bool GetJoypadRelease(JOYKEY Key)
{
	return (g_joykeyStateRelease.Gamepad.wButtons & (0x01 << Key)) ? true : false;
}

//================================================================================================================
// ジョイパッドのリピート情報を取得
//================================================================================================================
bool GetJoypadRepeat(JOYKEY Key, int nCounterRepeat)
{
	g_nCounterRepeatJoypad[Key]++;
	if (g_nCounterRepeatJoypad[Key] <= 30)
	{
		return (g_joykeyStateTrigger.Gamepad.wButtons & (0x01 << Key)) ? true : false;
	}
	else
	{
		if (g_nCounterRepeatJoypad[Key] % nCounterRepeat == 0)
		{
			return (g_joykeyState.Gamepad.wButtons & (0x01 << Key)) ? true : false;
		}
		else
		{
			return false;
		}
	}
}

//================================================================================================================
// ジョイパッドの押下情報を取得
//===============================================================================================================
bool GetJoypadAny(void)
{
	return g_bUseJoykeyAny;
}

//================================================================================================================
// ジョイパッドの基本動作情報(WASD)を取得
//================================================================================================================
bool GetJoypadWASD(void)
{
	if (g_joykeyState.Gamepad.wButtons & (0x01 << JOYKEY_UP))
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.wButtons & (0x01 << JOYKEY_DOWN))
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.wButtons & (0x01 << JOYKEY_LEFT))
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.wButtons & (0x01 << JOYKEY_RIGHT))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//================================================================================================================
// ジョイパッドのバイブレーションのポインタを取得
//================================================================================================================
XINPUT_VIBRATION *GetJoyVibration(void)
{
	return &g_joyVibration;
}

//================================================================================================================
// ジョイパッドのポインタを取得
//================================================================================================================
XINPUT_STATE *GetJoypadState(void)
{
	return &g_joykeyState;
}

//================================================================================================================
// ジョイパッドの左スティック[X方向]の判定
//================================================================================================================
bool GetJoyThumbLXState(void)
{
	return (g_joykeyState.Gamepad.sThumbLX != 0) ? true : false;
}

//================================================================================================================
// ジョイパッドの左スティック[Y方向]の判定
//================================================================================================================
bool GetJoyThumbLYState(void)
{
	return (g_joykeyState.Gamepad.sThumbLY != 0) ? true : false;
}

//================================================================================================================
// ジョイパッドの右スティック[X方向]の判定
//================================================================================================================
bool GetJoyThumbRXState(void)
{
	return (g_joykeyState.Gamepad.sThumbRX != 0) ? true : false;
}

//================================================================================================================
// ジョイパッドの右スティック[Y方向]の判定
//================================================================================================================
bool GetJoyThumbRYState(void)
{
	return (g_joykeyState.Gamepad.sThumbRY != 0) ? true : false;
}

//================================================================================================================
// ジョイパッドのスティックの値
//================================================================================================================
bool GetJoyThumbValue(int* pValueH, int* pValueV, int nThumb)
{
	if ((pValueH))
	{
		if (nThumb == JOYTHUMB_LEFT)
		{
			*pValueH = g_joykeyState.Gamepad.sThumbLX;
		}
		else if(nThumb == JOYTHUMB_RIGHT)
		{
			*pValueH = g_joykeyState.Gamepad.sThumbRX;
		}
	}

	if ((pValueV))
	{
		if (nThumb == JOYTHUMB_LEFT)
		{
			*pValueV = g_joykeyState.Gamepad.sThumbLY;
		}
		else if (nThumb == JOYTHUMB_RIGHT)
		{
			*pValueV = g_joykeyState.Gamepad.sThumbRY;
		}
	}

	if (nThumb == JOYTHUMB_LEFT)
	{
		if (g_joykeyState.Gamepad.sThumbLX <= -500 || g_joykeyState.Gamepad.sThumbLX >= 500
			|| g_joykeyState.Gamepad.sThumbLY <= -500 || g_joykeyState.Gamepad.sThumbLY >= 500)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (nThumb == JOYTHUMB_RIGHT)
	{
		if (g_joykeyState.Gamepad.sThumbRX <= -500 || g_joykeyState.Gamepad.sThumbRX >= 500
			|| g_joykeyState.Gamepad.sThumbRY <= -500 || g_joykeyState.Gamepad.sThumbRY >= 500)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

//================================================================================================================
// ジョイパッドのスティックの値
//================================================================================================================
float GetJoyThumbPow(int nThumb)
{
	switch (nThumb)
	{
	case (JOYTHUMB_LEFT + JOYTHUMB_X):
		
		return g_joykeyState.Gamepad.sThumbLX;

		break;

	case (JOYTHUMB_LEFT + JOYTHUMB_Y):
		
		return g_joykeyState.Gamepad.sThumbLY;

		break;

	case (JOYTHUMB_RIGHT + JOYTHUMB_X):
		
		return g_joykeyState.Gamepad.sThumbRX;

		break;

	case (JOYTHUMB_RIGHT + JOYTHUMB_Y):

		return g_joykeyState.Gamepad.sThumbRY;

		break;

	default:

		return 0.0f;

		break;
	}
}

//================================================================================================================
// ジョイパッドのスティックの入力遅延
//================================================================================================================
bool GetJoyThumbSlow(JOYTHUMB Thumb)
{
	g_nCounterJoyThumb[Thumb]++;

	switch (Thumb)
	{
	case JOYTHUMB_LX_UP:
		if (g_joykeyState.Gamepad.sThumbLX > 0
			&& (g_nCounterJoyThumb[Thumb] % THUMB_SLOW) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}

		break;

	case JOYTHUMB_LX_DOWN:
		if (g_joykeyState.Gamepad.sThumbLX < 0
			&& (g_nCounterJoyThumb[Thumb] % THUMB_SLOW) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}

		break;

	case JOYTHUMB_LY_UP:
		if (g_joykeyState.Gamepad.sThumbLY > 0
			&& (g_nCounterJoyThumb[Thumb] % THUMB_SLOW) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}

		break;

	case JOYTHUMB_LY_DOWN:
		if (g_joykeyState.Gamepad.sThumbLY < 0
			&& (g_nCounterJoyThumb[Thumb] % THUMB_SLOW) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}

		break;

	case JOYTHUMB_RX_UP:
		if (g_joykeyState.Gamepad.sThumbRX > 0
			&& (g_nCounterJoyThumb[Thumb] % THUMB_SLOW) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}

		break;

	case JOYTHUMB_RX_DOWN:
		if (g_joykeyState.Gamepad.sThumbRX < 0
			&& (g_nCounterJoyThumb[Thumb] % THUMB_SLOW) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}

		break;

	case JOYTHUMB_RY_UP:
		if (g_joykeyState.Gamepad.sThumbRY > 0
			&& (g_nCounterJoyThumb[Thumb] % THUMB_SLOW) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}

		break;

	case JOYTHUMB_RY_DOWN:
		if (g_joykeyState.Gamepad.sThumbRY < 0
			&& (g_nCounterJoyThumb[Thumb] % THUMB_SLOW) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}

		break;

	default:

		return false;

		break;
	}
}

//================================================================================================================
// ジョイパッドのスティックのリピート処理
//================================================================================================================
bool GetJoyThumbRepeat(JOYTHUMB Thumb, int nCounterRepeat)
{
	switch (Thumb)
	{
	case JOYTHUMB_LX_UP:
		if (g_joykeyState.Gamepad.sThumbLX > UP_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[Thumb]++;
			if (g_nCounterJoyThumbRepeat[Thumb] == 1)
			{
				return true;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > 1 && g_nCounterJoyThumbRepeat[Thumb] <= REPEAT_TIME)
			{
				return false;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > REPEAT_TIME)
			{
				if (g_nCounterJoyThumbRepeat[Thumb] % nCounterRepeat == 0)
				{
					return true;
				}
			}
		}

		break;

	case JOYTHUMB_LX_DOWN:
		if (g_joykeyState.Gamepad.sThumbLX < DOWN_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[Thumb]++;
			if (g_nCounterJoyThumbRepeat[Thumb] == 1)
			{
				return true;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > 1 && g_nCounterJoyThumbRepeat[Thumb] <= REPEAT_TIME)
			{
				return false;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > REPEAT_TIME)
			{
				if (g_nCounterJoyThumbRepeat[Thumb] % nCounterRepeat == 0)
				{
					return true;
				}
			}
		}

		break;

	case JOYTHUMB_LY_UP:
		if (g_joykeyState.Gamepad.sThumbLY > UP_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[Thumb]++;
			if (g_nCounterJoyThumbRepeat[Thumb] == 1)
			{
				return true;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > 1 && g_nCounterJoyThumbRepeat[Thumb] <= REPEAT_TIME)
			{
				return false;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > REPEAT_TIME)
			{
				if (g_nCounterJoyThumbRepeat[Thumb] % nCounterRepeat == 0)
				{
					return true;
				}
			}
		}

		break;

	case JOYTHUMB_LY_DOWN:
		if (g_joykeyState.Gamepad.sThumbLY < DOWN_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[Thumb]++;
			if (g_nCounterJoyThumbRepeat[Thumb] == 1)
			{
				return true;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > 1 && g_nCounterJoyThumbRepeat[Thumb] <= REPEAT_TIME)
			{
				return false;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > REPEAT_TIME)
			{				
				if (g_nCounterJoyThumbRepeat[Thumb] % nCounterRepeat == 0)
				{
					return true;
				}
			}
		}

		break;

	case JOYTHUMB_RX_UP:
		if (g_joykeyState.Gamepad.sThumbRX > UP_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[Thumb]++;
			if (g_nCounterJoyThumbRepeat[Thumb] == 1)
			{
				return true;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > 1 && g_nCounterJoyThumbRepeat[Thumb] <= REPEAT_TIME)
			{
				return false;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > REPEAT_TIME)
			{
				if (g_nCounterJoyThumbRepeat[Thumb] % nCounterRepeat == 0)
				{
					return true;
				}
			}
		}

		break;

	case JOYTHUMB_RX_DOWN:
		if (g_joykeyState.Gamepad.sThumbRX < DOWN_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[Thumb]++;
			if (g_nCounterJoyThumbRepeat[Thumb] == 1)
			{
				return true;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > 1 && g_nCounterJoyThumbRepeat[Thumb] <= REPEAT_TIME)
			{
				return false;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > REPEAT_TIME)
			{
				if (g_nCounterJoyThumbRepeat[Thumb] % nCounterRepeat == 0)
				{
					return true;
				}
			}
		}

		break;

	case JOYTHUMB_RY_UP:
		if (g_joykeyState.Gamepad.sThumbRY > UP_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[Thumb]++;
			if (g_nCounterJoyThumbRepeat[Thumb] == 1)
			{
				return true;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > 1 && g_nCounterJoyThumbRepeat[Thumb] <= REPEAT_TIME)
			{
				return false;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > REPEAT_TIME)
			{
				if (g_nCounterJoyThumbRepeat[Thumb] % nCounterRepeat == 0)
				{
					return true;
				}
			}
		}

		break;

	case JOYTHUMB_RY_DOWN:
		if (g_joykeyState.Gamepad.sThumbRY < DOWN_DEADZONE)
		{
			g_nCounterJoyThumbRepeat[Thumb]++;
			if (g_nCounterJoyThumbRepeat[Thumb] == 1)
			{
				return true;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > 1 && g_nCounterJoyThumbRepeat[Thumb] <= REPEAT_TIME)
			{
				return false;
			}
			else if (g_nCounterJoyThumbRepeat[Thumb] > REPEAT_TIME)
			{
				if (g_nCounterJoyThumbRepeat[Thumb] % nCounterRepeat == 0)
				{
					return true;
				}
			}
		}

		break;

	default:

		return false;

		break;
	}

	return false;
}

//================================================================================================================
// ジョイパッドのスティック基本動作情報(WASD)を取得
//================================================================================================================
bool GetJoyThumbWASD(void)
{
	if (g_joykeyState.Gamepad.sThumbLX > UP_DEADZONE)
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.sThumbLX < DOWN_DEADZONE)
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.sThumbLY > UP_DEADZONE)
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.sThumbLY < DOWN_DEADZONE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//================================================================================================================
// ジョイパッドのスティック基本動作情報(視点移動)を取得
//================================================================================================================
bool GetJoyThumbWatch(void)
{
	if (g_joykeyState.Gamepad.sThumbRX > UP_DEADZONE)
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.sThumbRX < DOWN_DEADZONE)
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.sThumbRY > UP_DEADZONE)
	{
		return true;
	}
	else if (g_joykeyState.Gamepad.sThumbRY < DOWN_DEADZONE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//================================================================================================================
// ジョイパッドの角度取得
//================================================================================================================
float GetJoyThumbAngle(int nThumb)
{
	float fAngle = 0.0f;

	if (nThumb == JOYTHUMB_LEFT)
	{
		fAngle = atan2f((float)g_joykeyState.Gamepad.sThumbLX, (float)g_joykeyState.Gamepad.sThumbLY);
	}
	else if(nThumb == JOYTHUMB_RIGHT)
	{
		fAngle = atan2f((float)g_joykeyState.Gamepad.sThumbRX, (float)g_joykeyState.Gamepad.sThumbRY);
	}
	else
	{
		fAngle = 0.0f;
	}

	return fAngle;
}

//================================================================================================================
// ジョイパッドのトリガー押し込み処理
//================================================================================================================
bool GetJoyTrigger(int nTrigger)
{
	switch (nTrigger)
	{
	case JOYTRIGGER_LEFT:

		return (g_joykeyState.Gamepad.bLeftTrigger > 0.0f) ? true : false;

		break;

	case JOYTRIGGER_RIGHT:

		return (g_joykeyState.Gamepad.bRightTrigger > 0.0f) ? true : false;

		break;

	default:

		return false;

		break;
	}
}

//================================================================================================================
// バイブレーションの設定
//================================================================================================================
int SetVibration(int nLPower, int nRPower, int nTime)
{
	g_nCounterVibretion = nTime;
	g_joyVibration.wLeftMotorSpeed = nLPower;
	g_joyVibration.wRightMotorSpeed = nRPower;

	XInputSetState(0, &g_joyVibration);

	return nTime;
}

//================================================================================================================
// バイブレーションの取得
//================================================================================================================
void GetVibration(int *nLPower, int *nRPower)
{
	if (nLPower)
	{
		*nLPower = g_joyVibration.wLeftMotorSpeed;
	}

	if (nRPower)
	{
		*nRPower = g_joyVibration.wRightMotorSpeed;
	}
}

//================================================================================================================
// マウスの初期化処理
//================================================================================================================
HRESULT InitMouse(HWND hWnd)
{
	/*** 変数の初期化 ***/
	ZeroMemory(&g_alMouseMoveCurrent, sizeof(LONG) * MOUSESLOPE_MAX);
	ZeroMemory(&g_alMouseMoveExacLast, sizeof(LONG) * MOUSESLOPE_MAX);

	// 入力デバイス(マウス)の生成
	if (FAILED(g_pInput->CreateDevice(GUID_SysMouse,
		&g_pDevMouse,
		NULL)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "入力デバイス(マウス)の生成に失敗しました。");
		return E_FAIL;
	}

	// データフォーマットを設定
	if (FAILED(g_pDevMouse->SetDataFormat(&c_dfDIMouse)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "データフォーマットの設定に失敗しました。");
		return E_FAIL;
	}

	// 協調モードを設定
	if (FAILED(g_pDevMouse->SetCooperativeLevel(hWnd,
		(DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "協調モードの設定に失敗しました。");
		return E_FAIL;
	}

	// マウスへのアクセス権を獲得
	g_pDevMouse->Acquire();

	return S_OK;
}

//================================================================================================================
// マウスの終了処理
//================================================================================================================
void UninitMouse(void)
{
	// 入力デバイス(マウス)の破棄
	if (g_pDevMouse != NULL)
	{
		g_pDevMouse->Unacquire();
		g_pDevMouse->Release();
		g_pDevMouse = NULL;
	}
}

//================================================================================================================
// マウスの更新処理
//================================================================================================================
void UpdateMouse(void)
{
	BYTE aMouseState[NUM_MOUSE_MAX];			// マウスの入力情報

	// 更新前に過去の入力情報(マウス)を保存
	g_PrevMouseState = g_CurrentMouseState;
	g_alMouseMoveExacLast[0] = g_CurrentMouseState.lX;
	g_alMouseMoveExacLast[1] = g_CurrentMouseState.lY;
	g_alMouseMoveExacLast[2] = g_CurrentMouseState.lZ;

	// 現在のマウスの入力情報を取得
	if (SUCCEEDED(g_pDevMouse->GetDeviceState(sizeof(DIMOUSESTATE), &g_CurrentMouseState)))
	{
		g_alMouseMoveCurrent[0] = g_CurrentMouseState.lX;
		g_alMouseMoveCurrent[1] = g_CurrentMouseState.lY;
		g_alMouseMoveCurrent[2] = g_CurrentMouseState.lZ;

		for (int nCntMouse = 0; nCntMouse < NUM_MOUSE_MAX; nCntMouse++)
		{
			aMouseState[nCntMouse] = g_CurrentMouseState.rgbButtons[nCntMouse];
			g_aMouseStateTrigger[nCntMouse] = ((aMouseState[nCntMouse] ^ g_aMouseState[nCntMouse]) & aMouseState[nCntMouse]);
			g_aMouseStateRelease[nCntMouse] = (g_aMouseState[nCntMouse] & (g_aMouseState[nCntMouse] ^ aMouseState[nCntMouse]));
			g_aMouseState[nCntMouse] = aMouseState[nCntMouse];
		}
	}
	else
	{
		g_pDevMouse->Acquire();			// マウスへのアクセス権を取得
	}
}

//================================================================================================================
// マウスのプレス情報を取得
//================================================================================================================
bool GetMousePress(int nButton)
{
	return (g_aMouseState[nButton] & 0x80) ? true : false;
}

//================================================================================================================
// マウスのトリガー情報を取得
//================================================================================================================
bool GetMouseTrigger(int nButton)
{
	return (g_aMouseStateTrigger[nButton] & 0x80) ? true : false;
}

//================================================================================================================
// マウスのリリース情報を取得
//================================================================================================================
bool GetMouseRelease(int nButton)
{
	return (g_aMouseStateRelease[nButton] & 0x80) ? true : false;
}

//================================================================================================================
// マウスの位置情報を取得
//================================================================================================================
POINT GetMousePos(void)
{
	POINT mousePos;

	// マウスの位置を取得
	if (GetCursorPos(&mousePos) != TRUE)
	{// 取得失敗
		g_pDevMouse->Acquire();			// マウスへのアクセス権を取得
	}
	else
	{// 取得成功
		// 取得した座標をウィンドウ内の座標に変換
		ScreenToClient(g_InputhWnd, &mousePos);
		g_mousePos = mousePos;			// 現在の位置を保存
	}

	return g_mousePos;
}

//================================================================================================================
// マウスの過去からの回転情報を取得
//================================================================================================================
LONG GetMouseMove(MOUSESLOPE slope) 
{
	return (float)g_alMouseMoveCurrent[slope];
}