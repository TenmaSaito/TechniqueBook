//================================================================================================================
//
// DirectXのテクスチャファイル読み込み表示処理 [Texture.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "Texture.h"
#include "mathUtil.h"

using namespace MyMathUtil;

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
TEXTURE_INFO g_aTexInfo[MAX_TEXTURE];		// オブジェクト情報
char g_aResetFileNameTexture[MAX_TEXTURE][MAX_PATH];
int g_nNumLoadTexture;

//================================================================================================================
// --- 初期化 ---
//================================================================================================================
void InitTexture(void)
{
	ZeroMemory(&g_aTexInfo[0], sizeof(TEXTURE_INFO) * (MAX_TEXTURE));

	ZeroMemory(&g_aResetFileNameTexture[0], sizeof(g_aResetFileNameTexture));

	g_nNumLoadTexture = 0;
}

//================================================================================================================
// --- モデルの読み込み ---
//================================================================================================================
HRESULT LoadTexture(_In_ const char *pTexFileName, int *pOutIdx)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	HRESULT hr;
	LPTEXTURE_INFO pTexInfo = &g_aTexInfo[0];

	if (pOutIdx)
	{ // エラー番号を格納
		*pOutIdx = ERROR_TEXTURE;
	}

	/*** 過去に読み込んでいないか確認 ***/
	for (int nCntTexture = 0; nCntTexture < MAX_TEXTURE; nCntTexture++, pTexInfo++)
	{
		if (strcmp(&pTexInfo->aTexFileName[0], pTexFileName) == NULL)
		{
			if (pOutIdx)
			{ // テクスチャ番号を格納
				*pOutIdx = nCntTexture;
			}

			EndDevice();

			return S_OK;
		}
	}

	pTexInfo = &g_aTexInfo[0];

	/*** Xファイルの読み込み ***/
	for (int nCntTexture = 0; nCntTexture < MAX_TEXTURE; nCntTexture++, pTexInfo++)
	{
		if (pTexInfo->bUse != true)
		{
			// 使用状態に
			pTexInfo->bUse = true;

			// テクスチャ読み込み
			hr = D3DXCreateTextureFromFile(pDevice,
				pTexFileName,
				&pTexInfo->pTexture);

			// デバイスリセット時用変数に保存
			strcpy(&g_aResetFileNameTexture[nCntTexture][0], pTexFileName);

			if (FAILED(hr))
			{ // 読み込み失敗時
				GenerateMessageBox(MB_ICONERROR,
					"TextureLoadError",
					"テクスチャ読み込みに失敗。\n対象パス:%s",
					pTexFileName);

				EndDevice();

				return hr;
			}

			// 相対パスになっているか確認
			CheckPath(pTexFileName);

			// テクスチャ呼び出しを可能に
			pTexInfo->bSafe = true;

			// テクスチャパスを保存
			ZeroMemory(&pTexInfo->aTexFileName[0], sizeof(char) * MAX_PATH);
			strcpy(&pTexInfo->aTexFileName[0], pTexFileName);

			if (pOutIdx)
			{ // テクスチャ番号を格納
				*pOutIdx = nCntTexture;
			}

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
void UninitTexture(void)
{
	LPTEXTURE_INFO pTexInfo = &g_aTexInfo[0];

	for (int nCntTexture = 0; nCntTexture < MAX_TEXTURE; nCntTexture++, pTexInfo++)
	{
		RELEASE(pTexInfo->pTexture);
	}
}

//================================================================================================================
// --- Xファイル表示のリセット ---
//================================================================================================================
void ResetTexture(void)
{
	ZeroMemory(&g_aTexInfo[0], sizeof(TEXTURE_INFO) * (MAX_TEXTURE));

	for (int nCntObject = 0; nCntObject < g_nNumLoadTexture; nCntObject++)
	{
		LoadTexture(g_aResetFileNameTexture[nCntObject], NULL);
	}
}

//================================================================================================================
// --- オブジェクト情報の取得 ---
//================================================================================================================
LPDIRECT3DTEXTURE9 GetTexture(_In_ int nType)
{
	if (nType < 0 || nType >= MAX_TEXTURE) return NULL;
	LPTEXTURE_INFO pTexInfo = &g_aTexInfo[nType];
	if (pTexInfo->bSafe == true)
	{
		return pTexInfo->pTexture;
	}
	else
	{
		return NULL;
	}
}