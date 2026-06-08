#include "precomp.h"
#include "Player.h"


Player::Player(Scene* scene, Camera* mCamera) {
	mpScene = scene;
	mpCamera = mCamera;
	mpPlayerAmbient = new PointLight(100, mpScene, mWorldPos + float3(0.5f, 2.0f, 0.5f), float3(1.0f, 1.0f, 1.0f), mAmbientIntensity);
	mpPlayerFlash = new SpotLight(101, mpScene, mWorldPos + float3(0.75f, 0.5f, -0.75f), float3(1.0f, 1.0f, 1.0f), float3(0.0f, 0.0f, -1.0f), mFlashIntensity, 1.3f, 0.666f);

	mpModelFront =  new VoxelObject(float3(0.0f, 0.0f, 0.0f), mScale, mpScene->mpScene->models[mpScene->mpScene->num_models - 1], &mpScene->mMaterials);
	mpModelLeft =  new VoxelObject(float3(2.0f, 0.0f, 0.0f), mScale, mpScene->mpScene->models[mpScene->mpScene->num_models - 2], &mpScene->mMaterials);
	mpModelBack =  new VoxelObject(float3(4.0f, 0.0f, 0.0f), mScale, mpScene->mpScene->models[mpScene->mpScene->num_models - 3], &mpScene->mMaterials);
	mpModelRight =  new VoxelObject(float3(6.0f, 0.0f, 0.0f), mScale, mpScene->mpScene->models[mpScene->mpScene->num_models - 4], &mpScene->mMaterials);
}

Player::~Player(){
	delete mpPlayerAmbient;
	delete mpPlayerFlash;
	delete mpModelFront;
	delete mpModelBack;
	delete mpModelLeft;
	delete mpModelRight;
}

void Player::Tick(float deltaTime) {
	mChanged = false;
	
	switch (mPlayerPlayState)
	{
	case Player::Walking:
		Movement(deltaTime);
		if (IsKeyDown(GLFW_KEY_SPACE)) {
			if (Scan(deltaTime)) mPlayerPlayState = Sucking, mKilledGhost->mObjectPlayState = VoxelObject::Pause;
		}

		break;
	case Player::Sucking:
		if (!IsKeyDown(GLFW_KEY_SPACE)) mPlayerPlayState = Walking;
		else {
			Suck(deltaTime);
		}
		break;
	case Player::Pause:
		break;
	default:
		break;
	}
	
}

void Player::Suck(float deltaTime) {
	mChanged = true;
	mElapsedT += deltaTime * mSuckSpeed * 3;
	
	//move and shrink the ghost
	float3 ghostPos = mKilledGhost->mWorldPos;
	float3 suckDirection = mWorldPos - ghostPos;
	float suckLength = length(suckDirection);
	suckDirection = suckDirection / suckLength;

	if (suckLength < mSuckSpeed * deltaTime) {
		//sucking is complete
		mPlayerPlayState = Walking;
		mKilledGhost->mObjectPlayState = VoxelObject::Dead;
		mpPlayerFlash->mLightColor = float3(1.0f, 1.0f, 1.0f);
		mpPlayerAmbient->mLightColor = float3(1.0f, 1.0f, 1.0f);
		mpPlayerFlash->mIntensity = mFlashIntensity;
		mElapsedT = 0;
	}
	float suckTicks = suckLength / (mSuckSpeed * deltaTime);
	float ghostScale = mKilledGhost->mScale;
	float scaleDecrement = ghostScale / suckTicks;

	mKilledGhost->SetPosition(ghostPos + (suckDirection * mSuckSpeed * deltaTime));
	mKilledGhost->SetScale(ghostScale - scaleDecrement);

	//destroy the ghost slowly
	float3 ghostDimensions = mKilledGhost->mDimensions;
	mKilledGhost->SetCircle( float3(RandomFloat() * (ghostDimensions.x - 6) + 5, RandomFloat() * (ghostDimensions.y - 6) + 5, RandomFloat() * (ghostDimensions.z - 6) + 5), 4, 0);


	//make spotlight go rainbow
	if (mElapsedT > 1) {
		mTargetColour = float3(RandomFloat(), RandomFloat(), RandomFloat());
		mElapsedT -= 1;
	}
	float3 newColour = lerp(mpPlayerFlash->mLightColor, mTargetColour, mElapsedT);
	mpPlayerFlash->mLightColor = newColour;
	mpPlayerAmbient->mLightColor = newColour;
	//make spotlight go bright
	mpPlayerFlash->mIntensity += RandomFloat() * 0.5f - 0.2f;



}


