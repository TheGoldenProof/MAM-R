cbuffer Cbuf
{
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{
    float4 color : COLOR;
    float4 pos : SV_Position;
    float4 worldPos : TEXCOORD0;
};

VSOut main(float3 pos : POSITION, float4 color : COLOR)
{
    VSOut ret;
    ret.color = color;
    ret.pos = mul(float4(pos, 1.0f), modelViewProj);
    ret.worldPos = float4(pos, 0);
    return ret;
}