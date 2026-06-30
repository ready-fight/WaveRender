struct VertexInput
{
    float3 Position : POSITION;
    float3 Color : COLOR0;
};

struct VertexOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR0;
};

VertexOutput VSMain(VertexInput input)
{
    VertexOutput output;
    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    return output;
}

float4 PSMain(VertexOutput input) : SV_Target0
{
    return float4(input.Color, 1.0f);
}