//================================================================================================================
//
// windowsアプリケーションの作成->ウィンドウの表示処理 [main.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルード ***
//**********************************************************************************
#include "main.h"
#include "input.h"
#include "mathUtil.h"
#include "debugproc.h"
#include "thread.h"
#include "fade.h"

// MODE
#include "mode.h"

using namespace MyMathUtil;

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define CLASS_NAME		"WindowClass"				// ウィンドウクラスの名前
#define WINDOW_NAME		"ウィンドウ表示処理"		// キャプションに表示される名前
#define SCREEN_WIDTH		(1280)					// ウィンドウの幅
#define SCREEN_HEIGHT		(720)					// ウィンドウの高さ
#define MODE_ON										// モード切り替え

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
bool InitWindow(HINSTANCE hInstance, RECT rect, LPWNDCLASSEX lpWcex, HWND *phWnd);
void UninitWindow(LPWNDCLASSEX lpWcex);
void MessageLoop(LPMSG lpMsg);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);
void ToggleFullscreen(HWND hWnd);
CREATE_LOOPID(loopThread);

void DrawPoly(void);

//**********************************************************************************
//*** グローバル変数 ***
//**********************************************************************************
LPDIRECT3D9				g_pD3D = NULL;					// Direct3Dオブジェクトへのポインタ
LPDIRECT3DDEVICE9		g_pD3DDevice = NULL;			// Direct3Dデバイスへのポインタ
LPDIRECT3DSURFACE9 g_pBackBufferSurface = NULL;			// もともとレンダリングターゲットにセットされていたサーフェイス
LPDIRECT3DSURFACE9 g_pBackDepthSurface = NULL;			// もともとレンダリングターゲットにセットされていたサーフェイス
HWND g_hWnd = NULL;										// 獲得したウィンドウハンドル
int g_nCountFPS = 0;									// FPSカウンタ
bool g_isFullscreen = false;							// フルスクリーンの使用状況
RECT g_windowRect;										// ウィンドウサイズ
bool g_bMainThread;										// メインスレッドの実行状況
MultiData g_mdDirect3DDevice;							// スレッドセーフ
LPDIRECT3DSURFACE9 g_pSurface;							// ポストエフェクト用サーフェイス
LPDIRECT3DTEXTURE9 g_pTexSurface;						// ポストエフェクト用テクスチャ
LPDIRECT3DSURFACE9 g_pDepthSurface;						// レンダリング対象のテクスチャ用深度バッファ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffSurface;				// ポストエフェクト用ポリゴンバッファ
LPD3DXEFFECT g_pEffect;

//================================================================================================================
// --- メイン関数 ---
//================================================================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstancePrev, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);			// メモリリーク検知用のフラグ
#endif // _DEBUG
	HWND hWnd = NULL;							// ウィンドウハンドル
	MSG msg = {};								// メッセージを格納する変数
	RECT rect = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };
	WNDCLASSEX wcex = {};

	// ウィンドウ作成
	if (!InitWindow(hInstance, rect, &wcex, &hWnd))
	{ // ウィンドウ作成失敗
		UninitWindow(&wcex);

		return -1;
	}

	g_hWnd = hWnd;
	g_bMainThread = true;

	// 初期化処理
	if (FAILED(Init(hInstance, hWnd, TRUE)))
	{// 初期化処理が失敗した場合
		MessageBox(hWnd, "Initに失敗しました。", "Error (0)", MB_ICONERROR);
		Uninit();
		return -1;
	}

	/*** シェーダーを作成 ***/
	LPD3DXBUFFER pError = NULL;		// エラー文取得用
	if (FAILED(D3DXCreateEffectFromFile(g_pD3DDevice,		// Direct3DDeviceへのポインタ
		"posteffect.fx",									// fxファイル名
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
	}

	g_pD3DDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffSurface,
		NULL);

	VERTEX_2D *pVtx;

	g_pVtxBuffSurface->Lock(0, 0, (void**)&pVtx, 0);

	SetFullScreenPolygon(pVtx);

	pVtx[0].rhw = 1.0f;
	pVtx[1].rhw = 1.0f;
	pVtx[2].rhw = 1.0f;
	pVtx[3].rhw = 1.0f;

	pVtx[0].col = D3DXCOLOR_NULL;
	pVtx[1].col = D3DXCOLOR_NULL;
	pVtx[2].col = D3DXCOLOR_NULL;
	pVtx[3].col = D3DXCOLOR_NULL;

	pVtx[0].tex = D3DXVECTOR2(0, 0);
	pVtx[1].tex = D3DXVECTOR2(1, 0);
	pVtx[2].tex = D3DXVECTOR2(0, 1);
	pVtx[3].tex = D3DXVECTOR2(1, 1);

	g_pVtxBuffSurface->Unlock();

	// ウィンドウの表示
	ShowWindow(hWnd, nCmdShow);					// ウィンドウの表示状態を設定
	UpdateWindow(hWnd);							// クライアント領域を更新

	// メッセージループ
	MessageLoop(&msg);
	
	g_pVtxBuffSurface->Release();

	// DirectXの終了
	Uninit();

	// ウィンドウの削除
	UninitWindow(&wcex);

	// マルチスレッドのクリティカルセクションの終了
	g_mdDirect3DDevice.~MultiData();

	return (int)msg.wParam;
}

