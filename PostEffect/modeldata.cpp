//================================================================================================================
//
// DirectXのXファイル読み込み表示処理 [modeldata.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "modeldata.h"
#include "mathUtil.h"

using namespace MyMathUtil;

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
MODELDATA g_aModelData[MAX_MODELDATA];		// オブジェクト情報
char g_aResetFileNameXmodel[MAX_MODELDATA][MAX_PATH];
int g_nNumLoadModelData;

//================================================================================================================
// --- 初期化 ---
//================================================================================================================
void InitModelData(void)
{
	// モデルデータを初期化
	ZeroMemory(&g_aModelData[0], sizeof(MODELDATA) * (MAX_MODELDATA));

	// リセット用データを初期化
	ZeroMemory(&g_aResetFileNameXmodel[0], sizeof(g_aResetFileNameXmodel));

	// 読み込んだモデルデータの数を初期化
	g_nNumLoadModelData = 0;
}

//================================================================================================================
// --- モデルの読み込み ---
//================================================================================================================
HRESULT LoadModelData(_In_ const char* pXFileName, int *pOutnIdx)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPMODELDATA pModelData = &g_aModelData[0];
	D3DXMATERIAL* pMat = NULL;		// マテリアルへのポインタ
	HRESULT hr = E_FAIL;			// 読み込み成功判定
	int nNumVtx = 0;				// 頂点数
	DWORD dwSizeFVF = 0;			// 頂点フォーマットのサイズ
	BYTE* pVtxBuff = NULL;			// 頂点バッファへのポインタ

	if (pOutnIdx)
	{ // 読み込み失敗時の値を格納
		*pOutnIdx = ERROR_XFILE;
	}

	/*** 過去に読み込んでいないか確認 ***/
	for (int nCntXmodel = 0; nCntXmodel < MAX_MODELDATA; nCntXmodel++, pModelData++)
	{
		if (strcmp(&pModelData->aXFileName[0], pXFileName) == NULL)
		{ // 読み込み済みの場合
			if (pOutnIdx)
			{ // 読みこみモデルの値を格納
				*pOutnIdx = nCntXmodel;
			}

			EndDevice();

			return S_OK;
		}
	}

	pModelData = &g_aModelData[0];

	/*** Xファイルの読み込み ***/
	for (int nCntXmodel = 0; nCntXmodel < MAX_MODELDATA; nCntXmodel++, pModelData++)
	{
		if (pModelData->bUse != true)
		{
			// 使用済みに
			pModelData->bUse = true;
			g_nNumLoadModelData++;

			pMat = NULL;

			// Xファイルの読み込み
			hr = D3DXLoadMeshFromX(pXFileName,			// 読み込むXファイル名
				D3DXMESH_SYSTEMMEM,
				pDevice,								// デバイスポインタ
				NULL,
				&pModelData->pBuffMat,	// マテリアルへのポインタ
				NULL,
				&pModelData->dwNumMat,	// マテリアルの数
				&pModelData->pMesh);		// メッシュへのポインタ

			strcpy(&g_aResetFileNameXmodel[nCntXmodel][0], pXFileName);

			if (FAILED(hr))
			{ // 読み込み失敗時
				GenerateMessageBox(MB_ICONERROR,
					"ERROR!",
					"Xファイルの読み込みに失敗しました!\n対象パス : %s",
					pXFileName);

				EndDevice();

				return E_FAIL;
			}

			if (pOutnIdx)
			{ // 読みこんだモデルの値を格納
				*pOutnIdx = nCntXmodel;
			}

			/*** マテリアルデータへのポインタを取得 ***/
			pMat = (D3DXMATERIAL*)pModelData->pBuffMat->GetBufferPointer();

			for (int nCntMat = 0; nCntMat < (int)pModelData->dwNumMat; nCntMat++)
			{
				if (pMat[nCntMat].pTextureFilename != NULL)
				{
					// 相対パスになっているか確認
					CheckPath(pMat[nCntMat].pTextureFilename);

					/*** テクスチャの読み込み ***/
					hr = D3DXCreateTextureFromFile(pDevice,
						pMat[nCntMat].pTextureFilename,
						&pModelData->apTexture[nCntMat]);

					if (FAILED(hr))
					{ // テクスチャ読み込み失敗時
						GenerateMessageBox(MB_ICONERROR,
							"Error (4)",
							"Xファイルのテクスチャ読み込みに失敗しました。\n対象パス : %s",
							&pMat[nCntMat].pTextureFilename[0]);
					}
				}
			}

			/*** 頂点数を取得 ***/
			nNumVtx = pModelData->pMesh->GetNumVertices();

			/*** 頂点フォーマットのサイズを取得 ***/
			dwSizeFVF = D3DXGetFVFVertexSize(pModelData->pMesh->GetFVF());

			/*** 頂点バッファをロック ***/
			pModelData->pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVtxBuff);

			/*** 頂点の最大、最小値を取得 ***/
			for (int nCntVtx = 0; nCntVtx < nNumVtx; nCntVtx++)
			{
				D3DXVECTOR3 vtx = *(D3DXVECTOR3*)pVtxBuff;	// 頂点座標の代入

				/*** 最小値を取得 ***/
				if (pModelData->mtxMin.x > vtx.x)
				{
					pModelData->mtxMin.x = vtx.x;
				}
				if (pModelData->mtxMin.y > vtx.y)
				{
					pModelData->mtxMin.y = vtx.y;
				}
				if (pModelData->mtxMin.z > vtx.z)
				{
					pModelData->mtxMin.z = vtx.z;
				}

				/*** 最大値を取得 ***/
				if (pModelData->mtxMax.x < vtx.x)
				{
					pModelData->mtxMax.x = vtx.x;
				}
				if (pModelData->mtxMax.y < vtx.y)
				{
					pModelData->mtxMax.y = vtx.y;
				}
				if (pModelData->mtxMax.z < vtx.z)
				{
					pModelData->mtxMax.z = vtx.z;
				}

				pVtxBuff += dwSizeFVF;		// 頂点フォーマットのサイズ分ポインタを進める
			}

			/*** 頂点バッファをアンロック ***/
			pModelData->pMesh->UnlockVertexBuffer();

			// 情報を参照可能に
			pModelData->bSafe = true;

			// モデルファイル名を保存
			ZeroMemory(&pModelData->aXFileName[0], sizeof(char) * MAX_PATH);
			strcpy(&pModelData->aXFileName[0], pXFileName);

			EndDevice();

			/*** 成功 ***/
			return S_OK;
		}
	}

	EndDevice();

	/*** 上限オーバー ***/
	return E_FAIL;
}

