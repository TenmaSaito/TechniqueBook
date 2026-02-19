//================================================================================================================
//
// DirectXのインスタンシングモデル表示用ヘッダファイル [InstancingModel.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _IC_MODEL_H_
#define _IC_MODEL_H_

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
	D3DXCOLOR col;				// 頂点カラー
} VTX_GEOMETRY_IC;

#define FVF_GEOMETRY		(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)

typedef struct
{
	D3DXMATRIX mtxWorld;		// ワールドマトリックス
} VTX_INSTANCE_IC;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitInstancingModel(void);
void UninitInstancingModel(void);
void UpdateInstancingModel(void);
void DrawInstancingModelInstance(void);

void SetInstancingModel(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int);
#endif
