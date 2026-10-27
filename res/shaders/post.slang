struct Push
{
    int cameraIndex;
    uint textureIndex;
    float4 texturePos;
    float4 colour;
    float4x4 model;
};
[[vk::push_constant]]
Push push;

[[vk::binding(1, 1)]]
SamplerState sampler;
[[vk::binding(2, 2)]]
Texture2D<float4> textures[];

struct PostData
{
    float time;
    float hurt;        // 0..1, spikes when the player takes damage
    float lowHealth;   // 0..1, how close to death
    uint lightTexture; // bindless index of the light map (same size as the world target)
    float2 resolution; // world target size in pixels
    float vignette;
    float grain;
};
[[vk::binding(3, 3)]]
ConstantBuffer<PostData> post;

float3 albedoAt(float2 uv)
{
    return textures[push.textureIndex].Sample(sampler, uv).rgb;
}
float3 lightAt(float2 uv)
{
    float2 p = uv - 0.5;
    float2 i = floor(p);
    float2 f = p - i;
    float2 base = i + 0.5;
    float3 a = textures[post.lightTexture].Sample(sampler, base).rgb;
    float3 b = textures[post.lightTexture].Sample(sampler, base + float2(1, 0)).rgb;
    float3 c = textures[post.lightTexture].Sample(sampler, base + float2(0, 1)).rgb;
    float3 d = textures[post.lightTexture].Sample(sampler, base + float2(1, 1)).rgb;
    float3 l = lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
    return l * 1.6; // light map stores 0.625 as neutral so lights can over-expose
}

float3 litAt(float2 uv)
{
    return albedoAt(uv) * textures[post.lightTexture].Sample(sampler, uv).rgb * 1.6;
}

float3 aces(float3 x)
{
    return saturate((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14));
}

float hash(float2 p)
{
    return frac(sin(dot(p, float2(12.9898, 78.233))) * 43758.5453);
}

[shader("pixel")]
float4 PixelShader(float2 uv)
{
    float2 res = post.resolution;
    float2 n = uv / res - 0.5;
    float dist = length(n * float2(res.x / res.y, 1.0)) * 1.15;
    float2 ca = n * (0.35 + post.hurt * 4.0);
    float3 albedo = float3(albedoAt(uv + ca).r, albedoAt(uv).g, albedoAt(uv - ca).b);
    float3 col = albedo * lightAt(uv);
    float2 taps[8] = {
        float2(1, 0), float2(-1, 0), float2(0, 1), float2(0, -1),
        float2(0.7, 0.7), float2(-0.7, 0.7), float2(0.7, -0.7), float2(-0.7, -0.7)
    };
    float3 bloom = 0;
    for (int k = 0; k < 8; k++)
        bloom += max(litAt(uv + taps[k] * 1.5) - 1.15, 0.0);
    col += min(bloom * 0.06, 0.2);

    col = aces(col * 1.15);
    float luma = dot(col, float3(0.2126, 0.7152, 0.0722));
    col = lerp(col, col * float3(0.92, 0.97, 1.12), 1.0 - saturate(luma * 1.6));
    col = lerp(float3(luma, luma, luma), col, 1.12);

    col *= 1.0 - smoothstep(0.55, 1.25, dist) * post.vignette;

    float edge = smoothstep(0.45, 1.1, dist);
    float pulse = 0.6 + 0.4 * sin(post.time * 7.0);
    col = lerp(col, float3(0.5, 0.02, 0.06), edge * post.lowHealth * pulse * 0.65);
    col += float3(0.4, 0.03, 0.05) * post.hurt * edge;

    col += (hash(floor(uv) + frac(post.time * 7.31) * 113.0) - 0.5) * post.grain;

    return float4(saturate(col), 1.0) * push.colour;
}