//================================================================================================================
// --- Xファイル表示の終了 ---
//================================================================================================================
void UninitModelData(void)
{
	LPMODELDATA pModelData = &g_aModelData[0];

	for (int nCntObject = 0; nCntObject < MAX_MODELDATA; nCntObject++, pModelData++)
	{
		/*** メッシュの破棄 ***/
		RELEASE(pModelData->pMesh);

		/*** マテリアルの破棄 ***/
		RELEASE(pModelData->pBuffMat);

		/*** テクスチャの破棄 ***/
		for (int nCntTex = 0; nCntTex < MAX_MODELTEXTURE; nCntTex++)
		{
			RELEASE(pModelData->apTexture[nCntTex]);
		}
	}
}

//================================================================================================================
// --- Xファイル表示のリセット ---
//================================================================================================================
void ResetModelData(void)
{
	// モデルデータの初期化
	ZeroMemory(&g_aModelData[0], sizeof(MODELDATA) * (MAX_MODELDATA));

	// モデルデータの再読み込み
	for (int nCntObject = 0; nCntObject < g_nNumLoadModelData; nCntObject++)
	{
		LoadModelData(g_aResetFileNameXmodel[nCntObject], NULL);
	}
}

//================================================================================================================
// --- オブジェクト情報の取得 ---
//================================================================================================================
LPMODELDATA GetModelData(_In_ int nType)
{
	LPMODELDATA pModelData = &g_aModelData[nType];
	if (pModelData->bSafe == true)
	{
		return pModelData;
	}
	else
	{
		return NULL;
	}
}