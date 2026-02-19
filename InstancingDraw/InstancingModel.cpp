//================================================================================================================
//
// DirectXのビルボードエフェクト表示処理 [effect.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "effect.h"
#include "Texture.h"
#include "light.h"
#include "debugproc.h"
#include "mathUtil.h"
#include "modeldata.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_MODEL			(500)		// モデルの最大出現数
#define MAX_DATA			(30)
#define MAX_MATERIAL		15
#define MAX_LIGHT			(3)			// シェーダーのライトの最大数

//*************************************************************************************************
//*** エフェクト構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXVECTOR3 rot;		// 回転
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	int nIdxModel;			// モデルインデックス
	bool bUse;				// 使っているか
}InstancingModel;

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
	LPDIRECT3DTEXTURE9 pTexture;		// テクスチャバッファ
	DWORD vtxStart;						// 始まりの頂点
	DWORD vtxCount;						// 頂点カウント
	DWORD PrimCount;					// ポリゴンカウント
	bool bUse;
}Material;

typedef struct
{
	Material aMaterial[MAX_MATERIAL];	// マテリアル毎のデータ
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
	LPDIRECT3DVERTEXDECLARATION9 pDecl;	// デクレーションバッファ
	LPD3DXMESH pMesh;					// インスタンシング後のメッシュポインタ
	LPDIRECT3DVERTEXBUFFER9 pMtxBuff;	// マトリックスバッファへのポインタ
	InstancingModel aMtx[MAX_MODEL];	// マトリックスの情報
	int dwNumAttr;						// マテリアル数
	int nCount;							// 描画数
	bool bSafe;							// 描画可能か
} MODELDATA_IC;

//*************************************************************************************************
//*** 頂点宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffInstancingModel = NULL;			// 頂点バッファのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pMtxBuffInstancingModel = NULL;			// マトリックスバッファへのポインタ

LPDIRECT3DVERTEXDECLARATION9 g_pMtxDeclBuffInstancingModel = NULL;	// デクレ―ションバッファへのポインタ
LPDIRECT3DINDEXBUFFER9 g_pIdxBuffInstancingModel;					// インデックスバッファへのポインタ
LPD3DXEFFECT g_pInstancingModel;										// シェーダーへのポインタ
MODELDATA_IC g_aIC[MAX_DATA];
D3DXVECTOR3 g_posInstancingModel;							// ポリゴンの位置
int g_nIndexTextureInstancingModel;							// テクスチャインデックス
int g_nNumInstancingModel;									// エフェクトの数

