#include "precomp.h"
#include "SkyDome.h"
#include "stb_image.h"

mSkyDome::mSkyDome() {
    // load HDR sky
    // https://jacco.ompf2.com/2022/05/27/how-to-build-a-bvh-part-8-whitted-style/
    // this code was taken from Jacco's website. however I do understand how it works.

    int bpp = 0;
    skyPixels = stbi_loadf("assets/kloppenheim_02_puresky_4k.hdr", &skyWidth, &skyHeight, &bpp, 0);
    for (int i = 0; i < skyWidth * skyHeight * 3; i++) skyPixels[i] = sqrtf(skyPixels[i]);
}

float3 mSkyDome::GetSkyValue(float3 rayDirection) {
    //https://jacco.ompf2.com/2022/05/27/how-to-build-a-bvh-part-8-whitted-style/
    // this code is nearly identical to the code on Jacco's website and therefore not made by me.
    // However, I do understand how this worked and had to spent some time debugging to make it work in my project. 
    // sample sky
    uint u = (skyWidth - 1) * (atan2f(rayDirection.z, rayDirection.x) * INV2PI + 0.5f);
    uint v = (skyHeight-1) * acosf(rayDirection.y) * INVPI;
    uint skyIdx = u + v * skyWidth;
    return 0.65f * float3(skyPixels[skyIdx * 3], skyPixels[skyIdx * 3 + 1], skyPixels[skyIdx * 3 + 2]);
}