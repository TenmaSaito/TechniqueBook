//==================================================================================
//
// DirectXの計算関連ヘッダファイル [mathUtil.h]
// Author : TENMA
//
//==================================================================================
#ifndef _MATHUTIL_H_
#define _MATHUTIL_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "param.h"

using namespace Constants;

// modeldata.hが存在する場合
#if __has_include("modeldata.h")
#include "modeldata.h"
#define MODELDATA_INCLUDED
#else
SetWarning("modeldata.hがインクルードされていません。modeldata関連関数は無効化されます。")
#endif

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define RECT_NULL				(RECT{0, 0, 0, 0})					// RECT初期化
#define INT_VECTOR3_NULL		(MyMathUtil::INT_VECTOR3{ 0, 0, 0 })	// INT_VECTOR3初期化
#define INT_VECTOR2_NULL		(MyMathUtil::INT_VECTOR2{ 0, 0 })		// INT_VECTOR2初期化
#define D3DCOLOR_NULL			(D3DCOLOR_RGBA(255, 255, 255, 255)) // D3DCOLOR_NULL
#define INT_INFINITY			(0x7FFFFFFF)						// int型の最大(signed intの最大値)
#define VECNULL					D3DXVECTOR3(0.0f,0.0f,0.0f)			// 省略版vectorNull
#define VEC_X(x)				D3DXVECTOR3(x, 0.0f, 0.0f)			// Xのみ変更
#define VEC_Y(y)				D3DXVECTOR3(0.0f, y, 0.0f)			// Yのみ変更
#define VEC_Z(z)				D3DXVECTOR3(0.0f, 0.0f, z)			// Zのみ変更
#define InitRot(x, y, z)		RepairedRot(D3DXVECTOR3(x, y, z))	// 修正済み角度
#define DEF_COL					D3DXCOLOR_NULL						// デフォルトカラー
#define foreach(type, var, lpArray)		for(type &var : lpArray)	// foreach構文
#ifndef STRUCT
#define STRUCT(...)				typedef struct __VA_ARGS__			// STRUCT宣言
#endif
#ifndef ENUM
#define ENUM(...)				typedef enum __VA_ARGS__			// ENUM宣言
#endif
#ifndef PARENT
#define PARENT(...)				: public __VA_ARGS__				// 継承宣言
#endif

//----------------------------------------------------------------------------------
/*** フラグ関連 ***/
#define END_SHADER				(-1)						// シェーダーの終了コード
//----------------------------------------------------------------------------------

namespace MyMathUtil
{
	//******************************************************************************
	//*** int型Vector3構造体 ***
	//******************************************************************************
	STRUCT()
	{
		int x;		// x
		int y;		// y
		int z;		// z
	}INT_VECTOR3;

	//******************************************************************************
	//*** int型Vector2構造体 ***
	//******************************************************************************
	STRUCT()
	{
		int x;		// x
		int y;		// y
	}INT_VECTOR2;

	//******************************************************************************
	//*** DualInputのフラグ列挙 ***
	//******************************************************************************
	ENUM()
	{
		DUAL_KEYBOARD	= 0x00000001,			// Keyboard入力
		DUAL_JOYPAD		= 0x00000002,			// ジョイパッド入力
		DUAL_OR			= 0x00000010,			// 片方
		DUAL_AND		= 0x00000020,			// 両方
		DUAL_PRESS		= 0x00000100,			// プレス
		DUAL_TRIGGER	= 0x00000200,			// トリガー
		DUAL_RELEASE	= 0x00000400,			// リリース
		DUAL_REPEAT		= 0x00000800,			// リピート
		DUAL_DUAL		= 0x00001000,			// コントローラー2番で判定

		DUAL_FORCE_DWORD = 0x7fffffff
	} DUAL;

#define DUAL_DUALID		3						// DUALをずらす距離

	//******************************************************************************
	//*** プロトタイプ宣言 ***
	//******************************************************************************
	