//================================================================================================================
// --- エフェクトの初期化処理 ---
//================================================================================================================
void InitInstancingModel(void)
{
	ZeroMemory(&g_aIC[0], sizeof(g_aIC));

	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPMODELDATA pModelData = GetModelData(0);
	MODELDATA_IC* pIC_MODEL = &g_aIC[0];
	LPD3DXMESH pClone = nullptr;

	if (pModelData->bSafe == false)
	{
		return;
	}

	pIC_MODEL->dwNumAttr = pModelData->dwNumMat;

	// マトリックスバッファ作成
	pDevice->CreateVertexBuffer(sizeof(VTX_INSTANCE_UNUSEDCOL) * MAX_MODEL,
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
		pIC_MODEL->dwNumAttr = dwNumAttr;

		if (pRange != nullptr)
		{
			for (DWORD nCntInstance = 0; nCntInstance < dwNumAttr; nCntInstance++)
			{
				// 頂点数取得
				pIC_MODEL->aMaterial[nCntInstance].vtxStart = pRange[nCntInstance].VertexStart;
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

			pIC_MODEL->bSafe = true;
		}
	}

	EndDevice();
}

//================================================================================================================
// --- エフェクトの終了処理 ---
//================================================================================================================
void UninitInstancingModel(void)
{
	MODELDATA_IC* pIc = &g_aIC[0];

	for (int nCntObject = 0; nCntObject < MAX_DATA; nCntObject++, pIc++)
	{
		/*** メッシュの破棄 ***/
		RELEASE(pIc->pMesh);

		/*** インデックスの破棄 ***/
		RELEASE(pIc->pIdxBuff);

		/*** 頂点宣言の破棄 ***/
		RELEASE(pIc->pDecl);
	}
}

//================================================================================================================
// --- エフェクトの更新処理 ---
//================================================================================================================
void UpdateInstancingModel(void)
{
	
}

//================================================================================================================
// --- エフェクトの描画処理 ---
//================================================================================================================
void DrawInstancingModel(void)
{
	//*************************************************************************************************
	//*** シェーダー内照明構造体の定義 ***
	//*************************************************************************************************
	typedef struct
	{
		D3DXVECTOR4 LightVec;		// ライトの向き
		D3DXVECTOR4 Color;			// ライトの色
	} Light;

	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPD3DXEFFECT pEffect = GetShader();
	MODELDATA_IC* pIc = &g_aIC[0];
	HRESULT hr;

	/*** ライトオフ ***/
	pDevice->LightEnable(0, FALSE);

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

	// アルファブレンディングを加算合成に設定！
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	/*** アルファテストを有効にする ***/
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);		// アルファテストを有効
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);	// 基準値よりも大きい場合にZバッファに書き込み
	pDevice->SetRenderState(D3DRS_ALPHAREF, 254);				// 基準値

	for (int nCntObject = 0; nCntObject < MAX_DATA; nCntObject++, pIc++)
	{
		if (pIc->bSafe != true) continue;

		//==========================================================================
		// インスタンシング描画設定開始
		//==========================================================================

		/*** 頂点ストリームを指定 ***/
		hr = pDevice->SetStreamSource(0, pIc->pVtxBuff, 0, sizeof(VTX_GEOMETRY_UNUSEDCOL));	// 頂点バッファ
		hr = pDevice->SetStreamSource(1, pIc->pMtxBuff, 0, sizeof(VTX_INSTANCE_UNUSEDCOL));	// マトリックスバッファ

		/*** インスタンス宣言 ***/
		hr = pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | pIc->nCount);
		hr = pDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1);

		/*** 頂点及びインデックスを指定して描画 ***/
		hr = pDevice->SetVertexDeclaration(pIc->pDecl);

		/*** テクニック等を指定 ***/
		hr = pEffect->SetTechnique("MainTec");
		UINT passNum = 0;
		hr = pEffect->Begin(&passNum, 0);

		/*** 照明の情報を取得、型を変換して保存 ***/
		D3DLIGHT9* pLight = GetLight();
		Light aLight[MAX_LIGHT];
		for (int nCntLight = 0; nCntLight < MAX_LIGHT; nCntLight++, pLight++)
		{
			aLight[nCntLight].LightVec = D3DXVECTOR4(pLight->Direction, 1.0f);
			aLight[nCntLight].Color = D3DXVECTOR4(pLight->Diffuse.r,
				pLight->Diffuse.g,
				pLight->Diffuse.b,
				pLight->Diffuse.a);
		}

		/*** 照明の情報をシェーダーへ渡す ***/
		hr = pEffect->SetRawValue("g_aLight", &aLight[0], 0, sizeof(aLight));

		D3DXMATRIX World, view, proj, wvp;
		pDevice->GetTransform(D3DTS_WORLD, &World);
		pDevice->GetTransform(D3DTS_VIEW, &view);
		pDevice->GetTransform(D3DTS_PROJECTION, &proj);
		wvp = World * view * proj;

		pEffect->SetMatrix("WorldViewProj", &wvp);

		/*** インデックスを指定 ***/
		hr = pDevice->SetIndices(pIc->pIdxBuff);

		for (int nCntMat = 0; nCntMat < (int)pIc->dwNumAttr; nCntMat++)
		{
			/*** テクスチャを設定 ***/
			if (pIc->aMaterial[nCntMat].pTexture)
			{
				hr = pEffect->BeginPass(0);
				pEffect->SetBool("bUseTexture", true);
				pEffect->SetTexture("TextureFileName", pIc->aMaterial[nCntMat].pTexture);
			}
			else
			{
				hr = pEffect->BeginPass(2);
				pEffect->SetBool("bUseTexture", false);
			}

			/*** インデックスを使用して描画 ***/
			hr = pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
				0,
				0,
				pIc->aMaterial[nCntMat].vtxCount,
				pIc->aMaterial[nCntMat].vtxStart,
				pIc->aMaterial[nCntMat].PrimCount);

			hr = pEffect->EndPass();
		}

		hr = pEffect->End();

		/*** クリーンアップ ***/
		hr = pDevice->SetStreamSourceFreq(0, 1);
		hr = pDevice->SetStreamSourceFreq(1, 1);
	}

	//==========================================================================

	/*** アルファテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);		// アルファテストを無効化
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);	// 無条件にZバッファに書き込み
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0);					// 基準値

	// アルファブレンディングを元に戻す！！(重要！)
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	/*** ライトオン ***/
	pDevice->LightEnable(0, TRUE);

	EndDevice();
}

//================================================================================================================
// --- エフェクトのインスタンシング描画処理 ---
//================================================================================================================
void DrawInstancingModelInstance(void)
{
	
}

//================================================================================================================
// --- エフェクトの発生処理 ---
//================================================================================================================
void SetInstancingModel(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nIdxModel)
{
	for (int nCntInstancingModel = 0; nCntInstancingModel < MAX_MODEL; nCntInstancingModel++)
	{
		if (g_aIC[nIdxModel].aMtx[nCntInstancingModel].bUse == true) continue;

		D3DXVec3Normalize(&rot, &rot);

		g_aIC[nIdxModel].aMtx[nCntInstancingModel].pos = pos;
		g_aIC[nIdxModel].aMtx[nCntInstancingModel].rot = rot;
		g_aIC[nIdxModel].aMtx[nCntInstancingModel].nIdxModel = nIdxModel;
		g_aIC[nIdxModel].aMtx[nCntInstancingModel].bUse = true;
		g_aIC[nIdxModel].nCount++;

		break;
	}
}