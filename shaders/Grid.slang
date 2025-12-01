struct PushConstants
{
    float4x4 viewProj;
    float4x4 invViewProj; // Inverse for ray reconstruction
    float3 cameraPos;
    float gridScale; // Scale factor for zooming
};

[[vk::push_constant]]
PushConstants pc;

struct VSOut
{
    float4 position : SV_Position;
    float3 nearPoint : TEXCOORD0;
    float3 farPoint : TEXCOORD1;
};

// Fullscreen quad vertices (NDC space, covering entire screen)
static const float3 gridPlane[4] = {
    float3(-1.0, -1.0, 0.0), // bottom-left
    float3(1.0, -1.0, 0.0),  // bottom-right
    float3(-1.0, 1.0, 0.0),  // top-left
    float3(1.0, 1.0, 0.0)    // top-right
};

// Unproject NDC point to world space
float3 UnprojectPoint(float x, float y, float z)
{
    float4 unprojectedPoint = mul(pc.invViewProj, float4(x, y, z, 1.0));
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

VSOut vs_main(uint vertexID: SV_VertexID)
{
    float3 p = gridPlane[vertexID];

    VSOut output;
    output.position = float4(p, 1.0);                 // Already in NDC, no transform needed
    output.nearPoint = UnprojectPoint(p.x, p.y, 0.0); // Near plane
    output.farPoint = UnprojectPoint(p.x, p.y, 1.0);  // Far plane
    return output;
}

// Compute grid lines
float grid(float3 worldPos, float scale)
{
    float2 coord = worldPos.xz * scale; // Use XZ plane (horizontal)
    float2 derivative = fwidth(coord);  // Screen-space derivative for anti-aliasing
    float2 grid = abs(frac(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    return 1.0 - min(line, 1.0);
}

float4 ps_main(VSOut input) : SV_Target
{
    // Compute intersection with XZ plane (y = 0)
    float t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);

    // Discard if ray doesn't intersect plane (looking up or below horizon)
    if (t < 0.0)
        discard;

    float3 worldPos = input.nearPoint + t * (input.farPoint - input.nearPoint);

    // Compute depth for proper depth testing
    float4 clipPos = mul(pc.viewProj, float4(worldPos, 1.0));
    float depth = clipPos.z / clipPos.w;

    // Multi-scale grid (fade between levels)
    float gridScale = pc.gridScale;
    float grid1 = grid(worldPos, gridScale * 1.0);  // Fine grid
    float grid2 = grid(worldPos, gridScale * 0.1);  // Medium grid
    float grid3 = grid(worldPos, gridScale * 0.01); // Coarse grid

    // Combine grids with different intensities
    float gridPattern = max(grid1 * 0.5, max(grid2 * 0.7, grid3 * 1.0));

    // Fade grid at distance
    float distanceToCamera = length(worldPos - pc.cameraPos);
    float fadeStart = 50.0 / gridScale;
    float fadeEnd = 100.0 / gridScale;
    float fade = 1.0 - smoothstep(fadeStart, fadeEnd, distanceToCamera);

    // Axis lines (X = red, Z = blue)
    float axisLineWidth = 0.05 / gridScale;
    float xAxisLine = smoothstep(axisLineWidth, 0.0, abs(worldPos.z));
    float zAxisLine = smoothstep(axisLineWidth, 0.0, abs(worldPos.x));

    float3 gridColor = float3(0.2, 0.2, 0.2);  // Gray grid
    float3 xAxisColor = float3(1.0, 0.0, 0.0); // Red X axis
    float3 zAxisColor = float3(0.0, 0.0, 1.0); // Blue Z axis

    float3 color = gridColor * gridPattern * fade;
    color = lerp(color, xAxisColor, xAxisLine);
    color = lerp(color, zAxisColor, zAxisLine);

    float alpha = (gridPattern + xAxisLine + zAxisLine) * fade;

    // Discard fully transparent pixels
    if (alpha < 0.01)
        discard;

    return float4(color, alpha);
}
