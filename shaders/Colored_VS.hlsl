cbuffer Cbuf
{
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{
    float4 color : COLOR;
    float4 pos : SV_Position;
};

VSOut main( float3 pos : POSITION, float4 color : COLOR)
{
    VSOut ret;
    ret.pos = mul(float4(pos, 1.0f), modelViewProj);
    ret.color = color;
    return ret;
}