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

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define EFFECT_SIZE_X		(3)			// エフェクトの基本サイズ - X
#define EFFECT_SIZE_Z		(3)			// エフェクトの基本サイズ - Y
#define EFFECT_SPD			(2.0f)		// エフェクトの移動スピード
#define EFFECT_WDSPD		(0.1f)		// エフェクトの拡縮スピード
#define MAX_EFFECT			(100000)		// エフェクトの最大出現数
#define EFFECT_LIFE			(10)		// エフェクトの体力
#define MAX_LIGHT			(3)			// シェーダーのライトの最大数

//*************************************************************************************************
//*** エフェクト構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXVECTOR3 move;		// 移動量
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	D3DXCOLOR col;			// 色
	int nLife;				// 体力
	int nDrawNum;			// 描画順番
	float fWidth;			// 幅
	float fHeight;			// 高さ
	float fRadius;
	bool bUse;				// 使っているか
}Effect;

//*************************************************************************************************
//*** シェーダー内照明構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR4 LightVec;		// ライトの向き
	D3DXVECTOR4 Color;			// ライトの色
} Light;

//*************************************************************************************************
//*** 頂点宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************
void SortEffect(int nIdxEffect);

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffEffect = NULL;			// 頂点バッファのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pMtxBuffEffect = NULL;			// マトリックスバッファへのポインタ

LPDIRECT3DVERTEXDECLARATION9 g_pMtxDeclBuffEffect = NULL;	// デクレ―ションバッファへのポインタ
LPDIRECT3DINDEXBUFFER9 g_pIdxBuffEffect;					// インデックスバッファへのポインタ
LPD3DXEFFECT g_pEffect;										// シェーダーへのポインタ
Effect g_aEffect[MAX_EFFECT];						// エフェクトの情報
D3DXVECTOR3 g_posEffect;							// ポリゴンの位置
int g_nIndexTextureEffect;							// テクスチャインデックス
int g_nNumEffect;									// エフェクトの数

