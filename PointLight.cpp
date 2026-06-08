#include "precomp.h"
#include "PointLight.h"

float3 PointLight::CalculateLight(float3 intersectionPoint, float3 Normal) {
	float cosine = dot(Normal, (mWorldPos - intersectionPoint));
	if (cosine >= 0) {
		float3 lightDirection = mWorldPos - intersectionPoint;
		float distanceToLight = length(lightDirection);
		float inverseDistance = 1 / distanceToLight;
		lightDirection = lightDirection * inverseDistance;
		//shadow rays
		if (!mpScene->IsOccluded(Ray(intersectionPoint + (lightDirection * epsilon), lightDirection, distanceToLight - (2 * epsilon)))) {
			float distanceAttenuation = (inverseDistance * inverseDistance); //distance attenuation
			return (cosine * mLightColor * distanceAttenuation* mIntensity); //final color
		}
	}
	return float3(0.0f, 0.0f, 0.0f);
}

bool PointLight::LightUI() {
	bool mChanged = false;
	char label[32];
	sprintf(label, "Point Light %i", mNumber);
	if (ImGui::CollapsingHeader(label ,mNumber))
	{
		sprintf(label, "LightIntense %i", mNumber);
			if(ImGui::SliderFloat(label, &mIntensity, 0.0f, 10.0f)) mChanged = true;
			ImGui::Text("Light Position");
		sprintf(label, "X %i", mNumber); 
		ImGui::SetNextItemWidth(100);
			if(ImGui::SliderFloat(label, &mWorldPos.x, -4.0f, 20.0f)) mChanged = true;
		sprintf(label, "Y %i", mNumber); 
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
			if(ImGui::SliderFloat(label, &mWorldPos.y, -4.0f, 20.0f)) mChanged = true;
		sprintf(label, "Z %i", mNumber); 
		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
			if(ImGui::SliderFloat(label, &mWorldPos.z, -4.0f, 20.0f)) mChanged = true;
		sprintf(label, "Color %i", mNumber); 
			if(ImGui::ColorEdit3(label, &mLightColor[0])) mChanged = true;
	}
	return mChanged;
}