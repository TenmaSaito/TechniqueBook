//==================================================================================
//
// DirectXの共通値のヘッダーファイル [param.h]
// Author : TENMA
//
//==================================================================================
#ifndef _PARAM_H_
#define _PARAM_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"

//**********************************************************************************
//*** データ格納構造体群 ***
//**********************************************************************************
namespace Constants 
{
	// int型
	STRUCT(CParamInt)
	{
		static const int FPS;				// デフォルトFPS
		static const int SIGNED_INFINITY;	// signed intの最大値
		static const int SCWIDTH;			// スクリーンの幅
		static const int SCHEIGHT;			// スクリーンの高さ
	} CParamInt;

	// float型
	STRUCT(CParamFloat)
	{
		static const float HALFPI;				// 1/4円
		static const float DOUBLEPI;			// 円
	} CParamFloat;

	// char型
	STRUCT(CParamString)
	{
		static const char *DEFAULT_ERROR;		// 通常エラー文
	} CParamString;

	// vector型
	STRUCT(CParamVector)
	{
		static const D3DXVECTOR4 V4NULL;		// ベクター4null
		static const D3DXVECTOR3 V3NULL;		// ベクター3null
		static const D3DXVECTOR3 WINMID;		// ウィンドウの中心
		static const D3DXVECTOR2 V2NULL;		// ベクター2null
	} CParamVector;

	// color型
	STRUCT(CParamColor)
	{
		static const D3DXCOLOR WHITE;			// 白
		static const D3DXCOLOR BLACK;			// 黒
		static const D3DXCOLOR RED;				// 赤
		static const D3DXCOLOR BLUE;			// 青
		static const D3DXCOLOR GREEN;			// 緑
		static const D3DXCOLOR CYAN;			// 空
		static const D3DXCOLOR MAGENTA;			// マゼンタ
		static const D3DXCOLOR YELLOW;			// 黄色
		static const D3DXCOLOR INV_WHITE;		// 透明白
		static const D3DXCOLOR INV_NONE;		// 無色
	} CParamColor;

	// 特殊型
	STRUCT(CParamEx)
	{
		static const D3DXMATRIX MTX_IDENTITY;	// マトリックス初期値
		static const D3DVIEWPORT9 DEF_VP;		// デフォルトビューポート
		static const size_t VTX2DSIZE;			// 1ポリゴンのサイズ(VERTEX_2D)
		static const size_t VTX3DSIZE;			// 1ポリゴンのサイズ(VERTEX_3D)
	} CParamEx;
}
#endif
