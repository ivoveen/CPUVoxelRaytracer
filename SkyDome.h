#pragma once
class mSkyDome
{
public:
	mSkyDome();
	float3 GetSkyValue(float3 rayDirection);
private:
	float* skyPixels;
	int  skyWidth, skyHeight;
};