//================================================================================================================
// --- エフェクトの初期化処理 ---
//================================================================================================================
void InitEffect(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VTX_GEOMETRY* pVtx;					// 頂点情報へのポインタ
	HRESULT hr;

	/*** 各変数の初期化 ***/
	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		g_aEffect[nCntEffect].pos = D3DXVECTOR3_NULL;
		g_aEffect[nCntEffect].move = D3DXVECTOR3_NULL;
		g_aEffect[nCntEffect].nLife = 0;
		g_aEffect[nCntEffect].fWidth = 0.0f;
		g_aEffect[nCntEffect].fHeight = 0.0f;
		g_aEffect[nCntEffect].bUse = false;
	}

	g_nIndexTextureEffect = -1;
	g_nNumEffect = 0;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VTX_GEOMETRY) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_GEOMETRY,
		D3DPOOL_MANAGED,
		&g_pVtxBuffEffect,
		NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffEffect->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標の設定の設定 ***/
	pVtx[0].pos.x = -EFFECT_SIZE_X;
	pVtx[0].pos.y = EFFECT_SIZE_Z;
	pVtx[0].pos.z = 0.0f;

	pVtx[1].pos.x = EFFECT_SIZE_X;
	pVtx[1].pos.y = EFFECT_SIZE_Z;
	pVtx[1].pos.z = 0.0f;

	pVtx[2].pos.x = -EFFECT_SIZE_X;
	pVtx[2].pos.y = -EFFECT_SIZE_Z;
	pVtx[2].pos.z = 0.0f;

	pVtx[3].pos.x = EFFECT_SIZE_X;
	pVtx[3].pos.y = -EFFECT_SIZE_Z;
	pVtx[3].pos.z = 0.0f;

	/*** 法線ベクトルの設定 ***/
	pVtx[0].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[1].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[2].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[3].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	/*** テクスチャ座標の設定 ***/
	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffEffect->Unlock();

	LoadTexture("data\\TEXTURE\\shadow000.jpg", &g_nIndexTextureEffect);

	//===================================================================================
	// インスタンシング用バッファ作成
	//===================================================================================

	/*** マトリックスバッファの作成 ***/
	hr = pDevice->CreateVertexBuffer(sizeof(VTX_INSTANCE) * MAX_EFFECT,	// sizeof(VTX_INSTANCE) * マトリックス数
		0,															// 特になし
		0,															// 頂点バッファではない為、FVFは指定しない
		D3DPOOL_MANAGED,											// 変わらず
		&g_pMtxBuffEffect,											// 頂点バッファへのダブルポインタ
		NULL);														// ハンドルの取得はしないためNULL

	// 描画の際に使用するインデックスを指定
	WORD aIdx[6] = { 0, 1, 2, 2, 1, 3 };
	WORD* pIdx = NULL;

	/*** インデックスバッファの作成 ***/
	hr = pDevice->CreateIndexBuffer(sizeof(WORD) * 6,	// sizeof(WORD) * インデックス数
		0,												// 特になし
		D3DFMT_INDEX16,									// 固定
		D3DPOOL_MANAGED,								// 変わらず
		&g_pIdxBuffEffect,								// インデックスバッファへのダブルポインタ
		NULL);											// ハンドルの取得はしないためNULL

	// インデックスをロック
	g_pIdxBuffEffect->Lock(0, 0, (void**)&pIdx, 0);

	for (int nCntIdx = 0; nCntIdx < sizeof aIdx / sizeof(WORD); nCntIdx++)
	{
		pIdx[nCntIdx] = aIdx[nCntIdx];		// インデックスを代入
	}

	// インデックスをアンロック
	g_pIdxBuffEffect->Unlock();

	/*** 頂点宣言作成！ ***/
	D3DVERTEXELEMENT9 declElems[] =
	{
		// 変わらない基本的な頂点情報 : StreamSource 0
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },	// ローカル座標
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },	// 法線
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },	// テクスチャ座標

		// 変動するマトリックス情報 : StreamSource 1
		{ 1,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},	// WORLD 1行目
		{ 1, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},	// WORLD 2行目
		{ 1, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},	// WORLD 3行目
		{ 1, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},	// WORLD 4行目
		{ 1, 64, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },	// 頂点カラー
		D3DDECL_END()
	};

	/*** 頂点宣言バッファを作成 ***/
	hr = pDevice->CreateVertexDeclaration(declElems, &g_pMtxDeclBuffEffect);

	/*** シェーダーを作成 ***/
	LPD3DXBUFFER pError = NULL;		// エラー文取得用
	if (FAILED(D3DXCreateEffectFromFile(pDevice,		// Direct3DDeviceへのポインタ
		"effectShader.fx",									// fxファイル名
		NULL,											// プリプロセッサマクロ定義へのポインタ(基本NULL)
		NULL,											// fxファイル内でインクルードする場合のポインタ(基本NULL)
		D3DXSHADER_DEBUG,								// 固定(無しでも良い?)
		NULL,											// シェーダーのプール指定(共有しない場合NULL)
		&g_pEffect,										// エフェクトバッファへのダブルポインタ
		&pError)) && pError != NULL)					// コンパイルエラー文取得用バッファへのポインタ
	{ // 読み込み失敗時、エラー文表示
		const char* pErrorStr = (const char*)pError->GetBufferPointer();
		OutputDebugString(pErrorStr);
		MyMathUtil::GenerateMessageBox(MB_ICONERROR,
			"EffectCompileError",
			pErrorStr);

		RELEASE(pError);

		return;
	}

	D3DCAPS9 caps;
	pDevice->GetDeviceCaps(&caps);

	if (caps.VertexShaderVersion < D3DVS_VERSION(3, 0))
	{
		// このGPUではDX9インスタンシング不可
		MyMathUtil::GenerateMessageBox(MB_ICONERROR,
			"Error",
			"インスタンシングが出来ません");
	}

	//===================================================================================
}

//================================================================================================================
// --- エフェクトの終了処理 ---
//================================================================================================================
void UninitEffect(void)
{
	/*** 頂点バッファの破棄 ***/
	RELEASE(g_pVtxBuffEffect);

	/*** マトリックスバッファバッファの破棄 ***/
	RELEASE(g_pMtxBuffEffect);

	/*** デクレ―ションバッファの破棄 ***/
	RELEASE(g_pMtxDeclBuffEffect);

	/*** インデックスバッファの破棄 ***/
	RELEASE(g_pIdxBuffEffect);

	/*** シェーダーの破棄 ***/
	RELEASE(g_pEffect);
}

