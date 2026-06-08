#pragma once
class Ghost
{
public:
	Ghost(Scene* scene);
	~Ghost();
	void Init();
	void Tick(float deltaTime);
	inline bool GetUpdated() { return mChanged; }
	bool CollisionDetection(float3 offset);

	
	enum Direction {
		Front, Left, Back, Right
	}mCurrentDirection = Front;

	float3 mWorldPos = float3(4.0f, 0.0f, 3.0f);
	float mWalkSpeed = 0.0008f;
	float mScale = 1.0f;
	VoxelObject* mpCurrentModel;

private:
	void Movement(float deltaTime);
	float3 mCurrentDirectionVector = float3(-1.0f,0.0f,0.0f);
	float mWalkTime = 10;
	const int mNumberOfCollisionRays = 10 * 0.5;
	float mDistanceSinceChanged = 0;
	const float mMaxDistanceSinceChanged = 0.0f;
	

	bool mChanged = false;
	Scene* mpScene;

	//models
	VoxelObject* mpModelFront;
	VoxelObject* mpModelLeft;
	VoxelObject* mpModelBack;
	VoxelObject* mpModelRight;

};