//================================================================================================================
// --- メッセージループ ---
//================================================================================================================
void MessageLoop(LPMSG lpMsg)
{
	DWORD dwCurrentTime;						// 現在時刻
	DWORD dwExecLastTime;						// 最後に処理した時刻
	DWORD dwFrameCount;							// フレームカウント
	DWORD dwFPSLastTime;						// 最後にFPSを計測した時刻

	//分解能を設定
	timeBeginPeriod(1);

	dwCurrentTime = 0;							// 初期化
	dwExecLastTime = timeGetTime();				// 現在時刻を取得
	dwFrameCount = 0;
	dwFPSLastTime = timeGetTime();

	// メッセージループ
	while (lpMsg != NULL)
	{
		if (PeekMessage(lpMsg, NULL, 0, 0, PM_REMOVE) != 0)
		{// windowsの処理
			if (lpMsg->message == WM_QUIT)
			{// WM_QUITメッセージを受けると、メッセージループを抜ける
				break;
			}
			else
			{
				// メッセージの設定
				TranslateMessage(lpMsg);				// 仮想キーメッセージを文字メッセージへ変換

				DispatchMessage(lpMsg);					// ウィンドウプロシージャへメッセージを送出
			}
		}
		else
		{// DirectXの処理
			dwCurrentTime = timeGetTime();				// 現在時刻を取得

			if ((dwCurrentTime - dwFPSLastTime) >= 500)
			{// 0.5秒経過
				// FPSを計測
				g_nCountFPS = (dwFrameCount * 1000) / (dwCurrentTime - dwFPSLastTime);

				dwFPSLastTime = dwCurrentTime;			// FPSを計測した時刻を取得
				dwFrameCount = 0;						// フレームカウントをクリア
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))
			{//60分の1秒経過

				dwExecLastTime = dwCurrentTime;			//処理開始時刻[現在時刻]を保存

				// 更新処理
				Update();

				// 描画処理
				Draw();

				dwFrameCount++;							// フレームカウントを加算
			}
		}
	}

	g_bMainThread = false;
}

