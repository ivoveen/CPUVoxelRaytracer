#pragma once
class Light
{
public:
	float epsilon = 0.0001f;
	int mNumber;
	float mIntensity;
	float3 mLightColor;
	float3 mWorldPos = 0;
	virtual float3 CalculateLight(float3 intersectionPoint, float3 Normal) { return float3(); };
	virtual bool LightUI() { return false; };
};

