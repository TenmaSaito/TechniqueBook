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
#include "effect.h"
#include "light.h"

using namespace MyMathUtil;

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_MATERIAL		15
#define MAX_MATRIX			256

typedef struct
{
	LPDIRECT3DTEXTURE9 pTexture;		// テクスチャバッファ
	DWORD vtxCount;						// 頂点カウント
	DWORD PrimCount;						// ポリゴンカウント
	bool bUse;
}Material;

typedef struct
{
	D3DXVECTOR3 pos;			// 頂点座標
	D3DXVECTOR3 nor;			// 法線
	D3DXVECTOR2 tex;			// テクスチャ座標
	D3DXCOLOR col;				// 頂点カラー
}VTX_GEOMETRY_UNUSEDCOL;

typedef struct
{
	D3DXMATRIX mtxWorld;		// 一個のマテリアルで複数のマトリックスを適用する
} VTX_INSTANCE_UNUSEDCOL;

typedef struct
{
	Material aMaterial[MAX_MATERIAL];	// マテリアル毎のデータ
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
	LPDIRECT3DVERTEXDECLARATION9 pDecl;	// デクレーションバッファ
	LPD3DXMESH pMesh;					// インスタンシング後のメッシュポインタ
	LPDIRECT3DVERTEXBUFFER9 pMtxBuff;	// マトリックスバッファへのポインタ
	D3DXMATRIX aMtxWorld[MAX_MATRIX];
} MODELDATA_IC;

#define FVF_VERTEX_GU		(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)

void SetInstancingModel(int nIdxModel, D3DXMATRIX *mtxWorld);

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
MODELDATA g_aModelData[MAX_MODELDATA];		// オブジェクト情報
MODELDATA_IC g_aModelData_IC[MAX_MODELDATA];	// インスタンシング用モデルデータ
char g_aResetFileNameXmodel[MAX_MODELDATA][MAX_PATH];
int g_nNumLoadModelData;

//================================================================================================================
// --- 初期化 ---
//================================================================================================================
void InitModelData(void)
{
	// モデルデータを初期化
	ZeroMemory(&g_aModelData[0], sizeof(MODELDATA) * (MAX_MODELDATA));

	// モデルデータを初期化
	ZeroMemory(&g_aModelData_IC[0], sizeof(MODELDATA_IC) * (MAX_MODELDATA));

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

			//InstancingModelData(0);

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
	MODELDATA_IC *pIc = &g_aModelData_IC[0];

	for (int nCntObject = 0; nCntObject < MAX_MODELDATA; nCntObject++, pModelData++, pIc++)
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

//================================================================================================================
// --- オブジェクト情報のインスタンシング化 ---
//================================================================================================================
void InstancingModelData(int nIdx)
{
	// return;

	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPMODELDATA pModelData = &g_aModelData[nIdx];
	MODELDATA_IC* pIC_MODEL = &g_aModelData_IC[0];
	LPD3DXMESH pClone = nullptr;

	if (pModelData->bSafe == false)
	{
		return;
	}

	// マトリックスバッファ作成
	pDevice->CreateVertexBuffer(sizeof(VTX_INSTANCE_UNUSEDCOL) * MAX_MATRIX,
		D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_MANAGED,
		&pIC_MODEL->pMtxBuff,
		NULL);

	/*** 頂点宣言作成！ ***/
	D3DVERTEXELEMENT9 declElems[] =
	{
		// 変わらない基本的な頂点情報 : StreamSource 0
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },	// ローカル座標
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },	// 法線
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },	// テクスチャ座標
		{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },	// 頂点カラー

		// 変動するマトリックス情報 : StreamSource 1
		{ 1,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},	// WORLD 1行目
		{ 1, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},	// WORLD 2行目
		{ 1, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},	// WORLD 3行目
		{ 1, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},	// WORLD 4行目
		D3DDECL_END()
	};

	// 頂点宣言バッファ生成
	pDevice->CreateVertexDeclaration(declElems, &pIC_MODEL->pDecl);

	// インデックスバッファを取得
	pModelData->pMesh->GetIndexBuffer(&pIC_MODEL->pIdxBuff);

	DWORD dwNumAttr;

	pModelData->pMesh->GetAttributeTable(NULL, &dwNumAttr);

	if (dwNumAttr > 0)
	{
		D3DXATTRIBUTERANGE* pRange = (D3DXATTRIBUTERANGE*)malloc(sizeof(D3DXATTRIBUTERANGE) * dwNumAttr);
		pModelData->pMesh->GetAttributeTable(pRange, &dwNumAttr);

		if (pRange != nullptr)
		{
			for (DWORD nCntInstance = 0; nCntInstance < dwNumAttr; nCntInstance++)
			{
				// 頂点数取得
				pIC_MODEL->aMaterial[nCntInstance].vtxCount = pRange[nCntInstance].VertexCount;

				// プリミティブ数取得
				pIC_MODEL->aMaterial[nCntInstance].PrimCount = pRange[nCntInstance].FaceCount;

				// テクスチャバッファをコピー
				pIC_MODEL->aMaterial[nCntInstance].pTexture = pModelData->apTexture[pRange[nCntInstance].AttribId];
			}

			VTX_GEOMETRY_UNUSEDCOL* pVtx = nullptr;
			D3DXMATERIAL* pMat = nullptr;

			/*** 頂点宣言作成！(Cloneに使用) ***/
			D3DVERTEXELEMENT9 declElemsForMesh[] =
			{
				// 変わらない基本的な頂点情報 : StreamSource 0
				{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },	// ローカル座標
				{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },	// 法線
				{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },	// テクスチャ座標
				{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },	// 頂点カラー
				D3DDECL_END()
			};

			pModelData->pMesh->CloneMesh(pModelData->pMesh->GetOptions(),
				declElemsForMesh,
				pDevice,
				&pClone);		// 新規で作り替え

			pClone->LockVertexBuffer(0, (void**)&pVtx);

			pMat = (D3DXMATERIAL*)pModelData->pBuffMat->GetBufferPointer();		// マテリアル取得

			if (pRange != nullptr)
			{
				for (int nCntAttr = 0; nCntAttr < dwNumAttr; nCntAttr++)
				{
					D3DCOLOR matColor = D3DCOLOR_COLORVALUE(
						pMat[pRange[nCntAttr].AttribId].MatD3D.Diffuse.r,
						pMat[pRange[nCntAttr].AttribId].MatD3D.Diffuse.g,
						pMat[pRange[nCntAttr].AttribId].MatD3D.Diffuse.b,
						pMat[pRange[nCntAttr].AttribId].MatD3D.Diffuse.a
					);

					// 範囲内の頂点にカラーを適用
					for (DWORD nCntCol = pRange[nCntAttr].VertexStart; nCntCol < pRange[nCntAttr].VertexStart + pRange[nCntAttr].VertexCount; nCntCol++)
					{
						pVtx[nCntCol].col = matColor;
					}
				}
			}

			pClone->UnlockVertexBuffer();

			pIC_MODEL->pMesh = pClone;
			pClone->GetVertexBuffer(&pIC_MODEL->pVtxBuff);

			free(pRange);
		}
	}

	EndDevice();
}

void Draw3DModelInstancing(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPMODELDATA pModelData = &g_aModelData[0];




	EndDevice();
}

void SetInstancingModel(int nIdxModel, D3DXMATRIX* mtxWorld)
{
	
}