bool Player::Scan(float deltaTime) {
 	float maxDegree = acos(mpPlayerFlash->mSoftBandSize);
	float incrementDegree = (maxDegree * 2.0f) / mNumberOfScanRays;
	float currentDegree = -maxDegree;
	for (int i = 0; i < mNumberOfScanRays; i++) {
		float3 rotatedVector = normalize(mpPlayerFlash->mDirection.RotateVectorY(currentDegree));
		Ray ray = Ray(mpPlayerFlash->mWorldPos + (rotatedVector * 0.05f), rotatedVector);
		mpScene->FindNearest(ray);
		if (ray.voxel == 234 || ray.voxel == 238) {
			//we hit a ghost
			mKilledGhost = (VoxelObject*)ray.nearestVoxelObject;
			return true;
		}
		currentDegree += incrementDegree;
	}
	return false;
}

void Player::Movement(float deltaTime) {
	Direction direction = mCurrentDirectionVector;
	float3 offset = float3(0.0f, 0.0f, 0.0f);
	
	//get input
	if (IsKeyDown(GLFW_KEY_W)) offset = mWalkSpeed * deltaTime * float3(0.0f, 0.0f, -1.0f), direction = Front, mChanged = true;
	else if (IsKeyDown(GLFW_KEY_A)) offset = mWalkSpeed * deltaTime * float3(1.0f, 0.0f, 0.0f), direction = Left, mChanged = true;
	else if (IsKeyDown(GLFW_KEY_S)) offset = mWalkSpeed * deltaTime * float3(0.0f, 0.0f, 1.0f), direction = Back, mChanged = true;
	else if (IsKeyDown(GLFW_KEY_D)) offset = mWalkSpeed * deltaTime * float3(-1.0f, 0.0f, 0.0f), direction = Right, mChanged = true;
	else return;

	//rotate player when switching direction
	if (direction != mCurrentDirectionVector) {
		VoxelObject* newDirectionModel;
		switch (direction)
		{
		case Front:
			newDirectionModel = mpModelFront;
			mpPlayerFlash->mDirection = float3(0.0f, 0.0f, -1.0f);
			mpPlayerFlash->mWorldPos = mWorldPos + mFrontFlashOffset;
			mpPlayerAmbient->mWorldPos = mWorldPos + mFrontAmbientOffset;
			break;
		case Left:
			newDirectionModel = mpModelLeft;
			mpPlayerFlash->mDirection = float3(1.0f, 0.0f, 0.0f);
			mpPlayerFlash->mWorldPos = mWorldPos + mLeftFlashOffset;
			mpPlayerAmbient->mWorldPos = mWorldPos + mLeftAmbientOffset;
			break;	
		case Right:
			newDirectionModel = mpModelRight;
			mpPlayerFlash->mDirection = float3(-1.0f, 0.0f, 0.0f);
			mpPlayerFlash->mWorldPos = mWorldPos + mRightFlashOffset;
			mpPlayerAmbient->mWorldPos = mWorldPos + mRightAmbientOffset;
			break;	
		case Back:
			newDirectionModel = mpModelBack;
			mpPlayerFlash->mDirection = float3(0.0f, 0.0f, 1.0f);
			mpPlayerFlash->mWorldPos = mWorldPos + mBackFlashOffset;
			mpPlayerAmbient->mWorldPos = mWorldPos + mBackAmbientOffset;
			break;	
		}
		newDirectionModel->SetScale(mScale);
		newDirectionModel->SetPosition(mWorldPos);

		mpScene->mObjects[0] = newDirectionModel;
		mCurrentDirectionVector = direction;
	}

	//check for collisions
	float3 Origin = float3(0.5f, 0.381f, 0.5f) + mWorldPos;
	Ray ray = Ray(Origin +  (mpPlayerFlash->mDirection * 0.5), mpPlayerFlash->mDirection, length(offset) *4);
	if (!mpScene->IsOccluded(ray))
	{
		//move the player if theres no collision
		mWorldPos += offset;
		mpScene->mObjects[0]->SetPosition(mWorldPos);
		mpPlayerAmbient->mWorldPos += offset;
		mpPlayerFlash->mWorldPos += offset;
		if (mFollowCamera) {
			mpCamera->camPos += offset;
			mpCamera->camTarget = mpCamera->camPos + mpCamera->ahead;
		}

	}


}


