//================================================================================================================
//
// DirectXのスレッド生成のcppファイル [thread.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "thread.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************

//=================================================================================================
// --- スレッド作成 ---
//=================================================================================================
_Check_return_ bool Thread::CreateThread(_In_ ThreadData* tData, _In_ _beginthreadex_proc_type ThreadProc, _In_ bool bOnDest, _Outptr_opt_ Thread** lpThread)
{
	// スレッド生成済みの場合、生成失敗
	if (hThread != NULL) return false;

	// スレッド生成情報へのポインタが無い場合、生成失敗
	if (tData == NULL) return false;

	// スレッド生成
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, tData, 0, &tData->ThreadId);
	if (hThread == NULL) return false;		// 生成失敗
	ptData = tData;			// スレッド生成情報へのポインタを保存
	bSafe = true;			// ハンドル取得を可能にする
	bDest = bOnDest;		// デストラクタの実行有無

	if (lpThread != NULL)
	{ // スレッドへのダブルポインタがNULLでは無ければ
		*lpThread = this;	// スレッドポインタへこのスレッドのポインタを格納
	}

	return true;
}

//=================================================================================================
// --- スレッド破棄 ---
//=================================================================================================
void Thread::Release()
{
	if (ptData != NULL && hThread != NULL)
	{ // スレッドハンドルが存在する場合、解放
		ptData->bExit = true;
		WaitForSingleObject(hThread, INFINITE);

		CloseHandle(hThread);
		hThread = NULL;
		ptData = NULL;
		bSafe = false;
		bDest = false;
	}
}

//=================================================================================================
// --- スレッドの強制破棄 ---
//=================================================================================================
bool Thread::ForceRelease(DWORD dwExitCode)
{
	if (hThread != NULL)
	{ // スレッドハンドルが存在する場合、スレッドを強制停止
		if (TerminateThread(hThread, dwExitCode))
		{ // 停止成功時、ハンドルを解放。
			CloseHandle(hThread);
			hThread = NULL;
			ptData = NULL;
			bSafe = false;
			bDest = false;

			return true;
		}
	}

	// 失敗
	return false;
}

//=================================================================================================
// --- スレッドハンドル取得 ---
//=================================================================================================
bool Thread::GetThreadHandle(HANDLE* pThreadHandle)
{
	if (bSafe)
	{ // スレッド生成が成功している且つ、スレッドが存在する場合
		if (pThreadHandle != NULL)
		{ // スレッド保存ポインタが存在する場合
			*pThreadHandle = hThread;		// スレッドハンドルを保存
			return true;
		}
	}

	return false;
}

//=================================================================================================
// --- スレッド確認 ---
//=================================================================================================
bool Thread::IsThread(void)
{
	DWORD dwExitCode;

	if (hThread != NULL)
	{ // スレッドハンドルが存在する場合
		if (GetExitCodeThread(hThread, &dwExitCode))
		{ // スレッドの終了コード獲得成功時
			// 終了コードがSTILL_ACTIVEなら
			if (dwExitCode == STILL_ACTIVE) return true;
		}
	}

	// スレッド未作成,または取得失敗
	return false;
}

//=================================================================================================
// --- スレッド停止 ---
//=================================================================================================
void Thread::StopThread(void)
{
	// スレッドを停止
	ptData->bPlay = false;

	// スレッド停止時間を設定
	ptData->nSleepTime = 0;
}

//=================================================================================================
// --- スレッド休止 ---
//=================================================================================================
void Thread::SleepThread(int nSleepTime)
{
	// スレッドを再生
	ptData->bPlay = true;

	// スレッド停止時間を設定
	ptData->nSleepTime = nSleepTime;
}

//=================================================================================================
// --- スレッド再生 ---
//=================================================================================================
void Thread::PlayThread(void)
{
	// スレッドを再生
	ptData->bPlay = true;

	// スレッド停止時間を設定
	ptData->nSleepTime = 0;
}

//=================================================================================================
// --- デフォルトスレッドプロシージャ ---
//=================================================================================================
unsigned WINAPI DefThreadProc(LPVOID lpParam)
{
	unsigned int nFPS = 60;
	ThreadData* tData = NULL;
	if (lpParam != NULL)
	{ // NULLチェック
		tData = (ThreadData*)lpParam;
		nFPS = tData->nFPS;
	}
	else
	{
		return 1;
	}

	MSG msg = {};
	DWORD dwCurrentTime = 0;	// 現在時刻
	DWORD dwExecLastTime = 0;	// 最後に処理した時刻
	DWORD dwCurrentTimeSleep = 0;		// スリーブカウント
	DWORD dwExecLastTimeSleep = 0;		// 最後にスリーブカウントを計測した時刻
	int nLoopCount = 0;			// ループ回数

	dwCurrentTime = 0;					// 初期化
	dwExecLastTime = timeGetTime();		// 現在時刻を取得

	// メインスレッドが実行されている間、ループ
	while (1)
	{
		// 再生中でなければ処理をしない
		if (tData->bPlay != true) continue;

		// スリープ中なら処理をしない
		if (tData->nSleepTime <= 0)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) != 0)
			{// windowsの処理

			}
			else
			{
				dwCurrentTime = timeGetTime();				// 現在時刻を取得

				// ループ間隔
				if ((dwCurrentTime - dwExecLastTime) >= (1000 / nFPS))
				{ //60分の1秒経過
					dwExecLastTime = dwCurrentTime;			// 処理開始時刻[現在時刻]を保存

					if (tData->LoopID != NULL)
					{ // NULLチェック
						tData->LoopID();						// スレッドループ中の処理
					}

					nLoopCount++;
				}
			}

			// ループ回数が規定数を超えたら終了
			if (nLoopCount >= tData->nLoopCount && tData->nLoopCount != -1)
			{
				break;
			}

			// 最終スリーブカウント時刻をリセット
			if (dwExecLastTimeSleep != 0) dwExecLastTimeSleep = 0;
		}
		else
		{
			if (dwExecLastTimeSleep == 0) dwExecLastTimeSleep = dwCurrentTime;

			dwCurrentTimeSleep = timeGetTime();				// 現在時刻を取得

			tData->nSleepTime -= dwExecLastTimeSleep - dwCurrentTimeSleep;

			dwExecLastTimeSleep = dwCurrentTimeSleep;		// 処理開始時刻[現在時刻]を保存
		}

		// スレッドループ終了指示が来たら終了
		if (tData->bExit == true)
		{
			break;
		}

		// メインスレッド終了時、スレッド解放
		if (GetIsMainThread() == false)
		{
			break;
		}
	}

	// スレッドの終了コードを設定(GetExitCodeThread(threadハンドル, int*)で取得可能)
	_endthreadex(0);

	return 0;
}