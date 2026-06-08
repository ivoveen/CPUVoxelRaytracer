#include "precomp.h"
// YOU GET:
// 1. A fast voxel renderer in plain C/C++
// 2. Normals and voxel colors
// FROM HERE, TASKS COULD BE:							FOR SUFFICIENT
// * Materials:
//   - Reflections and diffuse reflections				<===
//   - Transmission with Snell, Fresnel					<===
//   - Textures, Minecraft-style						<===
//   - Beer's Law
//   - Normal maps
//   - Emissive materials with postproc bloom
//   - Glossy reflections (BASIC)
//   - Glossy reflections (microfacet)
// * Light transport:
//   - Point lights										<===
//   - Spot lights										<===
//   - Area lights										<===
//	 - Sampling multiple lights with 1 ray
//   - Importance-sampling
//   - Image based lighting: sky
// * Camera:
//   - Depth of field									<===
//   - Anti-aliasing									<===
//   - Panini, fish-eye etc.
//   - Post-processing: now also chromatic				<===
//   - Spline cam, follow cam, fixed look-at cam
//   - Low-res cam with CRT shader
// * Scene:
//   - HDR skydome										<===
//   - Spheres											<===
//   - Smoke & trilinear interpolation
//   - Signed Distance Fields
//   - Voxel instances with transform
//   - Triangle meshes (with a BVH)
//   - High-res: nested grid
//   - Procedural art: shapes & colors
//   - Multi-threaded Perlin / Voronoi
// * Various:
//   - Object picking
//   - Ray-traced physics
//   - Profiling & optimization
// * GPU:
//   - GPU-side Perlin / Voronoi
//   - GPU rendering *not* allowed!
// * Advanced:
//   - Ambient occlusion
//   - Denoising for soft shadows
//   - Reprojection for AO / soft shadows
//   - Line lights, tube lights, ...
//   - Bilinear interpolation and MIP-mapping
// * Simple game:										
//   - 3D Arkanoid										<===
//   - 3D Snake?
//   - 3D Tank Wars for two players
//   - Chess
// REFERENCE IMAGES:
// https://www.rockpapershotgun.com/minecraft-ray-tracing
// https://assetsio.reedpopcdn.com/javaw_2019_04_20_23_52_16_879.png
// https://www.pcworld.com/wp-content/uploads/2023/04/618525e8fa47b149230.56951356-imagination-island-1-on-100838323-orig.jpg
#include<omp.h>
#include"Light.h"
#include"PointLight.h"
#include"SpotLight.h"
#include"DirectionalLight.h"

