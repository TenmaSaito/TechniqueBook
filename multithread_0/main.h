//================================================================================================================
//
// DirectXのメインヘッダファイル [main.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _MAIN_H_				// このマクロ定義がされていなければ
#define _MAIN_H_				// 2重インクルード防止のマクロを定義

//#define _CRTDBG_MAP_ALLOC		// メモリリーク検知範囲拡大マクロ

//**********************************************************************************
//*** インクルードファイル(DirectX関係) ***
//**********************************************************************************
#include <windows.h>						// windowsアプリケーションを使う上で必須
#include "d3dx9.h"							// 描画処理に必要
#define DIRECTINPUT_VERSION		(0x0800)	// ビルド時の警告対処用マクロ
#include "dinput.h"							// 入力処理に必要(先に上のマクロを定義する)
#include "xaudio2.h"						// サウンド処理に必要
#include "xinput.h"							// ジョイパッド処理に必要
#include "X3DAudio.h"

//**********************************************************************************
//*** ライブラリのリンク ***
//**********************************************************************************
#pragma comment(lib,"d3d9.lib")				// 描画処理に必要
#pragma comment(lib,"d3dx9.lib")			// d3d9.libの拡張ライブラリ
#pragma comment(lib,"dxguid.lib")			// DirectXコンポーネント(部品)使用に必要
#pragma comment(lib,"winmm.lib")			// システムの時刻取得に必要
#pragma comment(lib,"dinput8.lib")			// 入力処理に必要
#pragma comment(lib,"xinput.lib")			// ジョイパッド処理に必要
#pragma comment(lib,"xaudio2.lib")			// 3Dオーディオに必要

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include <stdio.h>							// 標準入出力に必要
#include <stdlib.h>							// 疑似乱数に必要
#include <string.h>							// 文字列関連の処理に必要
#include <crtdbg.h>							// メモリリーク感知
#include <process.h>						// マルチスレッド処理

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define SCREEN_WIDTH		(1280)												// ウィンドウの幅
#define SCREEN_HEIGHT		(720)												// ウィンドウの高さ
#define FVF_VERTEX_2D		(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)		// 頂点フォーマット (2D)
#define FVF_VERTEX_3D		(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)		// 頂点フォーマット (3D)
#define D3DXVECTOR2_NULL	D3DXVECTOR2(0.0f, 0.0f)								// D3DXVECTOR2のNULL
#define D3DXVECTOR3_NULL	D3DXVECTOR3(0.0f,0.0f,0.0f)							// D3DXVECTOR3のNULL
#define D3DXVECTOR4_NULL	D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f)					// D3DXVECTOR4のNULL
#define D3DXCOLOR_NULL		D3DXCOLOR(1.0f,1.0f,1.0f,1.0f)						// COLORのNULL
#define D3DXCOLOR_INV		D3DXCOLOR(1.0f,1.0f,1.0f,0.0f)						// COLORの透明版
#define STRING_MAX			(1024)												// 文字列の最大文字数
#define WINDOW_MID			D3DXVECTOR3(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f)	// ウィンドウの中心
#define ENABLE_LOOP			false												// デバッグ用ENTERループをオンにするか
#define ENABLE_CLICKUP		false												// Uキーを押している間のみ更新するか
#define RELEASE(p)		{ if ((p) != NULL) {(p)->Release(); (p) = nullptr; } }	// メモリ確保ポインタのRelease用マクロ
#define LIGHTING(p, Enable)	{ (p)->SetRenderState(D3DRS_LIGHTING, Enable); }	// ライティングの有無効
#define D3DX_2PI			(D3DX_PI * 2.0f)									// 角度の一周分
#define D3DX_HALFPI			(D3DX_PI * 0.5f)									// 角度の1/4周分
#define _PATHCHECK_NO_WARNINGS 

#ifdef NDEBUG

#undef _PATHCHECK_NO_WARNINGS

#endif

//**********************************************************************************
//*** 画面モードの種類 ***
//**********************************************************************************
typedef enum
{
	MODE_TITLE = 0,			// タイトル画面
	MODE_TUTORIAL,			// チュートリアル付きゲーム画面
	MODE_GAME,				// ゲーム画面
	MODE_GOODEND,			// グッドエンド
	MODE_BADEND,			// バッドエンド
	MODE_RANKING,			// ランキング画面
	MODE_MAX
}MODE;

//**********************************************************************************
//*** ゲームの難易度 ***
//**********************************************************************************
typedef enum
{
	GAMEDIFFICULTY_EASY = 0,		// EASY
	GAMEDIFFICULTY_NORMAL,			// NORMAL
	GAMEDIFFICULTY_HARD,			// HARD
	GAMEDIFFICULTY_MAX
}GAMEDIFFICULTY;

//**********************************************************************************
//*** 頂点情報(2D)の構造体を定義 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 頂点情報(x,y,z)
	float rhw;				// 座標変換用係数(1.0fで固定)
	D3DCOLOR col;			// 頂点カラー(R,G,B,a)
	D3DXVECTOR2 tex;		// テクスチャ座標(x,y)
} VERTEX_2D;

//**********************************************************************************
//*** 頂点情報(3D)の構造体を定義 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 頂点情報(x,y,z)
	D3DXVECTOR3 nor;		// 法線ベクトル
	D3DCOLOR col;			// 頂点カラー(R,G,B,a)
	D3DXVECTOR2 tex;		// テクスチャ座標(x,y)
} VERTEX_3D;

//**********************************************************************************
//*** 四点範囲指定構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 point1;		// 左上
	D3DXVECTOR3 point2;		// 右上
	D3DXVECTOR3 point3;		// 左下
	D3DXVECTOR3 point4;		// 右下
}POINT_RECT;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
LPDIRECT3DDEVICE9 GetDevice(void);
void EndDevice(void);
void SetMode(MODE mode);
MODE GetMode(void);
MODE GetModeExac(void);
HWND GetHandleWindow(void);
bool GetIsMainThread(void);
void SetGameDifficulty(GAMEDIFFICULTY difficulty);
GAMEDIFFICULTY GetGameDifficulty(void);
void SetStageExac(int nStage);
int GetStageExac(void);

#endif