//================================================================================================================
// ウィンドウプロシージャ
//================================================================================================================
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nID;

	switch (uMsg)
	{
	case WM_DESTROY:		// ウィンドウ破棄のメッセージ
		// WM_QUITメッセージを送る
		PostQuitMessage(0);
		break;

		// キー押下のメッセージ
	case WM_KEYDOWN:
		// 押されたキーの判定
		switch (wParam)
		{
		case VK_ESCAPE:
			// 終了確認
			nID = MessageBox(hWnd, "終了しますか？", "終了確認メッセージ", (MB_YESNO | MB_ICONINFORMATION));
			if (nID == IDYES)
			{// もしYESだった場合
				// ウィンドウを破棄する(WM_DESTROYメッセージを送る)
				DestroyWindow(hWnd);
			}

			break;

		case VK_F11:
			// F11でフルスクリーン
			ToggleFullscreen(hWnd);
			break;
		}
		break;

	case WM_CLOSE:		// 閉じるボタン押下のメッセージ

		// ウィンドウを破棄する(WM_DESTROYメッセージを送る)
		DestroyWindow(hWnd);

		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//================================================================================================================
// ウィンドウ作成
//================================================================================================================
bool InitWindow(HINSTANCE hInstance, RECT rect, LPWNDCLASSEX lpWcex, HWND* phWnd)
{
	HWND hWnd = NULL;

	*lpWcex =
	{
		sizeof(WNDCLASSEX),						// ウィンドウクラスのメモリサイズ
		CS_CLASSDC,								// ウィンドウのスタイル
		WindowProc,								// ウィンドウプロシージャ
		0,										// 0
		0,										// 0
		hInstance,								// インスタンスハンドル
		LoadIcon(NULL,IDI_APPLICATION),			// タスクバーのアイコン
		LoadCursor(NULL,IDC_ARROW),				// マウスカーソル
		(HBRUSH)(COLOR_WINDOW + 1),				// クライアント領域の背景色
		NULL,									// メニューバー
		CLASS_NAME,								// ウィンドウクラスの名前
		NULL									// ファイルのアイコン
	};

	// ウィンドウクラスの登録
	RegisterClassEx(lpWcex);

	// クライアント領域を指定のサイズに調整
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	// ウィンドウの生成
	hWnd = CreateWindowEx(
		0,										// 拡張ウィンドウスタイル
		CLASS_NAME,								// ウィンドウクラスの名前
		WINDOW_NAME,							// ウィンドウの名前
		WS_OVERLAPPEDWINDOW,					// ウィンドウスタイル
		CW_USEDEFAULT,							// ウィンドウの左上X座標
		CW_USEDEFAULT,							// ウィンドウの左上Y座標
		(rect.right - rect.left),				// ウィンドウの幅
		(rect.bottom - rect.top),				// ウィンドウの高さ
		NULL,									// 親ウィンドウのハンドル
		NULL,									// メニュー(もしくは子ウィンドウ)ハンドル
		hInstance,								// インスタンスハンドル
		NULL);									// ウィンドウ作成データ

	if (hWnd == NULL)
	{
		MessageBox(NULL, "ウィンドウハンドルが確保されていません!", "警告！", MB_ICONWARNING);
		return false;
	}

	if (phWnd != NULL) *phWnd = hWnd;

	return true;
}

//================================================================================================================
// --- ウィンドウ削除 ---
//================================================================================================================
void UninitWindow(LPWNDCLASSEX lpWcex)
{
	// ウィンドウクラスの登録解除
	UnregisterClass(CLASS_NAME, lpWcex->hInstance);
}

//================================================================================================================
// --- 初期化処理 ---
//================================================================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	D3DDISPLAYMODE d3ddm;			// ディスプレイモード
	D3DPRESENT_PARAMETERS d3dpp;	// プレゼンテーションパラメータ		

	// Direct3Dオブジェクトの作成
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_pD3D == NULL)
	{
		MessageBox(hWnd, "Direct3Dオブジェクトの作成に失敗しました。", "Error (-1)", MB_ICONERROR);
		return E_FAIL;
	}

	// 現在のディスプレイモードを取得
	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,
		&d3ddm)))
	{
		MessageBox(hWnd, "ディスプレイモードの取得に失敗しました。", "Error (-1)", MB_ICONERROR);
		return E_FAIL;
	}

	// デバイスのプレゼンテーションパラメータの設定
	ZeroMemory(&d3dpp, sizeof(d3dpp));			// パラメータのゼロクリア

	d3dpp.BackBufferWidth = SCREEN_WIDTH;		// ゲームの画面サイズ(横)
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;		// ゲームの画面サイズ(高さ)
	d3dpp.BackBufferFormat = d3ddm.Format;		// バックバッファの形式
	d3dpp.BackBufferCount = 1;					// バックバッファの数
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// ダブルバッファの切り替え(映像信号と同期)
	d3dpp.EnableAutoDepthStencil = TRUE;		// デプスバッファとステンシルバッファを作成
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// デプスバッファとして16bitを使う
	d3dpp.Windowed = bWindow;					// ウィンドウモード
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;			// リフレッシュレート
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;			// インターバル

	// Direct3Dデバイスの作成
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
		&d3dpp,
		&g_pD3DDevice)))
	{
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
			&d3dpp,
			&g_pD3DDevice)))
		{
			if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
				D3DDEVTYPE_REF,
				hWnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
				&d3dpp,
				&g_pD3DDevice)))
			{
				MessageBox(hWnd, "Direct3Dデバイスの作成に失敗しました。", "Error (-1)", MB_ICONERROR);
				return E_FAIL;
			}
		}
	}

	// レンダーステートの設定(消さないこと！ALPHA値の設定を適用する為の設定！)
	g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// サンプラーステートの設定
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);		// テクスチャの拡縮補間の設定
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);		// テクスチャの繰り返しの設定
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	// テクスチャステージステートの設定(テクスチャのアルファブレンドの設定[テクスチャとポリゴンのALPHA値を混ぜる！])
	g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	// キーボードの初期化処理
	if (FAILED(InitKeyboard(hInstance, hWnd)))
	{
		MessageBox(hWnd, "InitKeyboardに失敗しました。", "Error (0)", MB_ICONERROR);
		return E_FAIL;
	}

	// ジョイパッドの初期化処理
	if (FAILED(InitJoypad()))
	{
		MessageBox(hWnd, "InitJoypadに失敗しました。", "Error (1)", MB_ICONERROR);
		return E_FAIL;
	}

	// マウスの初期化処理
	if (FAILED(InitMouse(hWnd)))
	{
		MessageBox(hWnd, "InitMouseに失敗しました。", "Error (2)", MB_ICONERROR);
		return E_FAIL;
	}

	// デバッグフォント作成
	InitDebugProc();