void Renderer::ResetAccumulator() {
	mFramesCount = 0;
	memset(mpAccumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
}
// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init()
{
	// create fp32 rgb pixel buffer to render to
	mpAccumulator = (float4*)MALLOC64(SCRWIDTH * SCRHEIGHT * 16);
	mFramesCount = 0;
	memset(mpAccumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
	
	// try to load a camera
	//FILE* f = fopen("camera.bin", "rb");
	//if (f)
	//{
	//	fread(&camera, 1, sizeof(Camera), f);
	//	fclose(f);
	//}
	//light init
	mLights.push_back(mPlayer.mpPlayerAmbient);
	mLights.push_back(mPlayer.mpPlayerFlash);

}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
float3 Renderer::Trace(Ray& ray, int rayDepth = 10) {
	if (rayDepth <= 0) return mSkyDome.GetSkyValue(ray.D);
	rayDepth--;
	float epsilon = 0.0001f;

	mpScene.FindNearest(ray);

	//check if there is only air involved, if so just print the skybox.
	uchar previousVoxelMaterial = ray.previousVoxel;
	uchar mMaterialID = ray.voxel; 
	if (mMaterialID <= 0 && previousVoxelMaterial <= 0) {
		return mSkyDome.GetSkyValue(ray.D);
	}
	//get data
	float3 intersectionPoint = ray.IntersectionPoint();
	float3 Normal = ray.GetNormal();
	float3 albedo = mpScene.mMaterials[mMaterialID]->albedo; //if not air;

	//glass is involved
	if (mpScene.mMaterials[previousVoxelMaterial]->type == Material::glass || mpScene.mMaterials[mMaterialID]->type == Material::glass) {
		return Glass(previousVoxelMaterial, mMaterialID, ray, Normal, intersectionPoint, epsilon, rayDepth);
	}
	//smoke is involved
	if (mpScene.mMaterials[previousVoxelMaterial]->type == Material::smoke || mpScene.mMaterials[mMaterialID]->type == Material::smoke) {
		return Smoke(previousVoxelMaterial, mMaterialID, ray, Normal, intersectionPoint, epsilon, rayDepth);
	}

	// check specularness
	float reflectionNormalIncidence = mpScene.mMaterials[mMaterialID]->reflectionNormalIncidence;
	if (reflectionNormalIncidence) {
		float reflectance = SchlickApproximation(-dot(ray.D, Normal), reflectionNormalIncidence);
		float randomFloat = RandomFloat();
		if (randomFloat < reflectance) {
			//reflect ray
			return  Trace(Reflect(ray, mMaterialID), rayDepth);
		}
	}
	//diffuse ray
	return albedo * (DirectIllumination(intersectionPoint, Normal));

	/*visualize normal*/ 
	///* visualize distance */ //return float3( 1 / (1 + ray.t) );
	///* visualize albedo */  //return albedo;
}

float3 Renderer::Glass(uchar previousVoxelMaterial, uchar mMaterialID, Tmpl8::Ray& ray, Tmpl8::float3& Normal, Tmpl8::float3& intersectionPoint, float epsilon, int rayDepth)
{
	float refractionIndex1 = mpScene.mMaterials[previousVoxelMaterial]->refractiveIndex;
	float refractionIndex2 = mpScene.mMaterials[mMaterialID]->refractiveIndex;

	float cosAI = dot(Normal, -ray.D);
	float3 refractDirection = normalize(SnellsLaw(cosAI, refractionIndex1, refractionIndex2, ray.D, Normal));

	if (refractDirection != NULL) {
		float fresnelProbability = Fresnel(cosAI, dot(-Normal, refractDirection), refractionIndex1, refractionIndex2);
		//refract ray
		float randomFloat = RandomFloat();
		if (randomFloat > fresnelProbability) {
			//refract
			float3 color = float3(1.0f, 1.0f, 1.0f);
			if (mpScene.mMaterials[previousVoxelMaterial]->type == Material::glass) {
				ray.tGlass += ray.t;
				color = BeersLaw( color - mpScene.mMaterials[previousVoxelMaterial]->albedo, ray.tGlass);
			}
			else ray.tGlass = 0;

			return color * Trace(Ray(intersectionPoint + (refractDirection * epsilon), refractDirection), rayDepth);
		}
	}
	//keep track of distance in the glass.
	if (mpScene.mMaterials[previousVoxelMaterial]->type == Material::glass) ray.tGlass += ray.t;
	else ray.tGlass = 0;
	//TIR or reflection from fresnell
	return  Trace(Reflect(ray, mMaterialID), rayDepth);
}

float3 Renderer::Smoke(uchar previousVoxelMaterial, uchar mMaterialID, Tmpl8::Ray& ray, Tmpl8::float3& Normal, Tmpl8::float3& intersectionPoint, float epsilon, int rayDepth)
{
	float3 color = float3(1.0f, 1.0f, 1.0f);
	float3 lightColor = float3(0.1f, 0.1f, 0.1f);
	Light* light = mLights[3];
	if (mpScene.mMaterials[previousVoxelMaterial]->type == Material::smoke) {
		//sample smoke density
		float segmentT = ray.t * 0.25 * 2;
		float totalDensityAddedDistance = 0;
		float totalDensity = 0;
		for (int i = 1; i < 3; i++) {
			float3 samplePoint = (ray.O + ray.D * (segmentT * i));
			float density = noise3DZeroToOne(samplePoint.x * 100 + 1000, (samplePoint.y * 100) - (mTotalFrames * 0.5), samplePoint.z * 100) * 10;
			totalDensity += density * segmentT;

			//get amount of light at this point
			float3 lightDirection = normalize(light->mWorldPos - samplePoint);
			Ray lightRay = Ray(samplePoint - (lightDirection * epsilon), lightDirection);
			mpScene.FindNearest(lightRay);
			float lightSegmentT = lightRay.t * 0.25 * 2;
			float totalLightdensity = 0;
			for (int j = 1; j < 3; j++) {
				float3 lightSamplePoint = (ray.O + ray.D * (lightSegmentT * j));
				float density = noise3DZeroToOne(lightSamplePoint.x * 100 + 1000, (lightSamplePoint.y * 100) - (mTotalFrames * 0.5), lightSamplePoint.z * 100);
				density *= density;
				totalLightdensity += density * lightSegmentT;
			}
			//add more distance attentuation.
			totalDensityAddedDistance += (lightRay.t - totalLightdensity);
		}
		//calculate light 
		float3 lightDirection = intersectionPoint - light->mWorldPos;
		float distanceToLight = length(lightDirection);
		float inverseDistance = 1 / (distanceToLight + totalDensityAddedDistance);
		lightDirection = lightDirection * inverseDistance;
		float distanceAttenuation = (inverseDistance * inverseDistance); //distance attenuation
		lightColor += (light->mLightColor * distanceAttenuation * light->mIntensity); //final color		

		float3 beersColor =  BeersLaw(float3(1.0f,1.0f,1.0f), totalDensity * 2);

		color = beersColor +((float3(1.0f, 1.0f, 1.0f) - beersColor) * lightColor * mpScene.mMaterials[previousVoxelMaterial]->albedo);
	}

	return color * Trace(Ray(intersectionPoint + (ray.D * epsilon), ray.D), rayDepth);
}

Ray Renderer::Reflect(Ray& ray, uchar mMaterialID) {
	float epsilon = 0.0001f;
	float3 intersectionPoint = ray.IntersectionPoint();
	float3 Normal = ray.GetNormal();
	
	float roughness = mpScene.mMaterials[mMaterialID]->roughness;
	float3 reflection = ray.D - 2 * dot(ray.D, Normal) * Normal;
	reflection = roughness * CosineWeightedDiffuseReflection(reflection) + ((1 - roughness) * reflection);

	return Ray(intersectionPoint + (reflection * epsilon), reflection);
}

float3 Renderer::SnellsLaw(float cosAI, float n1, float n2, float3 D, float3 N) {
	float n1Divn2 = n1 / n2;
	float k = 1.0f - (n1Divn2 * n1Divn2) * (1.0f - (cosAI* cosAI));
	if (k < 0.0f) {
		//Total internal reflection
		return NULL;
	}
	else {
		return n1Divn2 * D + N * (n1Divn2 * cosAI - sqrtf(k));
	}
}

float Renderer::Fresnel(float cosAI, float cosAT, float n1, float n2) {
	float T1 = ((n1 * cosAI) - (n2 * cosAT)) / ((n1 * cosAI) + (n2 * cosAT));
	float T2 = ((n1 * cosAT) - (n2 * cosAI)) / ((n1 * cosAT) + (n2 * cosAI));
	return ((T1 * T1) + (T2 * T2)) * 0.5f;
}

float3 Renderer::BeersLaw(float3 a, float d) {
	float3 I;
	I.x = exp(-a.x * d);
	I.y = exp(-a.y * d);
	I.z = exp(-a.z * d);
	return I;
}

float Renderer::SchlickApproximation(float cosTheta, float R0) {
	//this formula is Schlick's approximation.
	return R0 + (1 - R0) * powf((1 - cosTheta), 5);
}

float3 Renderer::DirectIllumination(float3 I, float3 N) {

		int size = mLights.size();
		return mLights[Rand(size) - 0.5]->CalculateLight(I, N) * (size);


}
// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick(float deltaTime)
{
	// pixel loop
	Timer t;
	// lines are executed as OpenMP parallel tasks (disabled in DEBUG)
	mFramesCount++;
	mTotalFrames++;
	float invFrame = 1.0f / mFramesCount;

	//check for photo mode
	if (IsKeyDown(GLFW_KEY_P)) {
		if (mGameState == Pause) {
			//unpause
			mPlayer.mPlayerPlayState = Player::Walking;
			mCamera.freeCam = false;
			mCamera.camPos = mGameCameraPos;
			mCamera.camTarget = mGameCameraTarget;
			ResetAccumulator();
			mGhost->mpCurrentModel->mObjectPlayState = VoxelObject::Normal;
			mGameState = Game;
		}
		else if (mGameState == Game) {
			//pause
			mGameState = Pause;
			mGameCameraPos = mCamera.camPos;
			mGameCameraTarget = mCamera.camTarget;

			mPlayer.mPlayerPlayState = Player::Pause;
			mCamera.freeCam = true;
			mGhost->mpCurrentModel->mObjectPlayState = VoxelObject::Pause;

		}
	}

	if(mGameState == Game){
		//gameplay loop
		mPlayer.Tick(deltaTime);
		if (mGhost->mpCurrentModel->mObjectPlayState == VoxelObject::Dead) {
			//respawn ghost
			mScore++;
			mGhost->Init();
		}
		mGhost->Tick(deltaTime);
	}


	//TRACE THE RAYS!!!!!!!!111
    #pragma omp parallel for schedule(dynamic) 
	for (int y = 0; y < SCRHEIGHT; y++)
	{
		// trace a primary ray for each pixel on the line
		for (int x = 0; x < SCRWIDTH; x++)
		{
			float3 pixelColor = Trace(mCamera.GetPrimaryRay((float)x + RandomFloat(), (float)y + RandomFloat()));
			float4 pixel = float4(pixelColor, 0);
			mpAccumulator[x + y * SCRWIDTH] += pixel;
			pixel = mpAccumulator[x + y * SCRWIDTH] * invFrame;
			if (mToneMapping) pixel = ApproxAces(pixel);
			screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(&pixel);

		}
	}
	if (mGameState == Game) {
		if (mPlayer.GetUpdated()) ResetAccumulator();
		if (mGhost->GetUpdated()) ResetAccumulator();
	}

	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
	// handle user input
	if (mCamera.HandleInput(deltaTime))
	{
		if (mAutoFocus)
		{
			Ray r = mCamera.GetPrimaryRay((float)SCRWIDTH * 0.5f, (float)SCRHEIGHT * 0.5f);
			mpScene.FindNearest(r);
			if (r.voxel && r.t != 0) {
				mCamera.focalLength = r.t;
			}
		}
		ResetAccumulator();
	}
}

float3 Renderer::ApproxAces(float3 color) {
	//this approximation of the ACES tonemapping technique was not made by me. I found and read it on the following website:
	// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	color = color * 0.6f;
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;

	color = ((color * (a * color + b))) / (color * (c * color + d) + e);
	return clamp(color, 0.0f, 1.0f);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI()
{
	if (mGameState == Menu) {
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = 4.0f;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoBackground;

		ImGuiWindowFlags window_flagsDebug = ImGuiWindowFlags_None;
		// Set the window position and size
		ImVec2 window_pos(0, 0);
		ImVec2 window_size(SCRWIDTH, SCRHEIGHT);

		ImGui::SetNextWindowPos(window_pos);
		ImGui::SetNextWindowSize(window_size);

		ImGui::Begin("Main menu", nullptr, window_flags);
		ImVec2 buttonSize(200, 60);
		ImVec2 textPos((window_size.x - ImGui::CalcTextSize("Ghost hunter: Ray L. Trace").x) * 0.5f, 20.0f);
		ImGui::SetCursorPos(textPos);
		ImGui::Text("Ghost hunter: Ray L. Trace");


		//easy button
		ImVec2 buttonPos((window_size.x - buttonSize.x) * 0.5f, (window_size.y - (buttonSize.y) - 10.0f));
		// Draw the play button
		if (ImGui::IsMouseHoveringRect(buttonPos, ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y))) {
			ImGui::GetWindowDrawList()->AddRectFilled(buttonPos, ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y), IM_COL32(0, 70, 200, 255));
		}
		else {
			ImGui::GetWindowDrawList()->AddRectFilled(buttonPos, ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y), IM_COL32(0, 120, 255, 255));
		}
		ImGui::GetWindowDrawList()->AddText(buttonPos, IM_COL32(255, 255, 255, 255), "Play", nullptr);
		// Check for button click
		if (ImGui::IsMouseReleased(0) && ImGui::IsMouseHoveringRect(buttonPos, ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y))) {
			mGameState = Game;


		}

		// End the ImGui window
		ImGui::End();
	}
	else if (mGameState == Game) {
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = 2.0f;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoBackground;

		ImGuiWindowFlags window_flagsDebug = ImGuiWindowFlags_None;
		// Set the window position and size
		ImVec2 window_pos(0, 0);
		ImVec2 window_size(SCRWIDTH, SCRHEIGHT);

		ImGui::SetNextWindowPos(window_pos);
		ImGui::SetNextWindowSize(window_size);

		ImGui::Begin("General Info", nullptr, window_flags);
		ImVec2 textPos = ImVec2((2), (0));
		ImGui::SetCursorPos(textPos);
		ImGui::Text("Ghost hunter : Ray L.Trace");

		 textPos = ImVec2((2), (window_size.y - 30));
		ImGui::SetCursorPos(textPos);
		ImGui::Text("Press and hold Space to catch Ghosts!   Press P for Camera mode!");


		textPos = ImVec2((window_size.x - ImGui::CalcTextSize("Score: 100").x), (0));
		ImGui::SetCursorPos(textPos);
		ImGui::Text("Score: %i", static_cast<int>(mScore));

		ImGui::End();
	}




 ////debug ImGUI
	//ImGui::SliderInt("Place radius", &mRadius, 1, 10);
	//if (ImGui::Button("reset pos")) {
	//	Camera newCamera;
	//	mCamera = newCamera;
	//	ResetAccumulator();
	//};
	//ImGui::Checkbox("Place Blocks", &mBlockPlace);
	//if (ImGui::Checkbox("toneMapping", &mToneMapping)) {
	//	ResetAccumulator();
	//};
	//if (ImGui::CollapsingHeader("Camera Settings"))
	//{
	//	if (ImGui::Checkbox("Auto focus", &mAutoFocus)) {
	//		mCamera.aperture = mAutoFocus * 0.004f;
	//		ResetAccumulator();
	//	};
	//	char label[32];
	//	ImGui::Text("Camera Position");
	//	sprintf(label, "X %i", 1);
	//	ImGui::SetNextItemWidth(100);
	//	if (ImGui::SliderFloat(label, &mCamera.camPos.x, -4.0f, 4.0f)) ResetAccumulator();
	//	sprintf(label, "Y %i", 1);
	//	ImGui::SetNextItemWidth(100);
	//	ImGui::SameLine();
	//	if (ImGui::SliderFloat(label, &mCamera.camPos.y, -4.0f, 4.0f))ResetAccumulator();
	//	sprintf(label, "Z %i", 1);
	//	ImGui::SetNextItemWidth(100);
	//	ImGui::SameLine();
	//	if (ImGui::SliderFloat(label, &mCamera.camPos.z, -4.0f, 4.0f)) ResetAccumulator();


	//	if (ImGui::SliderFloat("FOV", &mCamera.fov, 0.0f, 5.0f)) mCamera.CalculateFrustum(), ResetAccumulator();
	//	if (ImGui::SliderFloat("FocalLength", &mCamera.focalLength, 0, 2))mCamera.CalculateFrustum(), ResetAccumulator();
	//	if (ImGui::SliderFloat("Aperture", &mCamera.aperture, 0, 0.05f))mCamera.CalculateFrustum(), ResetAccumulator();
	//}
	//for each (Material * mMaterial in mpScene.mMaterials)
	//{
	//	if (mMaterial->UI())ResetAccumulator();
	//}
	//for each (Light * mLight in mLights)
	//{
	//	if (mLight->LightUI())ResetAccumulator();
	//}

	//if (mPlayer.UI())ResetAccumulator();

 // 1
}

// -----------------------------------------------------------
// controls
//// -----------------------------------------------------------
//void Renderer::MouseDown(int button) {
//	Ray r = camera.GetPrimaryRay((float)mousePos.x, (float)mousePos.y);
//	scene.FindNearest(r);
//	if (r.voxel) {
//		if (blockPlace) {
//			if (button == GLFW_MOUSE_BUTTON_1) {
//				scene.mVoxelObject.SetCircle(r, radius, r.voxel);
//			}
//			else if (button == GLFW_MOUSE_BUTTON_2) {
//				scene.mVoxelObject.SetCircle(r, radius, 0);
//			}
//		}
//		else {
//			//set focalLength
//			camera.focalLength = r.t;
//			camera.CalculateFrustum();
//		}
//		ResetAccumulator();
//	}
//
//
//}
//


// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Renderer::Shutdown()
{
	// save current camera
	FILE* f = fopen("camera.bin", "wb");
	fwrite(&mCamera, 1, sizeof(Camera), f);
	fclose(f);
}