//================================================================================================================
// --- エフェクトの更新処理 ---
//================================================================================================================
void UpdateEffect(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxTrans;		// 計算用マトリックス
	D3DXMATRIX mtxView;			// ビューマトリックスの取得用
	D3DXMATRIX mtxRot;			// 計算用
	VTX_INSTANCE *pInstance;	// マトリックスバッファへのポインタ

	// マトリックスバッファのロック
	g_pMtxBuffEffect->Lock(0, 0, (void**)&pInstance, 0);

	int nIdx = 0;
	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		if (!g_aEffect[nCntEffect].bUse) continue;

		// 位置を反映
		g_aEffect[nCntEffect].pos.x += g_aEffect[nCntEffect].move.x;
		g_aEffect[nCntEffect].pos.y += g_aEffect[nCntEffect].move.y;
		g_aEffect[nCntEffect].pos.z += g_aEffect[nCntEffect].move.z;
		g_aEffect[nCntEffect].fRadius -= 0.01f;

		g_aEffect[nCntEffect].nLife--;
		if (g_aEffect[nCntEffect].nLife <= 0 || g_aEffect[nCntEffect].fRadius <= 0.0f)
		{ // ライフが0以下なら死亡。ソートで枠を埋める
			g_aEffect[nCntEffect].bUse = false;
			g_nNumEffect--;
			nCntEffect--;
			continue;
		}

		D3DXMATRIX mtxScale;

		/*** ワールドマトリックスの初期化 ***/
		D3DXMatrixIdentity(&g_aEffect[nCntEffect].mtxWorld);

		D3DXMatrixScaling(&mtxScale,
			g_aEffect[nCntEffect].fRadius,
			g_aEffect[nCntEffect].fRadius,
			g_aEffect[nCntEffect].fRadius);

		D3DXMatrixMultiply(&g_aEffect[nCntEffect].mtxWorld, &g_aEffect[nCntEffect].mtxWorld, &mtxScale);

		/*** カメラのビューマトリックスを取得 ***/
		pDevice->GetTransform(D3DTS_VIEW, &mtxView);

		/*** マトリックスの逆行列を求める (※ 位置を反映する前に必ず行うこと！) ***/
		D3DXMatrixInverse(&mtxRot, NULL, &mtxView);
		/** 逆行列によって入ってしまった位置情報を初期化 **/
		mtxRot._41 = 0.0f;
		mtxRot._42 = 0.0f;
		mtxRot._43 = 0.0f;

		D3DXMatrixMultiply(&g_aEffect[nCntEffect].mtxWorld, &g_aEffect[nCntEffect].mtxWorld, &mtxRot);

		/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
		D3DXMatrixTranslation(&mtxTrans,
			g_aEffect[nCntEffect].pos.x,
			g_aEffect[nCntEffect].pos.y,
			g_aEffect[nCntEffect].pos.z);

		D3DXMatrixMultiply(&g_aEffect[nCntEffect].mtxWorld, &g_aEffect[nCntEffect].mtxWorld, &mtxTrans);

		pInstance[nIdx].mtxWorld= g_aEffect[nCntEffect].mtxWorld;
		pInstance[nIdx].col = g_aEffect[nCntEffect].col;
		nIdx++;
	}

	// マトリックスバッファのアンロック
	g_pMtxBuffEffect->Unlock();

	g_nNumEffect = nIdx;

	EndDevice();
}

//================================================================================================================
// --- エフェクトの描画処理 ---
//================================================================================================================
void DrawEffect(void)
{
	
}