#ifdef MODE_ON

	InitFade(GetMode());
	InitMode();

	/*** タイトルBGMをフェードイン ***/
	//FadeSound(SOUND_LABEL_BGM_TITLE);

#endif // MODE_ON

	// バックバッファのサーフェイスを記憶しておく
	g_pD3DDevice->GetRenderTarget(0, &g_pBackBufferSurface);

	// ステンシルバッファを取得しておく
	g_pD3DDevice->GetDepthStencilSurface(&g_pBackDepthSurface);

	// サーフェイスを作成
	if (FAILED(D3DXCreateTexture(
		g_pD3DDevice,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&g_pTexSurface)))
	{
		// テクスチャの作成に失敗
		MyMathUtil::GenerateMessageBox(MB_ICONERROR,
			"ERROR",
			"テクスチャの作成に失敗？！");

		return E_FAIL;
	}

	if (FAILED(g_pD3DDevice->CreateDepthStencilSurface(
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		D3DFMT_D16,
		D3DMULTISAMPLE_NONE,
		0,
		TRUE,
		&g_pDepthSurface,
		NULL)))
	{
		// ステンシルバッファの作成に失敗
		MyMathUtil::GenerateMessageBox(MB_ICONERROR,
			"ERROR",
			"ステンシルバッファの作成に失敗？！");

		return E_FAIL;
	}

	LPDIRECT3DSURFACE9 pSurface;
	if (FAILED(g_pTexSurface->GetSurfaceLevel(0, &pSurface)))
	{
		// サーフェイス取得に失敗
		MyMathUtil::GenerateMessageBox(MB_ICONERROR,
			"ERROR",
			"サーフェイス取得に失敗？！");

		return E_FAIL;
	}

	// 記憶しておく
	g_pSurface = pSurface;

	return S_OK;
}

//================================================================================================================
// --- 終了処理 ---
//================================================================================================================
void Uninit(void)
{
#ifdef MODE_ON
	// モードの解放
	UninitMode();

	// フェードの終了処理
	UninitFade();
#endif
	// キーボードの終了処理
	UninitKeyboard();

	// ジョイパッドの終了処理
	UninitJoypad();

	// マウスの終了処理
	UninitMouse();

	// デバッグフォント削除
	UninitDebugProc();

	RELEASE(g_pDepthSurface);

	RELEASE(g_pSurface);

	RELEASE(g_pTexSurface);

	// Direct3Dデバイスの破棄
	RELEASE(g_pD3DDevice);

	// Direct3Dオブジェクトの破棄
	RELEASE(g_pD3D);
}

//================================================================================================================
// --- 更新処理 ---
//================================================================================================================
void Update(void)
{
	CHAR aStr[MAX_PATH] = {};
	static int nCount = 0;
	static int nID[3] = {};

	PrintDebugProc("FPS : %d\n", g_nCountFPS);

	//キーボードの更新処理(これより上に書くな)
	UpdateKeyboard();

	//ジョイパッドの更新処理
	UpdateJoypad();

	// マウスの更新処理
	UpdateMouse();

	// デバッグ表示更新
	UpdateDebugProc();

	/*** 現在の画面(モード)の更新処理 ***/
	UpdateMode();

	// フェードの更新処理
	UpdateFade();
}

