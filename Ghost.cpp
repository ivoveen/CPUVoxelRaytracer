#include "precomp.h"
#include "Ghost.h"


Ghost::Ghost(Scene* scene) {
	mpScene = scene;
	mpModelFront = new VoxelObject(float3(0.0f, 0.0f, 0.0f), mScale, mpScene->mpScene->models[mpScene->mpScene->num_models - 9], &mpScene->mMaterials);
	mpModelLeft = new VoxelObject(float3(2.0f, 0.0f, 0.0f), mScale, mpScene->mpScene->models[mpScene->mpScene->num_models - 10], &mpScene->mMaterials);
	mpModelBack = new VoxelObject(float3(4.0f, 0.0f, 0.0f), mScale, mpScene->mpScene->models[mpScene->mpScene->num_models - 11], &mpScene->mMaterials);
	mpModelRight = new VoxelObject(float3(6.0f, 0.0f, 0.0f), mScale, mpScene->mpScene->models[mpScene->mpScene->num_models - 12], &mpScene->mMaterials);

	VoxelObject* newDirectionModel = mpModelRight;
	newDirectionModel->SetScale(mScale);
	newDirectionModel->SetPosition(mWorldPos);
	mpCurrentModel = newDirectionModel;
	mpScene->mObjects[1] = newDirectionModel;


	mWorldPos = float3(RandomFloat() * 15.5f  + 1.75f, 0.0f, RandomFloat() * 7.1f - 2.24f);
}

void Ghost::Init() {

	
	mpModelFront->StoreGrid();	
	mpModelLeft ->StoreGrid();	
	mpModelBack ->StoreGrid();	
	mpModelRight->StoreGrid();	

	VoxelObject* newDirectionModel = mpModelRight;
	newDirectionModel->SetScale(mScale);
	newDirectionModel->SetPosition(mWorldPos);
	mpCurrentModel = newDirectionModel;
	mpScene->mObjects[1] = newDirectionModel;


	mWorldPos = float3(RandomFloat() * 15.5f + 1.75f, 0.0f, RandomFloat() * 7.1f - 2.24f);
}
Ghost::~Ghost() {
	delete mpModelFront;
	delete mpModelBack;
	delete mpModelLeft;
	delete mpModelRight;
}

void Ghost::Tick(float deltaTime) {
	mChanged = false;

	switch (mpCurrentModel->mObjectPlayState)
	{
	case VoxelObject::Pause:
		//im getting sucked
		break;
	case VoxelObject::Normal:
		Movement(deltaTime);
		break;
	}
}



void Ghost::Movement(float deltaTime) {
	Direction direction = mCurrentDirection;
	float3 offset = float3(0.0f, 0.0f, 0.0f);

	offset = mWalkSpeed * deltaTime * mCurrentDirectionVector;
	mWalkTime -= deltaTime * mWalkSpeed;

	//check for collisions
	//float3 rayDirection = normalize(offset);
	//Ray ray = Ray(Origin + (rayDirection * 0.5), rayDirection, length(offset) * 4);




	if (CollisionDetection(offset) || mWalkTime < 0)
	{
		if (RandomFloat() * 8 <= 1) {
			mCurrentDirectionVector = float3(0.0f, 0.0f, 0.0f);
		}
		else {
			mCurrentDirectionVector = normalize(float3(RandomFloat() * 2 - 1, 0, RandomFloat() * 2 - 1));
		}

		//rotate ghost when switching direction
		VoxelObject* newDirectionModel;

		if (mCurrentDirectionVector.x > mCurrentDirectionVector.z) {
			if (mCurrentDirectionVector.x > 0) newDirectionModel = mpModelLeft;
			else newDirectionModel = mpModelRight;
		}
		else {
			if (mCurrentDirectionVector.z > 0) newDirectionModel = mpModelBack;
			else newDirectionModel = mpModelFront;
		}
		newDirectionModel->SetScale(mScale);
		newDirectionModel->SetPosition(mWorldPos);
		mpCurrentModel = newDirectionModel;
		mpScene->mObjects[1] = newDirectionModel;
		mCurrentDirection = direction;
		mWalkTime = RandomFloat() * 10;
	}
	else if(mCurrentDirectionVector != float3(0.0f,0.0f,0.0f))
	{
		mDistanceSinceChanged += deltaTime * mWalkSpeed;
		if (mDistanceSinceChanged > mMaxDistanceSinceChanged)mChanged = true, mDistanceSinceChanged = 0;
		//move the player if theres no collision
		mWorldPos += offset;
		mpScene->mObjects[1]->SetPosition(mWorldPos);
	}
}

bool Ghost::CollisionDetection(float3 offset) {
	float3 Origin = float3(0.5f, 0.381f, 0.5f) + mWorldPos;
	float maxDegree = 0.25 * PI;
	float incrementDegree = (maxDegree * 2.0f) / mNumberOfCollisionRays;
	float currentDegree = -maxDegree;
	for (int i = 0; i < mNumberOfCollisionRays; i++) {
		float3 rotatedVector = normalize(mCurrentDirectionVector.RotateVectorY(currentDegree));
		Ray ray = Ray(Origin + (rotatedVector * 0.5f), rotatedVector, length(offset) * 4);
		if (mpScene->IsOccluded(ray)) return true;
		currentDegree += incrementDegree;
	}
	return false;
}
