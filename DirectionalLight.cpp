#include "precomp.h"
#include "DirectionalLight.h"
#include "scene.h"
float3 DirectionalLight::CalculateLight(float3 intersectionPoint, float3 Normal) {
	float3 direction = normalize(mDirection) * -1;
	float cosine = dot(Normal, direction);	
	if (cosine > 0) {
		if (!mpScene->IsOccluded(Ray(intersectionPoint + (direction * epsilon), direction))) {
			//no distance attenuation
			return (cosine * mLightColor * mIntensity); //final color
		}
	}
	return float3(0.0f, 0.0f, 0.0f);
}

bool DirectionalLight::LightUI() {
	bool mChanged = false;
	char label[32];
	sprintf(label, "Directional Light %i", mNumber);
	if (ImGui::CollapsingHeader(label, mNumber))
	{
		sprintf(label, "LightIntense %i", mNumber);
		if(ImGui::SliderFloat(label, &mIntensity, 0.0f, 2.0f)) mChanged = true;
		ImGui::Text("Light Direction");
		sprintf(label, "X %i", mNumber);
		ImGui::SetNextItemWidth(100);
		if(ImGui::SliderFloat(label, &mDirection.x, -2.0f, 2.0f)) mChanged = true;
		sprintf(label, "Y %i", mNumber);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if(ImGui::SliderFloat(label, &mDirection.y, -2.0f, 2.0f)) mChanged = true;
		sprintf(label, "Z %i", mNumber);
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		if(ImGui::SliderFloat(label, &mDirection.z, -2.0f, 2.0f)) mChanged = true;
		sprintf(label, "Color %i", mNumber);
		if(ImGui::ColorEdit3(label, &mLightColor[0])) mChanged = true;
	}
	return mChanged;
}