cbuffer CBuf {
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut {
    float2 tex : TEXCOORD;
    float4 pos : SV_Position;
};

VSOut main( float3 pos : Position, float2 tex : TEXCOORD ) {
    VSOut ret;
    ret.pos = mul(float4(pos, 1.0f), modelViewProj);
    ret.tex = tex;
    return ret;
}