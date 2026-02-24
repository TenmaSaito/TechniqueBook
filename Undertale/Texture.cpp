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
HRESULT LoadTexture(_In_ const char* pTexFileName)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	HRESULT hr;
	LPTEXTURE_INFO pTexInfo = &g_aTexInfo[0];

	/*** 過去に読み込んでいないか確認 ***/
	for (int nCntTexture = 0; nCntTexture < MAX_TEXTURE; nCntTexture++, pTexInfo++)
	{
		if (strcmp(&pTexInfo->aTexFileName[0], pTexFileName) == NULL)
		{
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
			pTexInfo->bUse = true;

			hr = D3DXCreateTextureFromFile(pDevice,
				pTexFileName,
				&pTexInfo->pTexture);

			strcpy(&g_aResetFileNameTexture[nCntTexture][0], pTexFileName);

			if (FAILED(hr))
			{
				MyMathUtil::GenerateMessageBox(MB_ICONERROR,
					"TextureLoadError",
					"テクスチャ読み込みに失敗。\n対象パス:%s",
					pTexFileName);

				EndDevice();

				return hr;
			}

			pTexInfo->bSafe = true;

			ZeroMemory(&pTexInfo->aTexFileName[0], sizeof(char) * MAX_PATH);
			strcpy(&pTexInfo->aTexFileName[0], pTexFileName);

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
		LoadTexture(g_aResetFileNameTexture[nCntObject]);
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