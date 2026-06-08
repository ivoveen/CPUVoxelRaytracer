#pragma once
#include "Light.h"
class PointLight :
    public Light
{
public:
	PointLight() {};
	PointLight(int number, Scene* scene, float3 worldPos, float3 lightColor, float intensity) {
		mWorldPos = worldPos;
		mLightColor = lightColor;
		mIntensity = intensity;
		mpScene = scene;
		mNumber = number;
	};
    float3 CalculateLight(float3 intersectionPoint, float3 Normal);
	bool LightUI();

private:
	Scene* mpScene;
};

