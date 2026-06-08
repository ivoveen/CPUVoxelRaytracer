#pragma once
#include "ogt_vox.h"



namespace Tmpl8 {


class Ray
{
public:
	Ray() = default;
	Ray( const float3 origin, const float3 direction, const float rayLength = 1e34f, const int rgb = 0 )
		: O( origin ), D( direction ), t( rayLength ), voxel( rgb )
	{
		// calculate reciprocal ray direction for triangles and AABBs
		// TODO: prevent NaNs - or don't
		rD = float3( 1 / D.x, 1 / D.y, 1 / D.z );

		uint signx = (*(uint*)&D.x) >> 31;
		uint signy = (*(uint*)&D.y) >> 31;
		uint signz = (*(uint*)&D.z) >> 31;
		Dsign = (float3( (float)signx * 2 -1, (float)signy * 2 - 1, (float)signz * 2 - 1) + 1) * 0.5f;


		//Dsign = (float3( -copysign( 1.0f, D.x ), -copysign( 1.0f, D.y ), -copysign( 1.0f, D.z ) ) + 1) * 0.5f;
	}
	float3 IntersectionPoint() const { return O + t * D; }
	float3 GetNormal();

	// ray data
	float3 O;					// ray origin
	float3 rD;					// reciprocal ray direction
	float3 D = float3( 0 );		// ray direction
	float t = 1e34f;			// ray length
	float tGlass = 0;
	float3 Dsign = float3( 1 );	// inverted ray direction signs, -1 or 1
	uchar voxel = 0;				// 32-bit ARGB color of a voxelhit object index; 0 = NONE
	uint* nearestVoxelObject = nullptr;
	uchar previousVoxel = 0;
	float3 Normal = 0;

};

class Cube
{
public:
	Cube() = default;
	Cube( const float3 pos, const float3 size );
	Cube(const float3 pos, const float size_x, const float size_y, const float size_z);
	float Intersect( const Ray& ray ) const;
	bool Contains( const float3& pos ) const;
	float3 b[2];
};



struct Material {
public:
	enum matType {
		diffuse,
		glass,
		smoke
	} type;
	const char* matTypes[3] = { "diffuse", "glass", "smoke" };

	Material(uint number, float3 albedo, matType type, float specularProcent, float refractiveIndex, float roughness)
		: number(number),
		albedo(albedo),
		type(type),
		refractiveIndex(refractiveIndex),
		reflectionNormalIncidence(specularProcent),
		roughness(roughness) {}

	uint number;
	float3 albedo;
	float refractiveIndex;
	float reflectionNormalIncidence;
	float roughness;

	bool UI() {
		char label[32];
		bool mChanged = false;
		sprintf(label, "Material %i", number);
		if (ImGui::CollapsingHeader(label, number))
		{
			sprintf(label, "Material Type %i", number);
			if (ImGui::Combo(label, (int*)&type, matTypes, 3)) mChanged = true;
			sprintf(label, "refractiveindex %i", number);
			if (ImGui::SliderFloat(label, &refractiveIndex, 0.0f, 2.5f)) mChanged = true;
			sprintf(label, "Reflection %i", number);
			if (ImGui::SliderFloat(label, &reflectionNormalIncidence, 0.0f, 1.0f)) mChanged = true;
			sprintf(label, "roughness %i", number);
			if (ImGui::SliderFloat(label, &roughness, 0.0f, 1.0f)) mChanged = true;
			sprintf(label, "Color %i", number);
			if (ImGui::ColorEdit3(label, &albedo[0])) mChanged = true;
		}
		return mChanged;
	}
};

class Object {
public:
	float3 mWorldPos; //world position of the object

	virtual bool IsOccluded(const Ray& ray) const { return NULL; };
	virtual void FindNearest(Ray& ray) {};
	virtual void SetPosition(float3 pos) {};
};

class VoxelObject : public Object 
{
public:
	int3 mDimensions;
	float3 mCubeSize;
	int mLongestAxis;
	int mObjectSize = 64; // power of 2. Warning: max 512 for a 512x512x512x4 bytes = 512MB world!
	int mObjectSize2 = (mObjectSize * mObjectSize);
	int mObjectSize3 = (mObjectSize * mObjectSize * mObjectSize);
	float mVoxelSize = (1.0f / mObjectSize);
	float mInvVoxelSize;
	const ogt_vox_model* mpModel;
	float mScale, mInvScale; // scale of the object.
	struct DDAState
	{
		int3 step;				// 16 bytes
		uint X, Y, Z;			// 12 bytes
		float t;				// 4 bytes
		float3 tdelta;
		float dummy1 = 0;		// 16 bytes
		float3 tmax;
		float dummy2 = 0;		// 16 bytes, 64 bytes in total
	};
	VoxelObject(float3 pos, float mScale, const ogt_vox_model* mpModel, vector<Material*>* mMaterials);
	void SetPosition(float3 pos);
	void StoreGrid();
	void SetScale(float mScale);
	void FindNearest(Ray& ray);
	bool IsOccluded(const Ray& ray) const;
	void CalculateNormal(Ray& ray);
	void Set(const uint x, const uint y, const uint z, const uchar v);
	void SetCircle(Ray mouseRay, int Radius, const uchar v);
	void SetCircle(float3 p, int mRadius, const uchar v);
	void SetCube(Ray mouseRay, int size, const uchar v);
	uchar* mGrid;
	Cube mCube;
	vector<Material*>* mpMaterials;
	enum ObjectPlayState {
		Pause, Normal, Dead
	}mObjectPlayState = Normal;

private:
	// min3 is used in normal reconstruction.
	__inline static float3 min3(const float3& a, const float3& b)
	{
		return float3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
	}
	bool Setup3DDDA(const Ray& ray, DDAState& state) const;
};

class Sphere : public Object
{
public:
	Sphere() = default;
	Sphere(const float3 pos, const float radius, uchar materialID) : mRadius(radius), mMaterialID(materialID) { mWorldPos = pos; }
	bool IsOccluded(const Ray& ray) const;
	void SetPosition(float3 pos) {};
	void FindNearest( Ray& ray);
	void CalculateNormal(Ray& ray);
	float mRadius;
	uchar mMaterialID;
};


class Scene
{
public:

	Scene();
	~Scene();
	void CreateGameScene();
	void CreateMenuScene();

	void Load_vox_scene(const char* filename, uint32_t scene_read_flags);
	void FindNearest( Ray& ray );
	bool IsOccluded( const Ray& ray );

	const ogt_vox_scene* mpScene;
	vector<Object*> mObjects;
	vector<Material*> mMaterials;
	

};

}
