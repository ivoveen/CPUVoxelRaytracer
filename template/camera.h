#pragma once

// default screen resolution
#define SCRWIDTH	1024
#define SCRHEIGHT	640
#define INVSCRWIDTH	 1.0f/SCRWIDTH
#define INVSCRHEIGHT	1.0f/SCRHEIGHT
// #define FULLSCREEN
// #define DOUBLESIZE

namespace Tmpl8 {

	class Camera
	{
	public:
		Camera()
		{
			// setup a basic view frustum
			fov = 1.0f;
			aperture = 0.004f;
			focalLength = 2.0f;
			camPos = float3(4.0f, 3.294f, 4.187f);
			camTarget = float3(4.0f, 0.0f, 3.0f);
			topLeft = float3(-aspect, 1, 0);
			topRight = float3(aspect, 1, 0);
			bottomLeft = float3(-aspect, -1, 0);
		}
		Ray GetPrimaryRay(const float x, const float y)
		{
			// calculate pixel position on virtual screen plane
			const float u = (x * (1.0f * INVSCRWIDTH));
			const float v = (y * (1.0f * INVSCRHEIGHT));
			const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
			float xOffset = aperture * (RandomFloat() * 2 - 1);
			float yOffset = aperture * (RandomFloat() * 2 - 1);
			float3 rayOrigin = camPos + xOffset * right + yOffset * up;
			return Ray(rayOrigin, normalize(P - rayOrigin));
			// Note: no need to normalize primary rays in a pure voxel world
			// TODO: 
			// - if we have other primitives as well, we *do* need to normalize!
			// - there are far cooler camera models, e.g. try 'Panini projection'.
		}
		bool HandleInput(const float t)
		{

			if (!WindowHasFocus()) return false;
			float speed = 0.0025f * t;
			float3 tmpUp(0, 1, 0);
			bool mChanged = false;
			ahead = normalize(camTarget - camPos);
			right = normalize(cross(tmpUp, ahead));
			up = normalize(cross(ahead, right));
			camTarget = camPos + ahead;
			if (freeCam) {


				if (IsKeyDown(GLFW_KEY_UP)) camTarget += speed * up, mChanged = true;
				if (IsKeyDown(GLFW_KEY_DOWN)) camTarget -= speed * up, mChanged = true;
				if (IsKeyDown(GLFW_KEY_LEFT)) camTarget -= speed * right, mChanged = true;
				if (IsKeyDown(GLFW_KEY_RIGHT)) camTarget += speed * right, mChanged = true;
				ahead = normalize(camTarget - camPos);
				right = normalize(cross(tmpUp, ahead));
				up = normalize(cross(ahead, right));
				if (IsKeyDown(GLFW_KEY_A)) camPos -= speed * right, mChanged = true;
				if (IsKeyDown(GLFW_KEY_D)) camPos += speed * right, mChanged = true;
				if (GetAsyncKeyState('W')) camPos += speed * ahead, mChanged = true;
				if (IsKeyDown(GLFW_KEY_S)) camPos -= speed * ahead, mChanged = true;
				if (IsKeyDown(GLFW_KEY_R)) camPos += speed * up, mChanged = true;
				if (IsKeyDown(GLFW_KEY_F)) camPos -= speed * up, mChanged = true;
				camTarget = camPos + ahead;
			}
			ahead = normalize(camTarget - camPos);
			up = normalize(cross(ahead, right));
			right = normalize(cross(up, ahead));
			CalculateFrustum();
			if (!mChanged) return false;
			return true;
		}
		void CalculateFrustum()
		{
			topLeft = camPos + (focalLength * ahead) - (aspect * right * focalLength * fov) + (up * focalLength * fov);
			topRight = camPos + (focalLength * ahead) + (aspect * right * focalLength * fov) + (up * focalLength * fov);
			bottomLeft = camPos + (focalLength * ahead) - (aspect * right * focalLength * fov) - (up * focalLength * fov);

		}

		bool freeCam = false;
		float fov, aperture, focalLength, targetFocalLength;
		float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
		float3 camPos, camTarget;
		float3 topLeft, topRight, bottomLeft;
		float3 ahead, right, up;
	};

}