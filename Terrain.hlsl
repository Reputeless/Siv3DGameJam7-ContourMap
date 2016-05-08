
struct VS_INPUT
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	row_major float4x4 worldMatrix : MATRIX;
	float4 diffuseColor : COLOR;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 worldPosition : TEXCOORD0;
	float4 color : TEXCOORD1;
	float2 tex : TEXCOORD2;
};

struct PS_OUTPUT
{
	float4 color : SV_Target0;
	float  depth : SV_Target1;
	float4 normal : SV_Target2;
};

//-------------------------------------------------------------

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
SamplerState sampler0 : register(s0);

cbuffer vscbMesh0 : register( b0 )
{
	row_major float4x4 g_viewProjectionMatrix;
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	input.pos.y *= texture0.SampleLevel(sampler0, input.tex, 0).r * 80;

	const float4 posWS = mul(input.pos, input.worldMatrix);

	output.pos = mul(posWS,g_viewProjectionMatrix);

	output.worldPosition = posWS.xyz;

	output.color = input.diffuseColor;

	output.tex = input.tex;

	return output;
}

//-------------------------------------------------------------


cbuffer pscbMesh0 : register(b0)
{
	float3 g_cameraPosition;
	uint g_fogType;
	float4 g_fogParam;
	float4 g_fogColor;
}

cbuffer pscbMesh1 : register(b1)
{
	float4 g_param;
}

float3 Hue(float hue)
{
	float3 rgb = frac(hue + float3(0.0, 2.0 / 3.0, 1.0 / 3.0));
	rgb = abs(rgb*2.0 - 1.0);
	return clamp(rgb*3.0 - 1.0, 0.0, 1.0);
}

float3 HSVtoRGB(float3 hsv)
{
	return ((Hue(hsv.x) - 1.0)*hsv.y + 1.0) * hsv.z;
}

PS_OUTPUT PS(VS_OUTPUT input)
{
	PS_OUTPUT output;

	const float3 textureColor = texture0.Sample(sampler0, input.tex * 200).rgb;
	const float y = -input.worldPosition.y / 200.0 + 0.4;
	const float h = floor(y * 40) / 40;
	const float b = abs((y * 40) - round(y * 40));

	float3 lineColor = HSVtoRGB(float3(h, 0.9, 0.8));
	if (b < 0.02)
	{
		lineColor *= pow((b / 0.02), 8);
	}

	output.color.rgb = textureColor * g_param.x + lineColor * g_param.y;
	output.color.a = 1.0;

	output.depth.r = distance(g_cameraPosition.xyz, input.worldPosition);
	output.normal = texture1.Sample(sampler0, input.tex);

	return output;
}