//================================================================================================================
// --- 描画処理 ---
//================================================================================================================
void Draw(void)
{
	HRESULT hr;
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	// 深度バッファをテクスチャ深度バッファに変更
	pDevice->SetDepthStencilSurface(g_pDepthSurface);

	// レンダーターゲットをテクスチャバッファに変更
	pDevice->SetRenderTarget(0, g_pSurface);

	// 画面クリア(バックバッファとZバッファのクリア)
	pDevice->Clear(0, NULL,
		(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER),
		D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	// 描画開始
	if (SUCCEEDED(pDevice->BeginScene()))
	{// 描画開始が成功した場合
#ifdef MODE_ON
		DrawMode();

		// フェードの描画処理
		DrawFade();
#endif
		// デバック表示描画
		DrawDebugProc();

		// 描画終了
		g_pD3DDevice->EndScene();
	}

	if (GetKeyboardTrigger(DIK_P))
	{ // 画面を画像で書き出し
		HRESULT hr;
		hr = D3DXSaveTextureToFile("data\\TEXTURE\\sample.jpg", D3DXIFF_JPG, g_pTexSurface, NULL);
		hr = hr;
	}

	// 深度バッファをウィンドウ深度バッファに変更
	pDevice->SetDepthStencilSurface(g_pBackDepthSurface);

	// レンダーターゲットをバックバッファに変更
	pDevice->SetRenderTarget(0, g_pBackBufferSurface);

	// 画面クリア(バックバッファとZバッファのクリア)
	pDevice->Clear(0, NULL,
		(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER),
		D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	// 描画開始
	if (SUCCEEDED(pDevice->BeginScene()))
	{// 描画開始が成功した場合
		DrawPoly();

		// 描画終了
		g_pD3DDevice->EndScene();
	}

	// バックバッファとフロントバッファの入れ替え
	hr = g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
	EndDevice();

#ifdef _DEBUG
	if (FAILED(hr))
	{
		LPVOID* errorString;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER					// テキストのメモリ割り当てを要求する
			| FORMAT_MESSAGE_FROM_SYSTEM					// エラーメッセージはWindowsが用意しているものを使用
			| FORMAT_MESSAGE_IGNORE_INSERTS,				// 次の引数を無視してエラーコードに対するエラーメッセージを作成する
			NULL,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),		// 言語を指定
			(LPTSTR)&errorString,							// メッセージテキストが保存されるバッファへのポインタ
			0,
			NULL);

		MyMathUtil::GenerateMessageBox(MB_ICONERROR,
			"error",
			(LPCTSTR)errorString);

		LocalFree(errorString);
	}
#endif
}

//================================================================================================================
// --- デバイスの取得 ---
//================================================================================================================
LPDIRECT3DDEVICE9 GetDevice(void)
{
	g_mdDirect3DDevice.LockMultiData();
	return g_pD3DDevice;
}

//================================================================================================================
// --- デバイスの取得終了 ---
//================================================================================================================
void EndDevice(void)
{
	g_mdDirect3DDevice.UnlockMultiData();
}

//================================================================================================================
// --- メインスレッドの状態の取得 ---
//================================================================================================================
bool GetIsMainThread(void)
{
	return g_bMainThread;
}

//================================================================================================================
// -- 獲得済みウィンドウハンドルの取得 ---
//================================================================================================================
HWND GetHandleWindow(void)
{
	if (g_hWnd != NULL)
	{
		return g_hWnd;
	}
	else
	{
		MessageBox(g_hWnd, "ウィンドウハンドルが確保されていません!", "警告！", MB_ICONWARNING);
		return NULL;
	}
}

//===============================================================================================================
// --- ウィンドウフルスクリーン処理 ---
//===============================================================================================================
void ToggleFullscreen(HWND hWnd)
{
	// 現在のウィンドウスタイルを取得
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	if (g_isFullscreen)
	{
		// ウィンドウモードに切り替え
		SetWindowLong(hWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPos(hWnd, HWND_TOP, g_windowRect.left, g_windowRect.top,
			g_windowRect.right - g_windowRect.left, g_windowRect.bottom - g_windowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(hWnd, SW_NORMAL);
	}
	else
	{
		// フルスクリーンモードに切り替え
		GetWindowRect(hWnd, &g_windowRect);
		SetWindowLong(hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
		SetWindowPos(hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(hWnd, SW_MAXIMIZE);
	}

	g_isFullscreen = !g_isFullscreen;
}

CREATE_LOOPID(loopThread)
{

}

void DrawPoly(void)
{
	/*** テクニック等を指定 ***/
	g_pEffect->SetTechnique("MainTec");
	UINT passNum = 0;
	g_pEffect->Begin(&passNum, 0);
	g_pEffect->BeginPass(3);

	/*** テクスチャを設定 ***/
	g_pEffect->SetTexture("TextureFileName", g_pTexSurface);

	/*** 頂点バッファをデータストリームに設定 ***/
	g_pD3DDevice->SetStreamSource(0, g_pVtxBuffSurface, 0, sizeof(VERTEX_2D));

	g_pD3DDevice->SetFVF(FVF_VERTEX_2D);

	/*** ポリゴンの描画 ***/
	g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,	// プリミティブの種類
		0,												// 描画する最初の頂点インデックス
		2);												// 描画するプリミティブの数

	g_pEffect->EndPass();

	g_pEffect->End();
}