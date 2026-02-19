//================================================================================================================
//
// DirectXのビルボードバレット表示用ヘッダファイル [bullet.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _EFFECT_H_
#define _EFFECT_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************

typedef struct
{
	D3DXVECTOR3 pos;			// 頂点座標
	D3DXVECTOR3 nor;			// 法線
	D3DXVECTOR2 tex;			// テクスチャ座標
} VTX_GEOMETRY;

#define FVF_GEOMETRY		(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

typedef struct
{
	D3DXMATRIX mtxWorld;		// ワールドマトリックス
	D3DXCOLOR col;				// 頂点カラー
} VTX_INSTANCE;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitEffect(void);
void UninitEffect(void);
void UpdateEffect(void);
void DrawEffect(void);
void DrawEffectInstance(void);

void SetEffect(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fSpd, float fWidth, float fHeight, int nLife = -1);
void SetIndexTextureEffect(int nIndexTexture);
LPD3DXEFFECT GetShader(void);
#endif