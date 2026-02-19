//================================================================================================================
//
// DirectXのスレッド用ヘッダファイル [thread.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _THREAD_H_
#define _THREAD_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define DEFAULT_PROC			&DefThreadProc		// デフォルトスレッドプロシージャ
#define CREATE_LOOPID(loopname)	void loopname(void)	// ループ関数作成マクロ

//**********************************************************************************
//*** スレッド情報構造体定義 ***
//**********************************************************************************
typedef struct ThreadData
{
	unsigned int ThreadId;		// スレッドID
	int nFPS;					// スレッドループ間隔
	int nLoopCount;				// スレッドループ回数

	void (*LoopID)(void);		// ループ時実行関数
	bool bExit;					// スレッドループ終了変数
} ThreadData;

//**********************************************************************************
//*** マルチスレッド構造体定義 ***
//**********************************************************************************
typedef struct Thread
{
private:
	HANDLE hThread;			// スレッドハンドル
	ThreadData* ptData;		// スレッド情報
	bool bSafe;				// 参照可能判定
	bool bDest;				// デストラクタによる自動解放

public:
	// コンストラクタ
	Thread() : hThread(NULL), ptData(NULL), bSafe(false), bDest(true) {}

	// デストラクタ
	~Thread()
	{
		if (hThread != NULL && bDest == true)
		{ // スレッドハンドルが存在する場合、解放
			ptData->bExit = true;

			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			hThread = NULL;
			ptData = NULL;
			bSafe = false;
		}
	}

	// スレッド生成
	bool CreateThread(ThreadData* tData, _beginthreadex_proc_type ThreadProc, bool bOnDest);

	// スレッド破棄
	void Release();

	// スレッドハンドル取得
	bool GetThreadHandle(HANDLE* pThreadHandle);

} Thread;

typedef Thread* LPTHREAD;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
unsigned WINAPI DefThreadProc(LPVOID lpParam);

#endif