//================================================================================================================
// --- エフェクトのインスタンシング描画処理 ---
//================================================================================================================
void DrawEffectInstance(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
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

	//==========================================================================
	// インスタンシング描画設定開始
	//==========================================================================

	PrintDebugProc("%d\n", g_nNumEffect);

	/*** 頂点ストリームを指定 ***/
	hr = pDevice->SetStreamSource(0, g_pVtxBuffEffect, 0, sizeof(VTX_GEOMETRY));	// 頂点バッファ
	hr = pDevice->SetStreamSource(1, g_pMtxBuffEffect, 0, sizeof(VTX_INSTANCE));	// マトリックスバッファ

	/*** インスタンス宣言 ***/
	hr = pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | g_nNumEffect);
	hr = pDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1);

	/*** 頂点及びインデックスを指定して描画 ***/
	hr = pDevice->SetVertexDeclaration(g_pMtxDeclBuffEffect);

	/*** インデックスを指定 ***/
	hr = pDevice->SetIndices(g_pIdxBuffEffect);

	/*** テクニック等を指定 ***/
	hr = g_pEffect->SetTechnique("MainTec");
	UINT passNum = 0;
	hr = g_pEffect->Begin(&passNum, 0);
	hr = g_pEffect->BeginPass(1);

	/*** テクスチャを設定 ***/
	if (GetTexture(g_nIndexTextureEffect))
	{
		g_pEffect->SetBool("bUseTexture", true);
		g_pEffect->SetTexture("TextureFileName", GetTexture(g_nIndexTextureEffect));
	}

	/*** 照明の情報を取得、型を変換して保存 ***/
	D3DLIGHT9 *pLight = GetLight();
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
	hr = g_pEffect->SetRawValue("g_aLight", &aLight[0], 0, sizeof(aLight));

	D3DXMATRIX World, view, proj, vp;
	pDevice->GetTransform(D3DTS_WORLD, &World);
	pDevice->GetTransform(D3DTS_VIEW, &view);
	pDevice->GetTransform(D3DTS_PROJECTION, &proj);
	vp = view * proj;

	g_pEffect->SetMatrix("WorldViewProj", &vp);

	/*** インデックスを使用して描画 ***/
	hr = pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
		0,
		0,
		4,
		0,
		2);

	hr = g_pEffect->EndPass();

	hr = g_pEffect->End();

	/*** クリーンアップ ***/
	hr = pDevice->SetStreamSourceFreq(0, 1);
	hr = pDevice->SetStreamSourceFreq(1, 1);

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
// --- エフェクトの発生処理 ---
//================================================================================================================
void SetEffect(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fSpd, float fWidth, float fHeight, int nLife)
{
	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		if (g_aEffect[nCntEffect].bUse == true) continue;

		D3DXVec3Normalize(&rot, &rot);

		g_aEffect[nCntEffect].pos = pos;
		g_aEffect[nCntEffect].fWidth = fWidth;
		g_aEffect[nCntEffect].fHeight = fHeight;
		g_aEffect[nCntEffect].move.x = rot.x * fSpd;
		g_aEffect[nCntEffect].move.y = rot.y * fSpd;
		g_aEffect[nCntEffect].move.z = rot.z * fSpd;
		g_aEffect[nCntEffect].col = D3DXCOLOR_NULL;
		g_aEffect[nCntEffect].nLife = EFFECT_LIFE;
		g_aEffect[nCntEffect].fRadius = 1.0f;

		if (nLife != -1)
		{
			g_aEffect[nCntEffect].nLife = nLife;
			g_aEffect[nCntEffect].col = MyMathUtil::GetRandomColor(false);
		}

		g_aEffect[nCntEffect].bUse = true;

		g_nNumEffect++;

		break;
	}
}

//================================================================================================================
// --- エフェクトのテクスチャインデックス設定処理 ---
//================================================================================================================
void SetIndexTextureEffect(int nIndexTexture)
{
	g_nIndexTextureEffect = nIndexTexture;
}

//================================================================================================================
// --- エフェクトのソート処理 ---
//================================================================================================================
void SortEffect(int nIdxEffect)
{
	Effect *pEffect = &g_aEffect[nIdxEffect];

	for (int nCntSort = nIdxEffect; nCntSort < MAX_EFFECT - 1; nCntSort++)
	{ // エフェクトの空きを埋めるようにソート
		g_aEffect[nCntSort] = g_aEffect[nCntSort + 1];
	}
}

LPD3DXEFFECT GetShader(void)
{
	return g_pEffect;
}