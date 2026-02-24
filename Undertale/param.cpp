//=============================================================================
//
// DirectXの共通値のヘッダーファイル [param.h]
// Author : TENMA SAITO
//
//=============================================================================
//*****************************************************************************
//*** インクルードファイル ***
//*****************************************************************************
#include "param.h"

using namespace Constants;

//*****************************************************************************
//*** 共通変数の設定 ***
//*****************************************************************************

//=============================================================================
// --- CParamInt ---
//=============================================================================

// デフォルトのFPS
const int CParamInt::FPS = 60;

// signed intの最大値
const int CParamInt::SIGNED_INFINITY = 0x7FFFFFFF;

// スクリーンの幅
const int CParamInt::SCWIDTH = 1280;

// スクリーンの高さ
const int CParamInt::SCHEIGHT = 720;

//=============================================================================
// --- CParamFloat ---
//=============================================================================

// 1/4円
const float CParamFloat::HALFPI = D3DX_PI * 0.5f;

// 円
const float CParamFloat::DOUBLEPI = D3DX_PI * 2.0f;

//=============================================================================
// --- CParamVector ---
//=============================================================================

// VECTOR2NULL
const D3DXVECTOR2 CParamVector::V2NULL = D3DXVECTOR2(0, 0);

// VECTOR3NULL
const D3DXVECTOR3 CParamVector::V3NULL = D3DXVECTOR3(0, 0, 0);

// VECTOR4NULL
const D3DXVECTOR4 CParamVector::V4NULL = D3DXVECTOR4(0, 0, 0, 0);

// ウィンドウの中心
const D3DXVECTOR3 CParamVector::WINMID = D3DXVECTOR3(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0);

//=============================================================================
// --- CParamColor ---
//=============================================================================

// 白
const D3DXCOLOR CParamColor::WHITE = D3DXCOLOR(1, 1, 1, 1);

// 黒
const D3DXCOLOR CParamColor::BLACK = D3DXCOLOR(0, 0, 0, 1);

// 赤
const D3DXCOLOR CParamColor::RED = D3DXCOLOR(1, 0, 0, 1);

// 青
const D3DXCOLOR CParamColor::BLUE = D3DXCOLOR(0, 0, 1, 1);

// 緑
const D3DXCOLOR CParamColor::GREEN = D3DXCOLOR(0, 1, 0, 1);

// 空
const D3DXCOLOR CParamColor::CYAN = D3DXCOLOR(0, 1, 1, 1);

// マゼンタ
const D3DXCOLOR CParamColor::MAGENTA = D3DXCOLOR(1, 1, 0, 1);

// 黄色
const D3DXCOLOR CParamColor::YELLOW = D3DXCOLOR(1, 0, 1, 1);

// 透明白
const D3DXCOLOR CParamColor::INV_WHITE = D3DXCOLOR(1, 1, 1, 0);

// 無色
const D3DXCOLOR CParamColor::INV_NONE = D3DXCOLOR(0, 0, 0, 0);

//=============================================================================
// --- CParamString ---
//=============================================================================

// デフォルトのエラー文
const char *CParamString::DEFAULT_ERROR = "エラーが発生しました。";

//=============================================================================
// --- CParamEx ---
//=============================================================================

// デフォルトのビューポート設定
const D3DVIEWPORT9 CParamEx::DEF_VP = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1 };

// Matrix初期値
const D3DXMATRIX CParamEx::MTX_IDENTITY = D3DXMATRIX
(
	1, 0, 0, 0,
	0, 1, 0, 0, 
	0, 0, 1, 0,
	0, 0, 0, 1
);

// 1ポリゴンのサイズ(VERTEX_2D)
const size_t CParamEx::VTX2DSIZE = sizeof(VERTEX_2D) * 4;

// 1ポリゴンのサイズ(VERTEX_3D)
const size_t CParamEx::VTX3DSIZE = sizeof(VERTEX_3D) * 4;