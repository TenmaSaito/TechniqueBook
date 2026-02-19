//================================================================================================================
//
// DirectXのXファイル読み込み用ヘッダファイル [modeldata.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _MODELDATA_H_
#define _MODELDATA_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define MAX_MODELDATA		(256)			// 読み込めるモデル数
#define MAX_MODELTEXTURE	(14)			// 読み込むテクスチャの最大数
#define ERROR_XFILE			(-1)			// Xファイル読み込み失敗時の値

//*************************************************************************************************
//*** モデル情報構造体の定義 ***
//*************************************************************************************************
typedef struct MODELDATA
{
	LPD3DXMESH pMesh;								// メッシュ(頂点情報)へのポインタ
	LPD3DXBUFFER pBuffMat;							// マテリアルへのポインタ
	LPDIRECT3DTEXTURE9 apTexture[MAX_MODELTEXTURE];	// テクスチャへのポインタ
	DWORD dwNumMat;									// マテリアルの数
	D3DXVECTOR3 mtxMin;								// オブジェクトの最小値
	D3DXVECTOR3 mtxMax;								// オブジェクトの最大値
	D3DXMATRIX mtxWorld;							// ワールドマトリックス
	char aXFileName[MAX_PATH];						// Xファイル名
	bool bUse;										// 一度格納しようとしたか
	bool bSafe;										// 情報が格納されているか(参照可能か)
} MODELDATA;

typedef struct MODELDATA* PMODELDATA, * LPMODELDATA;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitModelData(void);
void UninitModelData(void);
void ResetModelData(void);

HRESULT LoadModelData(const char* pXFileName, int *pOutnIdx);
LPMODELDATA GetModelData(int nType);
void InstancingModelData(int nIdx);
void DrawModelDirectByInstancing(void);
#endif