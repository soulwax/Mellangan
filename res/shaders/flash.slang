// Sprite tint used for hit feedback: blends the sprite towards a flat colour, keeping its silhouette.

struct Push {
    int cameraIndex;
    uint textureIndex;
    float4 texturePos;
    float4 colour;
    float4x4 model;
};
[[vk::push_constant]] Push push;

[[vk::binding(1,1)]] SamplerState sampler;
[[vk::binding(2,2)]] Texture2D<float4> textures[];

struct FlashData {
    float amount;
    float r;
    float g;
    float b;
};
// VK2D reflects user data from the last global, so this must stay last.
[[vk::binding(3,3)]] ConstantBuffer<FlashData> flash;

[shader("pixel")]
float4 PixelShader(float2 uv) {
    float4 c = textures[push.textureIndex].Sample(sampler, uv) * push.colour;
    c.rgb = lerp(c.rgb, float3(flash.r, flash.g, flash.b), saturate(flash.amount));
    return c;
}
