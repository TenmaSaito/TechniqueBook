//================================================================================================================
//
// DirectXのフェード処理 [fade.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "mode.h"
#include "fade.h"
#include "input.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureFade = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffFade = NULL;	// 頂点バッファのポインタ
FADE g_fade;									// フェードの種類
MODE g_modeNext;								// 画面(モード)の種類
D3DXCOLOR g_colorFade;							// ポリゴンの色
float g_fAlphaFade;								// フェードのα値の増減値

//================================================================================================================
// --- フェードの初期化 ---
//================================================================================================================
void InitFade(MODE modeNext)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ

	g_fade = FADE_IN;					// フェードイン状態に
	g_modeNext = modeNext;				// 次の画面を設定
	g_colorFade.a = 1.0f;				// alpha値を初期化
	g_fAlphaFade = (1.0f / (float)NORMAL_FADECOUNT);		// フェード時間を初期化

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_2D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffFade,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffFade->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標の設定の設定 ***/
	pVtx[0].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(SCREEN_WIDTH, 0.0f, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(0.0f, SCREEN_HEIGHT, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);

	/*** rhwの設定 ***/
	pVtx[0].rhw = 1.0f;
	pVtx[1].rhw = 1.0f;
	pVtx[2].rhw = 1.0f;
	pVtx[3].rhw = 1.0f;

	/*** 頂点カラー設定 ***/
	pVtx[0].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, g_colorFade.a);
	pVtx[1].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, g_colorFade.a);
	pVtx[2].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, g_colorFade.a);
	pVtx[3].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, g_colorFade.a);

	/*** テクスチャ座標の設定 ***/
	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffFade->Unlock();
}

//================================================================================================================
// --- フェードの終了 ---
//================================================================================================================
void UninitFade(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTextureFade != NULL)
	{
		g_pTextureFade->Release();
		g_pTextureFade = NULL;
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffFade != NULL)
	{
		g_pVtxBuffFade->Release();
		g_pVtxBuffFade = NULL;
	}
}

//================================================================================================================
// --- フェードの更新 ---
//================================================================================================================
void UpdateFade(void)
{
	VERTEX_2D* pVtx;						// 頂点情報へのポインタ

	/*** フェードの更新 ***/
	if (g_fade != FADE_NONE)
	{
		if (g_fade == FADE_IN)
		{// フェードイン
			g_colorFade.a -= g_fAlphaFade;	// α値を低下
			if (g_colorFade.a <= 0.0f)		
			{// フェードイン終了
				g_colorFade.a = 0.0f;		// α値を0にリセット
				g_fade = FADE_NONE;			// フェードを解除
			}

			// タイトル画面のフェードでエンターが押されたとき
			if ((GetKeyboardTrigger(DIK_RETURN) == true
				|| GetJoypadTrigger(JOYKEY_A) == true
				|| GetJoypadTrigger(JOYKEY_START) == true) 
				&& g_modeNext == MODE_TITLE)
			{
				g_colorFade.a = 0.0f;		// フェードをスキップ
			}
		}
		else if(g_fade == FADE_OUT)
		{// フェードアウト
			g_colorFade.a += g_fAlphaFade;	// α値を増加
			if (g_colorFade.a >= 1.0f)
			{// フェードアウト終了
				g_colorFade.a = 1.0f;		// α値を1に設定
				g_fade = FADE_IN;			// フェードアウトからフェードインへ移行

				// モード設定
				SetMode(g_modeNext);		// 指定されたモードに設定
			}
		}

		/*** 頂点バッファの設定 ***/
		g_pVtxBuffFade->Lock(0, 0, (void**)&pVtx, 0);

		/*** 頂点カラー設定 ***/
		pVtx[0].col = g_colorFade;
		pVtx[1].col = g_colorFade;
		pVtx[2].col = g_colorFade;
		pVtx[3].col = g_colorFade;

		/*** 頂点バッファの設定を終了 ***/
		g_pVtxBuffFade->Unlock();
	}
}

//================================================================================================================
// --- フェードの描画 ---
//================================================================================================================
void DrawFade(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** 頂点バッファをデータストリームに設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffFade, 0, sizeof(VERTEX_2D));

	/*** 頂点フォーマットの設定 ***/
	pDevice->SetFVF(FVF_VERTEX_2D);

	/*** テクスチャの設定 ***/
	pDevice->SetTexture(0, NULL);				// NULL

	/*** ポリゴンの描画 ***/
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
					       0,						// 描画する最初の頂点インデックス
						   2);						// 描画するプリミティブの数
}

//================================================================================================================
// --- フェードの設置 ---
//================================================================================================================
void SetFade(MODE modeNext, int nCounterFade)
{
	/*** フェード設定 ***/
	g_fade = FADE_OUT;			// フェードアウトに設定
	g_modeNext = modeNext;		// フェードアウト後のモードを保存
	g_colorFade = NORMAL_FADE;	// フェードの色を黒(透明)に初期化
	g_fAlphaFade = (1.0f / (float)nCounterFade);
}

//================================================================================================================
// --- フェードの取得 ---
//================================================================================================================
FADE GetFade(void)
{
	return g_fade;
}