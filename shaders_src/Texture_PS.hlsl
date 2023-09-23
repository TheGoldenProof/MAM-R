Texture2D tex;
SamplerState splr;

float4 main(float2 tc : TEXCOORD) : SV_Target
{
    //return float4(0.5f, 0.5f, 0.5f, 0.5f);
    float4 sample = tex.Sample(splr, tc);
    clip(sample.a < 0.1f ? -1 : 1);
    return sample;
}