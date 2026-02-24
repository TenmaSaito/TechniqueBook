//================================================================================================================
//
// DirectXの3Dモデル配置のcppファイル [3DModel.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "3DModel.h"
#include "modeldata.h"
#include "mathUtil.h"

using namespace MyMathUtil;

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_3DMODEL		(256)			// 設置できるモデルの最大数

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
_3DMODEL g_aModel[MAX_3DMODEL];		// 3Dモデル情報
int g_nNumModel;					// 設置した3Dモデルの数

//=================================================================================================
// --- モデル初期化 ---
//=================================================================================================
void Init3DModel(void)
{
	// モデルを初期化
	ZeroMemory(&g_aModel[0], sizeof(_3DMODEL) * (MAX_3DMODEL));

	// 設置したモデルの数を初期化
	g_nNumModel = 0;
}

//=================================================================================================
// --- モデル終了 ---
//=================================================================================================
void Uninit3DModel(void)
{
	// 3Dmodel.cppにて動的にメモリを確保した場合(テクスチャ読み込み等)、ここで解放
}

//=================================================================================================
// --- モデル更新 ---
//=================================================================================================
void Update3DModel(void)
{
	
}

//=================================================================================================
// --- モデル描画 ---
//=================================================================================================
void Draw3DModel(void)
{
	// デバイスの取得開始
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LP3DMODEL p3DModel = &g_aModel[0];	// 3Dモデルへのポインタ
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	D3DMATERIAL9 matDef;				// 現在のマテリアル保存用
	D3DXMATERIAL* pMat;					// マテリアルデータへのポインタ

	// 3Dモデルの描画
	for (int nCnt3DModel = 0; nCnt3DModel < MAX_3DMODEL; nCnt3DModel++, p3DModel++)
	{
		if (p3DModel->bUse != false)
		{ // もし使われていれば
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&p3DModel->mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				p3DModel->rot.y,			// Y軸回転
				p3DModel->rot.x,			// X軸回転
				p3DModel->rot.z);			// Z軸回転

			D3DXMatrixMultiply(&p3DModel->mtxWorld, &p3DModel->mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				p3DModel->pos.x,
				p3DModel->pos.y,
				p3DModel->pos.z);

			D3DXMatrixMultiply(&p3DModel->mtxWorld, &p3DModel->mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &p3DModel->mtxWorld);

			// モデルデータを取得
			LPMODELDATA pModelData = GetModelData(p3DModel->nIdx3Dmodel);
			if (pModelData)
			{
				/*** 現在のマテリアルを保存 ***/
				pDevice->GetMaterial(&matDef);

				/*** マテリアルデータへのポインタを取得 ***/
				pMat = (D3DXMATERIAL*)pModelData->pBuffMat->GetBufferPointer();

				for (int nCntMat = 0; nCntMat < (int)pModelData->dwNumMat; nCntMat++)
				{
					/*** マテリアルの設定 ***/
					pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

					/*** テクスチャの設定 ***/
					pDevice->SetTexture(0, pModelData->apTexture[nCntMat]);

					/*** モデル(パーツ)の描画 ***/
					pModelData->pMesh->DrawSubset(nCntMat);
				}

				/*** 保存していたマテリアルを戻す！ ***/
				pDevice->SetMaterial(&matDef);
			}
		}
	}

	// デバイスの取得終了
	EndDevice();
}

//=================================================================================================
// --- モデル設置 ---
//=================================================================================================
int Set3DModel(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nIdxModelData)
{
	LP3DMODEL p3DModel = &g_aModel[0];
	int nCnt3DModel;

	// 3Dモデルの設置
	for (nCnt3DModel = 0; nCnt3DModel < MAX_3DMODEL; nCnt3DModel++, p3DModel++)
	{
		if (p3DModel->bUse == false)
		{ // もし使われていなければ
			p3DModel->pos = pos;
			p3DModel->rot = rot;
			p3DModel->bUse = true;
			g_nNumModel++;

			break;
		}
	}

	// 最大数を超えていれば-1に変更
	if (nCnt3DModel >= MAX_3DMODEL) nCnt3DModel = -1;

	return nCnt3DModel;
}

//=================================================================================================
// --- ポインタ取得 ---
//=================================================================================================
LP3DMODEL Get3DModel(int nIdxModel)
{
	// もしインデックス外なら
	if (nIdxModel < 0 || nIdxModel >= MAX_3DMODEL) return NULL;
	return &g_aModel[nIdxModel];
}