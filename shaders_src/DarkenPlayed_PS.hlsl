cbuffer Cbuf : register(b0)
{
    float planeX;
};

struct VSOut
{
    float4 color : COLOR;
    float4 pos : SV_Position;
    float4 worldPos : TEXCOORD0;
};

static float highlightMaxDist = 400.0f;
static float highlightStrength = 4.0f;
static float minFogDist = 1000.0f;
static float fogSize = 2000.0f;

float4 main(VSOut input) : SV_TARGET
{
    float4 color = input.color;
    float posX = input.worldPos.x;
    
    float dist = posX - planeX;
    float factor = 1.0f;
    
    if (dist < 0)
    {
        color.rgb /= 2.0f;
    }
    else
    {
        if (dist < highlightMaxDist)
        {
            float lerpA = 1.0f - dist / highlightMaxDist;
            factor = lerp(1.0f, highlightStrength, lerpA);
            clip(factor);
            color.rgb *= factor;
            color = lerp(color, float4(1.0f, 1.0f, 1.0f, 1.0f), lerpA*lerpA*0.5f);
        }
    }
    float absDist = abs(dist);
    if (absDist > minFogDist)
    {
        factor = 1.0f - (absDist - minFogDist) / fogSize;
        clip(factor);
        color.rgb = lerp(float3(0, 0, 0), color.rgb, factor*factor);
    }
    
    
    return color;
}