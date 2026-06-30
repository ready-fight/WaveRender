struct VertexOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR0;
};

VertexOutput VSMain(uint vertexId : SV_VertexID)
{
    float2 positions[3] =
    {
        float2(0.0f, 0.5f),
        float2(0.5f, -0.5f),
        float2(-0.5f, -0.5f)
    };

    float3 colors[3] =
    {
        float3(1.0f, 0.15f, 0.15f),
        float3(0.15f, 1.0f, 0.15f),
        float3(0.15f, 0.35f, 1.0f)
    };

    VertexOutput output;
    output.Position = float4(positions[vertexId], 0.0f, 1.0f);
    output.Color = colors[vertexId];

    return output;
}

float4 PSMain(VertexOutput input) : SV_Target0
{
    return float4(input.Color, 1.0f);
}