bool Player::UI() {
	bool mChanged = false;
	char label[32];
	sprintf(label, "Player %i", 1);
	if (ImGui::CollapsingHeader(label, 1))
	{
		if(ImGui::Checkbox("Auto focus", &mFollowCamera)) mChanged = true;

		ImGui::Text("Player scale");
		sprintf(label, "scale %i", 1);
		if (ImGui::SliderFloat(label, &mScale, 0.0f, 3.0f)) mChanged = true;
		ImGui::Text("Player Position");
		sprintf(label, "X %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mWorldPos.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Y %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mWorldPos.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Z %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mWorldPos.z, -4.0f, 4.0f)) mChanged = true;


		ImGui::Text("front flash");
		sprintf(label, "Xff %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mFrontFlashOffset.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Yff %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mFrontFlashOffset.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Zff %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mFrontFlashOffset.z, -4.0f, 4.0f)) mChanged = true;

		ImGui::Text("front ambient");
		sprintf(label, "Xfa %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mFrontAmbientOffset.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Yfa %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mFrontAmbientOffset.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Zfa %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mFrontAmbientOffset.z, -4.0f, 4.0f)) mChanged = true;




		ImGui::Text("left flash");
		sprintf(label, "Xlf %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mLeftFlashOffset.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Ylf %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mLeftFlashOffset.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Zlf %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mLeftFlashOffset.z, -4.0f, 4.0f)) mChanged = true;

		ImGui::Text("left ambient");
		sprintf(label, "Xla %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mLeftAmbientOffset.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Yla %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mLeftAmbientOffset.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Zla %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mLeftAmbientOffset.z, -4.0f, 4.0f)) mChanged = true;


		ImGui::Text("back flash");
		sprintf(label, "Xbf %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mBackFlashOffset.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Ybf %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mBackFlashOffset.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Zbf %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mBackFlashOffset.z, -4.0f, 4.0f)) mChanged = true;

		ImGui::Text("back ambient");
		sprintf(label, "Xba %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mBackAmbientOffset.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Yba %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mBackAmbientOffset.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Zba %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mBackAmbientOffset.z, -4.0f, 4.0f)) mChanged = true;

		ImGui::Text("right flash");
		sprintf(label, "Xrf %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mRightFlashOffset.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Yrf %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mRightFlashOffset.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Zrf %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mRightFlashOffset.z, -4.0f, 4.0f)) mChanged = true;

		ImGui::Text("right ambient");
		sprintf(label, "Xra %i", 1);
		ImGui::SetNextItemWidth(100);
		if (ImGui::SliderFloat(label, &mRightAmbientOffset.x, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Yra %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mRightAmbientOffset.y, -4.0f, 4.0f)) mChanged = true;
		sprintf(label, "Zra %i", 1);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if (ImGui::SliderFloat(label, &mRightAmbientOffset.z, -4.0f, 4.0f)) mChanged = true;





	}
	return mChanged;
}