#pragma once
#include "Light.h"
#include "Ghost.h"
#include "Player.h"
#include "SkyDome.h"
namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	float3 Trace( Ray& ray , int rayDepth);
	float3 Glass(uchar previousVoxelMaterial, uchar mMaterialID, Tmpl8::Ray& ray, Tmpl8::float3& Normal, Tmpl8::float3& intersectionPoint, float epsilon, int rayDepth);
	float3 Smoke(uchar previousVoxelMaterial, uchar mMaterialID, Tmpl8::Ray& ray, Tmpl8::float3& Normal, Tmpl8::float3& intersectionPoint, float epsilon, int rayDepth);
	Ray Reflect(Ray& ray, uchar mMaterialID);
	float3 ApproxAces(float3 color);
	
	float SchlickApproximation(float cosTheta, float refractionIndex);
	float3 SnellsLaw(float cosAI, float refractionIndex1, float refractionIndex2, float3 D, float3 N);
	float Fresnel(float cosAI, float cosAT, float refractionIndex1, float refractionIndex2);
	float3 BeersLaw( float3 a, float d);

	float3 DirectIllumination(float3 I, float3 N);
	void Tick( float deltaTime );
	void UI();
	void Shutdown();
	// input handling
	void MouseUp(int button) {};
	//void MouseDown(int button);
	void MouseMove( int x, int y ) { mousePos.x = x, mousePos.y = y; }
	void MouseWheel( float y ) { /* implement if you want to handle the mouse wheel */ }
	void KeyUp( int key ) {  }
	void KeyDown(int key) { }
	// data members
	void ResetAccumulator();
	int2 mousePos;
	float4* mpAccumulator;
	Scene mpScene;

	enum GameState {
		Menu, Game, Pause
	}mGameState = Menu;

	Camera mCamera;
	float3 mGameCameraPos, mGameCameraTarget;
	Player mPlayer = Player(&mpScene, &mCamera);	
	Ghost* mGhost = new Ghost(&mpScene);
	vector<Light*> mLights;
	mSkyDome mSkyDome;
	int mFramesCount = 0;
	uint mTotalFrames = 0;
	int mScore = 0;
	int mRadius = 1;
	bool mToneMapping = true;
	bool mBlockPlace = true;
	bool mAutoFocus = false;
};

} 