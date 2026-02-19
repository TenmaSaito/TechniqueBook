//================================================================================================================
//
// DirectXのデバッグ表示のcppファイル [debugproc.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "debugproc.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPD3DXFONT g_pFont = NULL;			// フォントへのポインタ
char g_aStrDebug[4096];				// デバッグ表示用文字列格納バッファ

//================================================================================================================
// --- デバッグ表示の初期化 ---
//================================================================================================================
void InitDebugProc(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** フォントの作成 ***/
	D3DXCreateFont(pDevice,
		20,
		0,
		0,
		0,
		FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		"PixelMplus12",
		&g_pFont);

	/*** 文字列格納バッファのクリア ***/
	memset(&g_aStrDebug, NULL, sizeof g_aStrDebug);

	EndDevice();
}

//================================================================================================================
// --- デバッグ表示の終了 ---
//================================================================================================================
void UninitDebugProc(void)
{
	/*** フォントの解放 ***/
	RELEASE(g_pFont);
}

//================================================================================================================
// --- デバッグ表示の更新 ---
//================================================================================================================
void UpdateDebugProc(void)
{
	/*** 文字列格納バッファのクリア ***/
	memset(&g_aStrDebug, NULL, sizeof g_aStrDebug);
}

//================================================================================================================
// --- デバッグ表示の描画 ---
//================================================================================================================
void DrawDebugProc(void)
{
	RECT rect = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };			// 画面サイズ

	if (g_pFont != NULL)
	{
		/*** テキストの描画 ***/
		g_pFont->DrawText(NULL, &g_aStrDebug[0], -1, &rect, DT_LEFT, D3DCOLOR_RGBA(255, 255, 255, 255));
	}
}

//================================================================================================================
// --- サブウィンドウへのデバッグ表示の描画 ---
//================================================================================================================
void DrawDebugProcToSubWindow(RECT rect)
{
	if (g_pFont != NULL)
	{
		/*** テキストの描画 ***/
		g_pFont->DrawText(NULL, &g_aStrDebug[0], -1, &rect, DT_LEFT | DT_NOCLIP, D3DCOLOR_RGBA(255, 255, 255, 255));
	}
}

//================================================================================================================
// --- デバッグ表示の追加 ---
//================================================================================================================
void PrintDebugProc(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const char* p1;
	char aStr[256] = {};
	char aStr2[256] = {};
	char aStrEnd[256] = {};
	char aStrCopy[256] = {};
	int n = 0;
	int nc = 0;
	int d = 0;
	float f = 0.0f;

	int nCnt = 0;

	strcat(aStr2, fmt);

	for (const char* p = fmt; *p != '\0'; p++, n++, nc++)
	{
		if (*p == '%')
		{
			p++;
			n++;

			switch (*p)
			{
			case 'd':

				nc += sprintf(&aStr[0], "%d", va_arg(ap, int));

				/*** 文字列を結合 ***/
				strcat(aStrEnd, &aStr[0]);

				break;

			case 'f':

				f = va_arg(ap, double);
				nc += sprintf(&aStr[0], "%f", f);

				/*** 文字列を結合 ***/
				strcat(aStrEnd, &aStr[0]);

				break;

			case 'c':

				nc += sprintf(&aStr[0], "%c", va_arg(ap, char));

				/*** 文字列を結合 ***/
				strcat(aStrEnd, &aStr[0]);

				break;

			case 's':
				p1 = va_arg(ap, const char*);
				nc += sprintf(&aStr[0], "%s", p1);

				/*** 文字列を結合 ***/
				strcat(aStrEnd, &aStr[0]);

				break;

			case 'b':
				nc += sprintf(&aStr[0], "%d", va_arg(ap, bool));

				/*** 文字列を結合 ***/
				strcat(aStrEnd, &aStr[0]);

				break;

			case '~':

				p++;
				n++;

				sprintf(&aStr[0], "%c", *p);

				nCnt = atoi(&aStr[0]);

				p++;
				n++;

				switch (*p)
				{
				case 'd':

					for (int nCntPrint = 0; nCntPrint < nCnt; nCntPrint++)
					{
						memset(aStr, 0, sizeof aStr);

						nc += sprintf(&aStr[0], "%d ", va_arg(ap, int));

						/*** 文字列を結合 ***/
						strcat(aStrEnd, &aStr[0]);
					}

					break;

				case 'f':

					for (int nCntPrint = 0; nCntPrint < nCnt; nCntPrint++)
					{
						memset(aStr, 0, sizeof aStr);

						f = va_arg(ap, double);
						nc += sprintf(&aStr[0], "%f ", f);

						/*** 文字列を結合 ***/
						strcat(aStrEnd, &aStr[0]);
					}

					break;

				case 'c':

					for (int nCntPrint = 0; nCntPrint < nCnt; nCntPrint++)
					{
						memset(aStr, 0, sizeof aStr);

						nc += sprintf(&aStr[0], "%c ", va_arg(ap, char));

						/*** 文字列を結合 ***/
						strcat(aStrEnd, &aStr[0]);
					}

					break;

				case 's':

					for (int nCntPrint = 0; nCntPrint < nCnt; nCntPrint++)
					{
						memset(aStr, 0, sizeof aStr);

						p1 = va_arg(ap, const char*);
						nc += sprintf(&aStr[0], "%s ", p1);

						/*** 文字列を結合 ***/
						strcat(aStrEnd, &aStr[0]);
					}

					break;

				case 'b':

					for (int nCntPrint = 0; nCntPrint < nCnt; nCntPrint++)
					{
						memset(aStr, 0, sizeof aStr);

						nc += sprintf(&aStr[0], "%d ", va_arg(ap, bool));

						/*** 文字列を結合 ***/
						strcat(aStrEnd, &aStr[0]);
					}

					break;

				default:

					break;
				}

				break;

			default:

				break;
			}
		}
		else
		{
			/*** 文字列を結合 ***/
			sprintf(&aStr[0], "%c", *p);
			//strncpy(&aStrCopy[0], &aStr2[n], 1);

			strcat(&aStrEnd[0], &aStr[0]);
		}

		memset(aStr, 0, sizeof aStr);
	}

	/*** クリーンアップ ***/
	va_end(ap);

	aStrEnd[nc + 1] = '\0';

	/*** 文字列を結合 ***/
	strcat(&g_aStrDebug[0], aStrEnd);
}