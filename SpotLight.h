#pragma once
#include "Light.h"
class SpotLight :
    public Light
{
public:
	SpotLight() {};
	SpotLight(int number, Scene* scene, float3 worldPos, float3 lightColor, float3 direction, float intensity, float coneSize, float softBandSize) {
		mNumber = number;
		mWorldPos = worldPos;
		mLightColor = lightColor;
		mIntensity = intensity;
		mConeSize = coneSize;
		mSoftBandSize = softBandSize;
		mDirection = direction;
		mpScene = scene;
	};
	float3 CalculateLight(float3 intersectionPoint, float3 Normal);
	bool LightUI();
	float3 mDirection;
	float mSoftBandSize;
private:
	float mConeSize;
	Scene* mpScene;
};

