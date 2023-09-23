Texture2D tex;
SamplerState splr;

cbuffer CBuf : register(b1)
{
    float4 imgTint;
}

float4 main(float2 tc : TEXCOORD) : SV_TARGET {
    //return float4(0.5f, 0.5f, 0.5f, 0.5f);
    float4 sample = tex.Sample(splr, tc);
    sample *= imgTint;
    return sample;
}