	//------------------------------------------------------------------------------
	/*** Input関連 ***/
	bool GetDualInput(int nKey, DWORD nFlag1, int nKey2 = -1, DWORD nFlag2 = -1);
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** 当たり判定、角度関連 ***/
	bool CollisionBox(RECT rect, D3DXVECTOR3 pos);
	bool CollisionBoxZ(D3DXVECTOR4 rect, D3DXVECTOR3 pos);
	bool IsDetection(D3DXVECTOR3 p1, D3DXVECTOR3 p2, float fRadius);
	void RepairFloat(float* fRepairTarget, int nCnt = 3);
	float RepairRot(float fRot);
	void RepairRot(float* pOut, const float* fAngle);
	void RepairRot(D3DXVECTOR3* pOut, const D3DXVECTOR3* pIn);
	float InverseRot(float fRot);
	D3DXVECTOR3 InverseRot(D3DXVECTOR3 fRot);
	D3DXVECTOR3 RepairedRot(const D3DXVECTOR3 pIn);
	D3DXVECTOR3 DegreeToRadian(D3DXVECTOR3 degree);
	D3DXVECTOR3 RadianToDegree(D3DXVECTOR3 radian);
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** D3DXVECTOR3の位置関連 ***/
	D3DXVECTOR3 GetPTPLerp(D3DXVECTOR3 Start, D3DXVECTOR3 End, float s);
	void HomingPosToPos(D3DXVECTOR3 posTarget, D3DXVECTOR3* posMover, float fSpeed);
	float GetPosToPos(D3DXVECTOR3 posTarget, D3DXVECTOR3 posMover);
	D3DXVECTOR3 GetPosBetweenPos(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
	float GetPTPLength(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
	float GetPTPLength3D(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** D3DXCOLOR関連 ***/
	D3DXCOLOR GetColLerp(D3DXCOLOR Start, D3DXCOLOR End, float s);
	//------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------
	/*** ランダムな値取得関連 ***/
	D3DXVECTOR3 GetRandomVector3(int mx, int my, int mz);
	D3DXCOLOR GetRandomColor(bool bUseAlphaRand);
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** 頂点情報関連 ***/
	void RollPolygon(VERTEX_2D* pVtx, D3DXVECTOR3 pos, float fWidth, float fHeight, float fRot, int nSpeed);
	void SetFullScreenPolygon(VERTEX_2D* pVtx);
	void SetPolygonSize(VERTEX_3D* pVtx, D3DXVECTOR2 size, bool bY);
	void SetPolygonPos(VERTEX_2D* pVtx, D3DXVECTOR3 pos, D3DXVECTOR2 size);

	// テクスチャ設定
	template<typename VERTEX>
	void SetDefaultTexture(VERTEX* pVtx)
	{
		if (pVtx == nullptr) return;

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);
	}

	// 色設定
	template<typename VERTEX>
	void SetDefaultColor(VERTEX* pVtx, D3DXCOLOR col = D3DXCOLOR_NULL)
	{
		if (pVtx == nullptr) return;

		pVtx[0].col = col;
		pVtx[1].col = col;
		pVtx[2].col = col;
		pVtx[3].col = col;
	}

	void SetPolygonNormal(VERTEX_3D* pVtx, D3DXVECTOR3 nor);
	void SetPolygonRHW(VERTEX_2D* pVtx);
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** 変換関連 ***/
	D3DXVECTOR3 INTToFloat(INT_VECTOR3 nVector3);
	D3DXVECTOR2 INTToFloat(INT_VECTOR2 nVector2);
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** 描画関連 ***/
	void DrawPolygon(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ LPDIRECT3DVERTEXBUFFER9 pVtxBuff,
		_In_opt_ LPDIRECT3DTEXTURE9 pTexture,
		_In_ UINT VertexFormatSize,
		_In_ DWORD FVF,
		_In_ int nNumPolygon,
		_In_opt_ UINT OffSet = NULL);

	void DrawPolygonTextureArray(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ LPDIRECT3DVERTEXBUFFER9 pVtxBuff,
		_In_ LPDIRECT3DTEXTURE9 *pTexture,
		_In_ UINT nNumTexture,
		_In_ const int *pArrayTexNo,
		_In_ UINT VertexFormatSize,
		_In_ DWORD FVF,
		_In_ int nNumPolygon,
		_In_opt_ UINT OffSet = NULL);

	void DrawPolygonTextureFromIndex(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ LPDIRECT3DVERTEXBUFFER9 pVtxBuff,
		_In_ int nIdxTexture,
		_In_ UINT VertexFormatSize,
		_In_ DWORD FVF,
		_In_ int nNumPolygon,
		_In_opt_ UINT OffSet = NULL);

#pragma push_macro("NULL")
#undef NULL
#define NULL nullptr

	void Draw3DModelFromXFile(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ const D3DXMATERIAL *pMat,
		_In_ DWORD dwNumMat,
		_In_ LPDIRECT3DTEXTURE9 *ppTexture,
		_In_ LPD3DXMESH pMesh,
		_In_ const D3DXMATRIX *pMtxWorld,
		_In_opt_ const D3DXMATRIX *pMtxShadow = NULL);

	void Draw3DModelByCustomColorFromXFile(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ const D3DXMATERIAL* pMat,
		_In_ DWORD dwNumMat,
		_In_ LPDIRECT3DTEXTURE9* ppTexture,
		_In_ LPD3DXMESH pMesh,
		_In_ const D3DXMATRIX* pMtxWorld,
		_In_ D3DCOLORVALUE CustomColor);

#ifdef MODELDATA_INCLUDED
	void Draw3DModelFromModelData(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ const MODELDATA *pModelData,
		_In_ const D3DXMATRIX *pMtxWorld,
		_In_opt_ const D3DXMATRIX *pMtxShadow = NULL);

	void Draw3DModelByCustomColorFromModelData(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ const MODELDATA* pModelData,
		_In_ const D3DXMATRIX* pMtxWorld,
		_In_ D3DCOLORVALUE CustomColor);
#endif
#pragma pop_macro("NULL")

	D3DXMATRIX *CalcWorldMatrix(_Inout_ D3DXMATRIX *pMtxWorld,
		_In_ D3DXVECTOR3 pos,
		_In_ D3DXVECTOR3 rot);

	D3DXMATRIX *CalcWorldMatrixFromParent(_Inout_ D3DXMATRIX *pMtxWorld,
		_In_ const D3DXMATRIX *pMtxParent,
		_In_ D3DXVECTOR3 pos,
		_In_ D3DXVECTOR3 rot);

	D3DXMATRIX *CreateShadowMatrix(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ const D3DXMATRIX *pMtxWorld,
		_In_ D3DXVECTOR3 pos,
		_In_ D3DXVECTOR3 nor,
		_In_ UINT nIdxLight,
		_Out_ D3DXMATRIX *pOut);

	void SetEnableZFunction(_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ bool bEnable);

	void SetUpPixelFog(_In_ D3DXCOLOR Col,
		_In_ float fStart,
		_In_ float fEnd);

	void CleanUpPixelFog(void);
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** シェーダー関連 ***/
	_Check_return_ HRESULT LoadSheder(
		_In_ LPDIRECT3DDEVICE9 pDevice,
		_In_ SHADER_PATH pathName,
		_Out_ LPD3DXEFFECT* ppEffect
	);

	void SetSheder(
		_In_ LPD3DXEFFECT pEffect,
		_In_ const char *TechniqueName,
		_In_ UINT Pass
	);

	void RemovePass(
		_In_ LPD3DXEFFECT pEffect,
		_In_ UINT NextPass = END_SHADER
	);
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** 単純計算関連 ***/
	// --- int --- //
	// 最大値
	__forceinline int Max(int x, int y)
	{
		return (x >= y) ? x : y;
	}

	// 最小値
	__forceinline int Min(int x, int y)
	{
		return (x >= y) ? y : x;
	}

	// クランプ
	__forceinline int Clamp(int x, int clampMin, int clampMax)
	{
		return (x > clampMax) ? clampMax 
			: ((x < clampMin) ? clampMin 
				: x);
	}

	// 二乗
	__forceinline int Square(int x)
	{
		return x * x;
	}

	// 線形補間
	__forceinline int Lerp(int start, int end, float s)
	{
		return start + ((end - start) * s);
	}

	// 上下判定
	__forceinline int Step(int y, int x)
	{
		return (x >= y) ? 1 : 0;
	}

	// 絶対値変換済み上下判定
	__forceinline int StepAbs(int y, int x)
	{
		return (x >= abs(y)) ? 1.0f : 0.0f;
	}

	// 符号を返す
	__forceinline int Sign(int x)
	{
		return (x > 0) ? 1
			: ((x < 0) ? -1
				: 0);
	}

	// --- float --- //
	// 最大値
	__forceinline float Max(float x, float y)
	{
		return (x >= y) ? x : y;
	}

	// 最小値
	__forceinline float Min(float x, float y)
	{
		return (x >= y) ? y : x;
	}

	// クランプ
	__forceinline float Clamp(float x, float clampMin, float clampMax)
	{
		return (x > clampMax) ? clampMax
			: ((x < clampMin) ? clampMin
				: x);
	}

	// 二乗
	__forceinline float Square(float x)
	{
		return x * x;
	}

	// 線形補間
	__forceinline float Lerp(float start, float end, float s)
	{
		return start + ((end - start) * s);
	}

	// 上下判定
	__forceinline float Step(float y, float x)
	{
		return (x >= y) ? 1.0f : 0.0f;
	}

	// 絶対値変換済み上下判定
	__forceinline float StepAbs(float y, float x)
	{
		return (x >= fabsf(y)) ? 1.0f : 0.0f;
	}

	// 符号を返す
	__forceinline float Sign(float x)
	{
		return (x > 0.0f) ? 1.0f 
			: ((x < 0.0f) ? -1.0f 
				: 0.0f);
	}

	// --- VECTOR3 --- //
	// 円の弧の座標
	__forceinline D3DXVECTOR3 Arc(float radius, float radian, D3DXVECTOR3 offset = CParamVector::V3NULL)
	{
		return D3DXVECTOR3(offset.x + cosf(radian) * radius,
			offset.y + sinf(radian) * radius,
			offset.z + 0.0f);
	}

	// --- MATRIX --- //
	// 掛け算
	__forceinline D3DXMATRIX Multiply(D3DXMATRIX mtx1, D3DXMATRIX mtx2)
	{
		return mtx1 * mtx2;
	}
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	/*** システム関連 ***/
	HRESULT CheckIndex(int TargetIndexMax, int Index, int TargetIndexMin = NULL);
	bool CheckPath(_In_ const char* pFileName);
	char *UniteChar(char* pOut, const char* fmt, ...);
	int GenerateMessageBox(_In_ UINT nType, _In_ const char* pCaption, _In_ const char* fmt, ...);
	char *GetErrorMessage(_In_ HRESULT hr, char *pOut, size_t size, bool bPopupMessageBox);
	__forceinline void SetRand(void)
	{
		srand((unsigned int)time(0));
	}
	//------------------------------------------------------------------------------
}

#endif
