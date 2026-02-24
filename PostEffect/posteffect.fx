//================================================================================================================
//
// DirectXの基本エフェクトファイル [shader.fx]
// Author : TENMA
//
//================================================================================================================
//===================================================================
//
// パラメータ宣言
//
//===================================================================

// マクロ定義
#define MAX_LIGHT	(3)						// 照明の最大数

// 変数

struct LIGHT_INFO
{
	float4 Direction;						// ライトの方向
	float4 Color;							// ライトの色（輝度）
};

LIGHT_INFO g_aLight[MAX_LIGHT];

bool bUseMaterialColor;						// マテリアルカラーを使用するか

texture TextureFileName : TEXTUREFILENAME;	// テクスチャポインタ
sampler TexSampler = sampler_state			// テクスチャサンプラー
{
	texture = <TextureFileName>;
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

bool bUseTexture;							// テクスチャ使用の有無

// 座標変換行列
float4x4 WorldViewProj : WORLDVIEWPROJECTION;
float4x4 World : WORLD;

//===================================================================
//
// 頂点シェーダー関連
//
//===================================================================

// 入出力構造体
struct VS_INPUT
{
	float4 Position : POSITION0;	// 位置
	float4 Color : COLOR0;			// 色
	float3 Normal : NORMAL;			// 法線
	float2 Tex : TEXCOORD0;			// テクスチャ座標
	float4 mtxWorld1 : TEXCOORD1;	// ワールドマトリックス1行目
	float4 mtxWorld2 : TEXCOORD2;	// ワールドマトリックス2行目
	float4 mtxWorld3 : TEXCOORD3;	// ワールドマトリックス3行目
	float4 mtxWorld4 : TEXCOORD4;	// ワールドマトリックス4行目
};

// 入出力構造体
struct VS_INPUT2D
{
	float4 Position : POSITION0;	// 位置
	float4 Color : COLOR0;			// 色
	float2 Tex : TEXCOORD0;			// テクスチャ座標
};

struct VS_OUTPUT
{
	float4 Position : POSITION;		// 位置
	float4 VertexColor : COLOR;		// 頂点カラー
	float2 Tex : TEXCOORD0;			// テクスチャ座標
	float3 Normal : TEXCOORD1;		// ワールド空間法線
};

struct VS_OUTPUT2D
{
	float4 Position : POSITION;		// 位置
	float4 VertexColor : COLOR;		// 頂点カラー
	float2 Tex : TEXCOORD0;			// テクスチャ座標
};

// 頂点シェーダー
VS_OUTPUT Basic_VS(VS_INPUT In)
{
	VS_OUTPUT Out;
	float4 ScaleIn = In.Position;
	float4x4 mtxWorld = float4x4(In.mtxWorld1,
		In.mtxWorld2,
		In.mtxWorld3,
		In.mtxWorld4);

	// 射影変換
	float4 worldPos = mul(In.Position, mtxWorld);
	Out.Position = mul(worldPos, WorldViewProj);

	// テクスチャ座標を保存
	Out.Tex = In.Tex;

	// 法線をワールド空間へ変換
	float3x3 World3x3 = (float3x3)mtxWorld;
	Out.Normal = normalize(mul(In.Normal, World3x3));

	Out.VertexColor = In.Color;

	return Out;
}

VS_OUTPUT2D Basic2D_VS(VS_INPUT2D In)
{
	VS_OUTPUT2D Out;

	Out.Position = In.Position;

	// テクスチャ座標を保存
	Out.Tex = In.Tex;

	Out.VertexColor = In.Color;

	return Out;
}

// 描画確認用頂点シェーダー
VS_OUTPUT Test_VS(VS_INPUT In)
{
	VS_OUTPUT Out;
	Out.Position = float4(0, 0, 0.5, 1);
	Out.VertexColor = float4(1, 0, 0, 1);
	Out.Tex = float2(0, 0);
	Out.Normal = float3(0, 0, 1);
	return Out;
}

//===================================================================
//
// ピクセルシェーダー関連
//
//===================================================================

// ピクセルシェーダー
float4 Basic_PS(VS_OUTPUT In) : COLOR0
{
	// 法線の正規化
	float3 N = normalize(In.Normal);

	// 拡散反射光の合計を保持する変数
	float3 totalDiffuse = float3(0, 0, 0);

	// 各ライトの計算をループで回す
	for (int nCntLight = 0; nCntLight < MAX_LIGHT; nCntLight++)
	{
		// 光の方向を正規化(向きを逆転後、正規化した法線ベクトルを求める -> 光に当たった面が明るくなる)
		// ライト方向の逆ベクトル（光が来る方向）
		float3 L = normalize(-g_aLight[nCntLight].Direction.xyz);

		// Lambert（ランバート）反射モデル: N・L
		// 0以下にならないよう saturate（0?1にクランプ）する
		float diffuseIntensity = saturate(dot(N, L));

		// このライトの寄与分を加算
		totalDiffuse += diffuseIntensity * g_aLight[nCntLight].Color.rgb;
	}

	// 最終色
	float4 finalColor;

	// 「光の色」と「頂点カラー」を乗算する
	finalColor.rgb = totalDiffuse * In.VertexColor.rgb;

	// テクスチャ使用
	finalColor *= tex2D(TexSampler, In.Tex);

	// a値はそのまま適用
	finalColor.a = In.VertexColor.a;

	return finalColor;
}

// テクスチャ未使用ピクセルシェーダー
float4 UnusedTex_PS(VS_OUTPUT In) : COLOR0
{
	// 法線の正規化
	float3 N = normalize(In.Normal);

	// 拡散反射光の合計を保持する変数
	float3 totalDiffuse = float3(0, 0, 0);

	// 各ライトの計算をループで回す
	for (int nCntLight = 0; nCntLight < MAX_LIGHT; nCntLight++)
	{
		// 光の方向を正規化(向きを逆転後、正規化した法線ベクトルを求める -> 光に当たった面が明るくなる)
		// ライト方向の逆ベクトル（光が来る方向）
		float3 L = normalize(-g_aLight[nCntLight].Direction.xyz);

		// Lambert（ランバート）反射モデル: N・L
		// 0以下にならないよう saturate（0?1にクランプ）する
		float diffuseIntensity = saturate(dot(N, L));

		// このライトの寄与分を加算
		totalDiffuse += diffuseIntensity * g_aLight[nCntLight].Color.rgb;
	}

	// 最終色
	float4 finalColor;

	// 「光の色」と「頂点カラー」を乗算する
	finalColor.rgb = totalDiffuse * In.VertexColor.rgb;
	finalColor.a = In.VertexColor.a;

	return finalColor;
}

// ライト未使用ピクセルシェーダー
float4 UnusedLight_PS(VS_OUTPUT In) : COLOR0
{
	// 最終色
	float4 finalColor;

	// 
	float3 totalDiffuse = float3(1, 1, 1);

	// 「光の色」と「頂点カラー」を乗算する
	finalColor.rgb = totalDiffuse * In.VertexColor.rgb;
	
	if (bUseTexture)
	{	 // テクスチャ使用時、適用
		finalColor *= tex2D(TexSampler, In.Tex);
	}

	// a値はそのまま適用
	finalColor.a = In.VertexColor.a;

	return finalColor;
}

// 描画確認用ピクセルシェーダー
float4 Test_PS(VS_OUTPUT In) : COLOR0
{
	return float4(1,0,0,1);
}

float4 ps_main(VS_OUTPUT2D In) : COLOR0
{
	float MosaicSize = 0.001;
	float2 ScreenSize = float2(1280.0, 720.0);

	// 1. テクスチャ座標をピクセル単位に変換
	float2 pixelUV = In.Tex * ScreenSize;

	// 2. モザイクサイズに基づいてUVを切り捨てて「ブロック」化する
	float2 mosaicUV = floor(pixelUV / MosaicSize) * MosaicSize;

	// 3. ブロックの中央のテクスチャを参照する（少しずらすと綺麗になる）
	mosaicUV += MosaicSize * 0.5;

	// 4. 正規化されたUVに戻す
	float2 finalUV = mosaicUV / ScreenSize;

	float4 Out = tex2D(TexSampler, finalUV);
	//Out.w = 1.0f;

	// 5. テクスチャカラーを取得
	return Out;
}

//===================================================================
//
// テクニック・パス関連
//
//===================================================================

// テクニック
technique MainTec
{
	// パス
	pass ResultColor
	{
		// 頂点シェーダーをコンパイル
		VertexShader = compile vs_3_0 Basic_VS();

		// ピクセルシェーダーをコンパイル
		PixelShader = compile ps_3_0 Basic_PS();
	}

	pass UnusedLight
	{
		// 頂点シェーダーをコンパイル
		VertexShader = compile vs_3_0 Basic_VS();

		// ピクセルシェーダーをコンパイル
		PixelShader = compile ps_3_0 UnusedLight_PS();
	}

	pass UnusedTex
	{
		// 頂点シェーダーをコンパイル
		VertexShader = compile vs_3_0 Basic_VS();

		// ピクセルシェーダーをコンパイル
		PixelShader = compile ps_3_0 UnusedTex_PS();
	}

	pass PostEffect
	{
		// 頂点シェーダーをコンパイル
		VertexShader = compile vs_3_0 Basic2D_VS();

		// ピクセルシェーダーをコンパイル
		PixelShader = compile ps_3_0 ps_main();
	}

	pass Test
	{
		// 頂点シェーダーをコンパイル
		VertexShader = compile vs_3_0 Test_VS();

		// ピクセルシェーダーをコンパイル
		PixelShader = compile ps_3_0 Test_PS();
	}
}