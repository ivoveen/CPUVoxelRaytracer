#pragma once
#include "PointLight.h"
#include "SpotLight.h"
class Player
{
public:

	Player(Scene* mScene, Camera* camera);
	~Player();
	void Tick(float deltaTime);
	inline bool GetUpdated() { return mChanged; }
	bool UI();

	enum PlayerPlayState {
		Walking, Sucking, Pause
	}mPlayerPlayState = Walking;

	enum Direction {
		Front, Left, Back, Right
	}mCurrentDirectionVector = Front;

	float3 mWorldPos = float3(4.0f, 0.0f, 3.0f);
	float mWalkSpeed = 0.002f;
	float mScale = 1.0f;
	PointLight* mpPlayerAmbient;
	SpotLight* mpPlayerFlash;

private:
	void Movement(float deltaTime);
	bool Scan(float deltaTime);
	void Suck(float deltaTime);

	const int mNumberOfCollisionRays = 10 * 0.5;
	
	//sucking
	const int mNumberOfScanRays = 100;
	VoxelObject* mKilledGhost = nullptr;
	float mElapsedT = 0;
	float mSuckSpeed = 0.001f;
	float3 mTargetColour = float3(1.0f,0.0f,0.0f);
	
	const float mAmbientIntensity = 0.1f;
	const float mFlashIntensity = 2.0f;

	bool mChanged = false;
	bool mFollowCamera = true;
	Scene* mpScene;
	Camera* mpCamera;

	//model and rotation info, hardcoded
	VoxelObject* mpModelFront;
	 float3 mFrontFlashOffset = float3(0.703f, 0.381f, 0.309f);
	 float3 mFrontAmbientOffset = float3(0.618f, 2.381f, 0.219f);

	VoxelObject* mpModelLeft;
	float3 mLeftFlashOffset = float3(0.476f, 0.381f, 0.595f);
	float3 mLeftAmbientOffset = float3(0.762f, 2.381f, 0.585f);

	VoxelObject* mpModelBack;
	float3 mBackFlashOffset = float3(0.226f, 0.381f, 0.380f);
	float3 mBackAmbientOffset = float3(0.476f, 2.381f, 0.695f);

	VoxelObject* mpModelRight;
	float3 mRightFlashOffset = float3(0.131f, 0.381f, 0.119f);
	float3 mRightAmbientOffset = float3(-0.382f, 2.381f, 0.314f);
};

