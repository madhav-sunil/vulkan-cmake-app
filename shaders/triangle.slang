// Combined vertex + fragment in one Slang file (HLSL-like syntax)

struct VSOut {
    float4 position : SV_Position;
    float3 color    : COLOR0;
};

// Simple hard-coded triangle
VSOut vs_main(uint vertexID : SV_VertexID)
{
    float2 pos[3] = {
        float2( 0.0, -0.5),
        float2( 0.5,  0.5),
        float2(-0.5,  0.5)
    };

    float3 col[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };

    VSOut o;
    o.position = float4(pos[vertexID], 0.0, 1.0);
    o.color = col[vertexID];
    return o;
}

// Fragment: solid red (you can use IN.color instead if you want interpolated color)
float4 ps_main(VSOut IN) : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}