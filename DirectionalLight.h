#pragma once
#include "Light.h"
class DirectionalLight :
    public Light
{
public:
	DirectionalLight() {};
	DirectionalLight(int number, Scene* scene, float3 lightColor, float3 direction, float intensity) {
		mLightColor = lightColor;
		mIntensity = intensity;
		mDirection = direction;
		mpScene = scene;
		mNumber = number;
	};

	float3 CalculateLight(float3 intersectionPoint, float3 Normal);
	virtual bool LightUI();

private:
	float3 mLightColor, mDirection;
	float mIntensity;
	Scene* mpScene;
};

