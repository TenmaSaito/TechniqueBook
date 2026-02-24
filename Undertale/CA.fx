//=========================================================
// 
// 色収差 + グリッチのシェーダー [CA.fx]
// Author : TENMA SAITO
// 
//=========================================================

//---------------------------------------------------------
// --- マクロ定義 ---
//---------------------------------------------------------
#define TECHNIQUE_NAME		CA

//---------------------------------------------------------
// --- 構造体・グローバル変数宣言 ---
//---------------------------------------------------------
float g_fUpperGlitch = 0.5f;			// グリッチの上部座標
float g_fLowerGlitch = 0.5f;			// グリッチの下部座標
float2 distance = float2(0.0, 0.0);		// 色収差の幅

// テクスチャサンプラー
texture Texture;
sampler TexSampler = sampler_state
{
	texture = <Texture>;
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

// 入力構造体
struct VS_INPUT
{
	float4 Position : POSITION0;	// 位置
	float4 Color : COLOR0;			// 色
	float2 Tex : TEXCOORD0;			// テクスチャ座標
};

// 出力構造体
struct VS_OUTPUT
{
	float4 Position : POSITION;		// 位置
	float4 Color : COLOR;			// 頂点カラー
	float2 Tex : TEXCOORD0;			// テクスチャ座標
};

//---------------------------------------------------------
// --- 頂点シェーダー ---
//---------------------------------------------------------
VS_OUTPUT CA_VS(VS_INPUT In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	Out.Position = In.Position;
	Out.Color = In.Color;
	Out.Tex = In.Tex;

	return Out;
}

//---------------------------------------------------------
// --- ピクセルシェーダー ---
//---------------------------------------------------------
float4 CATEX_PS(VS_OUTPUT In) : COLOR0
{
	float2 TexCoord = In.Tex;		// テクスチャ座標を保存

	// 赤と青のオフセットを求める
	float2 redCoord = float2(distance.x, distance.y);
	float2 blueCoord = float2(-distance.x, -distance.y);

	// グリッチの座標から、帯を作成
	// 座標が適している場合、ずれる
	// stepの第二引数の大きさ分高さがずれる

	// abs(x) : 入力されたテクスチャ座標と基準のグリッチ座標を計算して、差分を絶対値で求める
	// step(y, x) : 求まった差分とxのどちらが大きいかを計算し、xの方が大きいと1を返す。小さいと0を返す。
	// -> 絶対値の差分が範囲内かを求め、範囲内なら1になる。
	float band1 = step(abs(TexCoord.y - g_fUpperGlitch), 0.03f);
	float band2 = step(abs(TexCoord.y - g_fLowerGlitch), 0.01f);

	// 帯に掛ける値分だけ、横にずれる
	// -> bandは範囲内なら1になるため掛け算が成立。範囲外なら0の為、掛け算が0になりずれない。
	TexCoord.x += 0.01f * band1;
	TexCoord.x -= 0.01f * band2;

	// RGBそれぞれをサンプリング
	float4 col;
	col.r = tex2D(TexSampler, TexCoord + redCoord).r;
	col.g = tex2D(TexSampler, TexCoord).g;
	col.b = tex2D(TexSampler, TexCoord + blueCoord).b;
	col.a = tex2D(TexSampler, TexCoord).a;

	return col;
}

//---------------------------------------------------------
// --- テクニック ---
//---------------------------------------------------------
technique TECHNIQUE_NAME
{
	pass P0
	{
		// 頂点シェーダーをコンパイル
		VertexShader = compile vs_3_0 CA_VS();

		// ピクセルシェーダーをコンパイル
		PixelShader = compile ps_3_0 CATEX_PS();
	}
}