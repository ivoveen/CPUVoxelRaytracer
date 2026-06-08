#include "precomp.h"
#include "SpotLight.h"

float3 SpotLight::CalculateLight(float3 intersectionPoint, float3 Normal) {
	float3 lightDirection = intersectionPoint - mWorldPos;
	float distanceToLight = length(lightDirection);
	float inverseDistance = 1 / distanceToLight;
	lightDirection = lightDirection * inverseDistance;
	float cosine = dot(Normal, (mWorldPos - intersectionPoint));
	if (cosine >= 0) {
		//shadow rays
		if (!mpScene->IsOccluded(Ray(mWorldPos + (lightDirection * epsilon), lightDirection, distanceToLight - (2 * epsilon)))) {
			float coneDot = max(dot(lightDirection, mDirection), 0.0f);
			float intensity = (mIntensity / powf(distanceToLight, 2.0f)); //distance attenuation
			if (coneDot > mConeSize) {
				return (cosine * mLightColor * intensity); //final color
			}
			else if (coneDot > mSoftBandSize) {
				// lerp intensity, conedot 0.9 = 100,  0.8 = 0
				float lerpSoftShadow = (coneDot - mSoftBandSize) / (mConeSize - mSoftBandSize);
				return ((cosine * mLightColor * intensity) * lerpSoftShadow); //final color
			}
		}
	}
	return float3(0.0f, 0.0f, 0.0f);
}

bool SpotLight::LightUI() {
	bool mChanged = false;
	char label[32];
	sprintf(label, "Spot Light %i", mNumber);
	if (ImGui::CollapsingHeader(label ,mNumber))
	{
		sprintf(label, "LightIntense %i", mNumber);
			if(ImGui::SliderFloat(label, &mIntensity, 0.0f, 2.0f)) mChanged = true;
			ImGui::Text("Light Position");
		sprintf(label, "X %i", mNumber); 
		ImGui::SetNextItemWidth(100);
			if(ImGui::SliderFloat(label, &mWorldPos.x, -2.0f, 2.0f)) mChanged = true;
		sprintf(label, "Y %i", mNumber); 
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
			if(ImGui::SliderFloat(label, &mWorldPos.y, -2.0f, 2.0f)) mChanged = true;
		sprintf(label, "Z %i", mNumber); 
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
			if(ImGui::SliderFloat(label, &mWorldPos.z, -2.0f, 2.0f)) mChanged = true;
			//light direction
			ImGui::Text("Light Direction");
			sprintf(label, "dX %i", mNumber); 
		ImGui::SetNextItemWidth(100);
			if(ImGui::SliderFloat(label, &mDirection.x, -2.0f, 2.0f)) mChanged = true;
		sprintf(label, "dY %i", mNumber); 
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
			if(ImGui::SliderFloat(label, &mDirection.y, -2.0f, 2.0f)) mChanged = true;
		sprintf(label, "dZ %i", mNumber); 
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
			if(ImGui::SliderFloat(label, &mDirection.z, -2.0f, 2.0f)) mChanged = true;

			//cone shape
			ImGui::Text("Cone shape");
			sprintf(label, "Cone %i", mNumber);
			ImGui::SetNextItemWidth(100);
			if(ImGui::SliderFloat(label, &mConeSize, 0.5f, 1.0f)) mChanged = true;
			sprintf(label, "SoftBand %i", mNumber);
			ImGui::SetNextItemWidth(100);
			ImGui::SameLine();
			if(ImGui::SliderFloat(label, &mSoftBandSize, -0.3f, 1.0f)) mChanged = true;

		sprintf(label, "Color %i", mNumber); 
			if(ImGui::ColorEdit3(label, &mLightColor[0])) mChanged = true;
	}
	return mChanged;
}