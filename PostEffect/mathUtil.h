//================================================================================================================
//
// DirectXの計算関連ヘッダファイル [mathUtil.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _MATHUTIL_H_
#define _MATHUTIL_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define RECT_NULL				(RECT{0, 0, 0, 0})				// RECT初期化
#define INT_VECTOR3_NULL		(INT_VECTOR3{ 0, 0, 0 })		// INT_VECTOR3初期化
#define INT_VECTOR2_NULL		(INT_VECTOR2{ 0, 0 })			// INT_VECTOR2初期化
#define D3DCOLOR_NULL			(D3DCOLOR_RGBA(255, 255, 255, 255)) // D3DCOLOR_NULL
#define INT_INFINITY			(0x7FFFFFFF)					// int_infinity
#define VECNULL					D3DXVECTOR3(0.0f,0.0f,0.0f)		// 省略版vectorNull
#define VEC_X(x)				D3DXVECTOR3(x, 0.0f, 0.0f)		// Xのみ変更
#define VEC_Y(y)				D3DXVECTOR3(0.0f, y, 0.0f)		// Yのみ変更
#define VEC_Z(z)				D3DXVECTOR3(0.0f, 0.0f, z)		// Zのみ変更
#define InitedVec3(name)		D3DXVECTOR3 name = {}			// 初期化済みVector3
#define OptVec3(name, x, y, z)	D3DXVECTOR3 name = D3DXVECTOR3(x, y, z)	// 初期化混合変数宣言
#define InitRot(x, y, z)		RepairedRot(D3DXVECTOR3(x, y, z))	// 修正済み角度
#define DEF_COL					D3DXCOLOR_NULL					// デフォルトカラー
#define COL_R(r)				D3DXCOLOR(r, 1.0f, 1.0f, 1.0f)	// R値変更
#define COL_A(a)				D3DXCOLOR(1.0f, 1.0f, 1.0f, a)	// A値変更

namespace MyMathUtil
{
	//**********************************************************************************
	//*** カリングタイプ ***
	//**********************************************************************************
	typedef enum
	{
		CULLINGTYPE_NONE = 0,		// カリング無し
		CULLINGTYPE_FRONT,			// 表面カリング
		CULLINGTYPE_BACK,			// 背面カリング
		CULLINGTYPE_MAX
	}CULLINGTYPE;

	//**********************************************************************************
	//*** int型Vector3構造体 ***
	//**********************************************************************************
	typedef struct
	{
		int x;		// x
		int y;		// y
		int z;		// z
	}INT_VECTOR3;

	//**********************************************************************************
	//*** int型Vector2構造体 ***
	//**********************************************************************************
	typedef struct
	{
		int x;		// x
		int y;		// y
	}INT_VECTOR2;

	//**********************************************************************************
	//*** プロトタイプ宣言 ***
	//**********************************************************************************
	bool CollisionBox(RECT rect, D3DXVECTOR3 pos);
	bool CollisionBoxZ(D3DXVECTOR4 rect, D3DXVECTOR3 pos);
	HRESULT CheckIndex(int TargetIndexMax, int Index, int TargetIndexMin = 0);
	POINT_RECT SetPointRect(D3DXVECTOR3 p1, D3DXVECTOR3 p2, D3DXVECTOR3 p3, D3DXVECTOR3 p4);
	POINT_RECT SetRectToPointRect(RECT rect);
	void HomingPosToPos(D3DXVECTOR3 posTarget, D3DXVECTOR3* posMover, float fSpeed);
	float GetPosToPos(D3DXVECTOR3 posTarget, D3DXVECTOR3 posMover);
	float RepairRot(float fRot);
	void RepairRot(float* pOut, const float* fAngle);
	void RepairRot(D3DXVECTOR3* pOut, const D3DXVECTOR3* pIn);
	float InverseRot(float fRot);
	D3DXVECTOR3 InverseRot(D3DXVECTOR3 fRot);
	D3DXVECTOR3 RepairedRot(const D3DXVECTOR3 pIn);
	D3DXVECTOR3 GetPosBetweenPos(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
	D3DXCOLOR GetRandomColor(bool bUseAlphaRand);
	float GetPTPLength(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
	void RollPolygon(VERTEX_2D* pVtx, D3DXVECTOR3 pos, float fWidth, float fHeight, float fRot, int nSpeed);
	void SetVtxTexture(VERTEX_2D* pVtx, float x, float y);
	void SetVtxTexture(VERTEX_3D* pVtx, float x, float y);
	void SetFullScreenPolygon(VERTEX_2D* pVtx);
	D3DXVECTOR3 DegreeToRadian(D3DXVECTOR3 degree);
	D3DXVECTOR3 RadianToDegree(D3DXVECTOR3 radian);
	D3DXVECTOR3 INTToFloat(INT_VECTOR3 nVector3);
	D3DXVECTOR2 INTToFloat(INT_VECTOR2 nVector2);
	D3DXVECTOR3 GetRandomVector3(int mx, int my, int mz);
	void SetIndexedVertex(VERTEX_3D* pVtx, D3DXVECTOR3 pos, int nXBlock, int nZBlock, float fWidth, float fDepth);
	int SetIndex(_Inout_ WORD* pIdx, _In_ int nXBlock, _In_ int nZBlock, _In_ int nOffSet = 0, _Inout_opt_ WORD* pIdxSub = NULL);
	void RepairFloat(float* fRepairTarget, int nCnt = 3);
	bool CheckPath(_In_ const char* pFileName);
	char* UniteChar(char* pOut, const char* fmt, ...);
	int GenerateMessageBox(_In_ UINT nType, _In_ const char* pCaption, _In_ const char* fmt, ...);
	void SetCullingType(D3DCULL type);
}
#endif
