#include "precomp.h"
#define OGT_VOX_IMPLEMENTATION

#if defined(_MSC_VER)
#include <io.h>
#endif
#include <stdio.h>

#include "ogt_vox.h"
#include<omp.h>

float3 Ray::GetNormal() 
{
	return Normal;

}
void VoxelObject::CalculateNormal(Ray& ray) {
	const float3 I1 = (ray.O + ray.t * ray.D - mWorldPos) * mInvVoxelSize; // our scene size is (1,1,1), so this scales each voxel to (1,1,1)
	const float3 fG = fracf(I1);
	const float3 d = min3(fG, 1.0f - fG);
	const float mind = min(min(d.x, d.y), d.z);
	const float3 sign = ray.Dsign * 2 - 1;
	ray.Normal = float3(mind == d.x ? sign.x : 0, mind == d.y ? sign.y : 0, mind == d.z ? sign.z : 0);
}

void Sphere::CalculateNormal(Ray& ray)
{
	ray.Normal = normalize(ray.IntersectionPoint() - mWorldPos);
	if (ray.previousVoxel) ray.Normal *= -1;
}

void Scene::Load_vox_scene(const char* filename, uint32_t scene_read_flags = 0)
{
	// open the file
#if defined(_MSC_VER) && _MSC_VER >= 1400
	FILE* fp;
	if (0 != fopen_s(&fp, filename, "rb"))
		fp = 0;
#else
	FILE* fp = fopen(filename, "rb");
#endif
	if (!fp) return;

	// get the buffer size which matches the size of the file
	fseek(fp, 0, SEEK_END);
	uint32_t buffer_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// load the file into a memory buffer
	uint8_t* buffer = new uint8_t[buffer_size];
	fread(buffer, buffer_size, 1, fp);
	fclose(fp);

	// construct the scene from the buffer
	mpScene = ogt_vox_read_scene_with_flags(buffer, buffer_size, scene_read_flags);
	// the buffer can be safely deleted once the scene is instantiated.
	delete[] buffer;
}

Scene::Scene()
{
	float invColorCorrect = 1.0f/ 255.0f;
	// load all voxel objects
	Load_vox_scene("./vox/ray-den.vox");

	mMaterials.push_back(new Material(0u, float3(0.0f, 0.0f, 0.0f), Material::diffuse, 0.0f, 1.000273f, 0.0f)); //air
	for (int i = 1; i <= 255; i++) {
		//scene->palette[scene->models[0]->voxel_data];
		ogt_vox_matl material = mpScene->materials.matl[i];
		ogt_vox_rgba color = mpScene->palette.color[i];
		float3 albedo = float3(color.r, color.g, color.b);
			Material::matType type = Material::diffuse;
			float IOR = 1.458f;
			float specularProcent = 0;
			float roughness = 0;

			switch (material.type) {
			case ogt_matl_type_diffuse:
				type = Material::diffuse;
				break;
			case ogt_matl_type_metal:
				type = Material::diffuse;
				specularProcent = material.metal;
				roughness = material.rough;
				break;
			case ogt_matl_type_glass:
				type = Material::glass;
				IOR = material.ior;
				roughness = material.rough;
				break;
			case ogt_matl_type_media:
				type = Material::smoke;
				IOR = material.ior;
				roughness = material.rough;
				break;
			}
			if (color.r != 75) {
				IOR = 1.547f;
			}
			mMaterials.push_back(new Material(i, albedo * invColorCorrect, type, specularProcent, IOR, roughness));
		
	}	
	CreateGameScene();
	//mObjects.push_back(new Sphere(float3(3.1f, 0.1f, 0.1f), 0.15f, 1));
	//for ( int i = 0; i < scene->num_models; i++)
	//{
	//	mObjects.push_back(new VoxelObject(float3(1.0f * i, 0.0f, 0.0f),1, scene->models[i], &mMaterials));
	//}

	
}

void Scene::CreateGameScene() {
	mObjects.push_back(new VoxelObject(float3(4.0f, 0.0f, 3.0f), 1, mpScene->models[mpScene->num_models - 1], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(10.0f, 0.0f, 3.0f), 1, mpScene->models[mpScene->num_models - 9], &mMaterials));

	//room one
	mObjects.push_back(new VoxelObject(float3(1.0f, 0.0f, 0.0f), 3, mpScene->models[7], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(4.0f, 0.0f, 0.0f), 3, mpScene->models[3], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(7.0f, 0.0f, 0.0f), 3, mpScene->models[1], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(10.0f, 0.0f, 0.0f), 3, mpScene->models[5], &mMaterials));

	mObjects.push_back(new VoxelObject(float3(1.0f, 0.0f, 6.0f), 3, mpScene->models[1], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(4.0f, 0.0f, 6.0f), 3, mpScene->models[3], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(7.0f, 0.0f, 6.0f), 3, mpScene->models[1], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(10.0f, 0.0f, 6.0f), 3, mpScene->models[1], &mMaterials));


	mObjects.push_back(new VoxelObject(float3(1.0f, 0.0f, 0.2f), 3, mpScene->models[2], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(1.0f, 0.0f, 3.2f), 3, mpScene->models[10], &mMaterials));

	mObjects.push_back(new VoxelObject(float3(12.8f, 0.0f, 0.2f), 3, mpScene->models[10], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(12.8f, 0.0f, 3.2f), 3, mpScene->models[6], &mMaterials));


	//halway 2
	mObjects.push_back(new VoxelObject(float3(1.0f, 0.0f, -3.0f), 3, mpScene->models[1], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(4.0f, 0.0f, -3.0f), 3, mpScene->models[9], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(7.0f, 0.0f, -3.0f), 3, mpScene->models[1], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(10.0f, 0.0f, -3.0f), 3, mpScene->models[9], &mMaterials));

	mObjects.push_back(new VoxelObject(float3(1.0f, 0.0f, -3.0f), 3, mpScene->models[10], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(12.8f, 0.0f, -2.8f), 3, mpScene->models[8], &mMaterials));


	//room 3
	mObjects.push_back(new VoxelObject(float3(13.0f, 0.0f, -3.0f), 3, mpScene->models[1], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(16.0f, 0.0f, -3.0f), 3, mpScene->models[3], &mMaterials));

	mObjects.push_back(new VoxelObject(float3(13.0f, 0.0f, 6.0f), 3, mpScene->models[1], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(16.0f, 0.0f, 6.0f), 3, mpScene->models[9], &mMaterials));

	mObjects.push_back(new VoxelObject(float3(18.8f, 0.0f, -2.8f), 3, mpScene->models[2], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(18.8f, 0.0f, 0.2f), 3, mpScene->models[10], &mMaterials));
	mObjects.push_back(new VoxelObject(float3(18.8f, 0.0f, 3.2f), 3, mpScene->models[2], &mMaterials));


	mObjects.push_back(new VoxelObject(float3(-5.0f, -30.0f, -10.0f), 30, mpScene->models[0], &mMaterials));
}

Scene::~Scene() {
	for (int i = 0; i <  mObjects.size(); i++)
	{
		delete mObjects[i];
	}
}

void VoxelObject::SetScale(float mScale) {
	this->mScale = mScale;
	mCube = Cube( mWorldPos, mCubeSize * mScale);
	mVoxelSize = (mScale / mLongestAxis);
	mInvVoxelSize = 1.0f / mVoxelSize;
}

void VoxelObject::SetPosition(float3 pos) {
	mCube = Cube(pos, mCubeSize * mScale);
	mWorldPos = pos;
}

VoxelObject::VoxelObject(float3 pos, float mScale, const ogt_vox_model* mpModel, vector<Material*>* mMaterials) {
	mpMaterials = mMaterials;
	this->mpModel = mpModel;
	this->mScale = mScale;
	
	mDimensions = int3(mpModel->size_x, mpModel->size_z, mpModel->size_y);
	
	mLongestAxis = mpModel->size_x > mpModel->size_y ? mpModel->size_x : mpModel->size_y;
	mLongestAxis = mLongestAxis > mpModel->size_z ? mLongestAxis : mpModel->size_z;

	mObjectSize = mDimensions.x; // power of 2. Warning: max 512 for a 512x512x512x4 bytes = 512MB world!
	mObjectSize2 = mDimensions.x * mDimensions.y;
	mObjectSize3 = mDimensions.x * mDimensions.y * mDimensions.z;
	mVoxelSize = (mScale / mLongestAxis);
	mInvVoxelSize = 1.0f / mVoxelSize;
	mCubeSize = make_float3(mDimensions) / mLongestAxis;

	SetPosition(pos);
	mGrid = (uchar*)MALLOC64(mObjectSize3 * sizeof(uchar));
    memset(mGrid, 0, mObjectSize3 * sizeof(uchar));	

	StoreGrid();

	//for (int z = 1; z < WORLDSIZE  - 2; z++)
	//{
	//	const float fz = (float)z / WORLDSIZE;
	//	for (int y = 1; y < WORLDSIZE - 2; y++)
	//	{
	//		const float fy = (float)y / WORLDSIZE;
	//		float fx = 0;
	//		for (int x = 1; x < WORLDSIZE - 2; x++, fx += 1.0f / WORLDSIZE)
	//		{
	//			const float n = noise3D(fx, fy, fz);
	//			//float sp = RandomFloat();
	//			//uchar materialID = 1;
	//			//uint voxelData = (materialID << 24) | (r << 16) | (g << 8) | b;

	//			Set(x, y, z, n > 0.09f ? 1 : 0);
	//		}
	//	}
	//}
}

void VoxelObject::StoreGrid() {
	mObjectPlayState = Normal;
	for (int z = 0; z < mpModel->size_z; z++)
	{
		for (int y = 0; y < mpModel->size_y; y++)
		{
			for (int x = 0; x < mpModel->size_x; x++)
			{
				int voxel_index = x + (y * mpModel->size_x) + (z * mpModel->size_x * mpModel->size_y);
				uint8_t color_index = mpModel->voxel_data[voxel_index];
				if (color_index != 0) {
					Set(x, z, y, color_index);

				}
			}
		}
	}
}

Cube::Cube(const float3 pos, const float3 size)
{
	// set cube bounds
	b[0] = pos;
	b[1] = pos + size;
}

float Cube::Intersect(const Ray& ray) const
{
	// test if the ray intersects the cube
	const int signx = ray.D.x < 0, signy = ray.D.y < 0, signz = ray.D.z < 0;
	float tmin = (b[signx].x - ray.O.x) * ray.rD.x;
	float tmax = (b[1 - signx].x - ray.O.x) * ray.rD.x;
	const float tymin = (b[signy].y - ray.O.y) * ray.rD.y;
	const float tymax = (b[1 - signy].y - ray.O.y) * ray.rD.y;
	if (tmin > tymax || tymin > tmax) goto miss;
	tmin = max(tmin, tymin), tmax = min(tmax, tymax);
	const float tzmin = (b[signz].z - ray.O.z) * ray.rD.z;
	const float tzmax = (b[1 - signz].z - ray.O.z) * ray.rD.z;
	if (tmin > tzmax || tzmin > tmax) goto miss; // yeah c has 'goto' ;)
	if ((tmin = max(tmin, tzmin)) > 0) return tmin;
miss:
	return 1e34f;
}

bool Cube::Contains(const float3& pos) const
{
	// test if pos is inside the cube
	return pos.x >= b[0].x && pos.y >= b[0].y && pos.z >= b[0].z &&
		pos.x <= b[1].x && pos.y <= b[1].y && pos.z <= b[1].z;
}


void VoxelObject::Set(const uint x, const uint y, const uint z, const uchar v)
{
	mGrid[x + y * mObjectSize + z * mObjectSize2] = v;
}
void VoxelObject::SetCircle(float3 p, int mRadius, const uchar v)
{
	p = float3(floor(p.x), floor(p.y), floor(p.z));

	for (int z = -mRadius; z < mRadius; z++) {
		for (int y = -mRadius; y < mRadius; y++) {
			for (int x = -mRadius; x < mRadius; x++) {
				if( (x*x) + (y*y) + (z*z) < mRadius * mRadius ){
					if (p.x + x < mDimensions.x && p.x + x >= 0
						&& p.y + y < mDimensions.y && p.y + y >= 0
						&& p.z + z < mDimensions.x && p.z + z >= 0) {
						Set(p.x + x, p.y + y, p.z + z, v);
					}			
				}
			}
		}
	}
}

void VoxelObject::SetCircle(Ray mouseRay, int mRadius, const uchar v)
{
	float3 p = ((mouseRay.O * mObjectSize) + (mouseRay.t * mObjectSize) * mouseRay.D);
	if (!v) {
		//erase
		p += (0.5f * mouseRay.D);
	}
	else {
		//place
		p -= (0.5f * mouseRay.D);
	}
	for (int z = -mRadius; z < mRadius; z++) {
		for (int y = -mRadius; y < mRadius; y++) {
			for (int x = -mRadius; x < mRadius; x++) {
				if ((x * x) + (y * y) + (z * z) < mRadius * mRadius) {
					if (!v) {
						//erase
						Set(p.x + x, p.y + y, p.z + z, v);
					}
					else {
						//place
						Set(p.x + x, p.y + y, p.z + z, v);
					}
				}

			}
		}
	}
}
void VoxelObject::SetCube(Ray mouseRay, int size, const uchar v)
{
	float3 p = ((mouseRay.O * mObjectSize) + (mouseRay.t * mObjectSize) * mouseRay.D);

	for (int z = -size * 0.5f; z <= size * 0.5f; z++) {
		for (int y = -size * 0.5f; y <= size * 0.5f; y++) {
			for (int x = -size * 0.5f; x <= size * 0.5f; x++) {
				if (!v) {
					//erase
					p += (0.5f * mouseRay.D);
					Set(p.x + x, p.y + y, p.z + z, v);
				}
				else {
					////place
					p -= (0.5f * mouseRay.D);
					Set(p.x + x, p.y + y, p.z + z, v);
				}
			}
		}
	}
}

bool VoxelObject::Setup3DDDA(const Ray& ray, DDAState& state) const
{
	// if ray is not inside the world: advance until it is
	state.t = 0;
	if (!mCube.Contains(ray.O))
	{
		state.t = mCube.Intersect(ray);
		if (state.t > 1e33f) return false; // ray misses voxel data entirely
	}
	// setup amanatides & woo - DONT assume world is 1x1x1, from (0,0,0) to (1,1,1)
	
	state.step = make_int3(1 - ray.Dsign * 2);
	const float3 posInGrid = mInvVoxelSize * ((ray.O + (state.t + 0.00005f) * ray.D) - mWorldPos);
	const float3 gridPlanes = (ceilf(posInGrid) - ray.Dsign) * mVoxelSize;
	const int3 P = int3(
		clamp(make_int3(posInGrid).x, 0, mDimensions.x - 1),
		clamp(make_int3(posInGrid).y, 0, mDimensions.y - 1),
		clamp(make_int3(posInGrid).z, 0, mDimensions.z - 1)
	);
				

	state.X = P.x, state.Y = P.y, state.Z = P.z;
	state.tdelta = mVoxelSize * float3(state.step) * ray.rD;
	state.tmax = (gridPlanes - ray.O + mCube.b[0]) * ray.rD;
	// proceed with traversal
	return true;
}

void Scene::FindNearest(Ray& ray)
{
	for (int i = 0; i < mObjects.size(); i++)
	{
		mObjects[i]->FindNearest(ray);
	}
}

void VoxelObject::FindNearest(Ray& ray)
{
	// setup Amanatides & Woo grid traversal
	DDAState s, bs;
	if (!Setup3DDDA(ray, s)) return;

	//glass needs to know if my previous voxel was made of glass, so it can refract when it enters air again.
	//ray.previousVoxel = 0;
	if (s.t == 0) {
		//ray origin is inside voxel grid
		float3 origin = (ray.O - mWorldPos) * mInvVoxelSize;
		ray.previousVoxel = mGrid[static_cast<int>(floor(origin.x)) + (static_cast<int>(floor(origin.y)) * mObjectSize) + (static_cast<int>(floor(origin.z)) * mObjectSize2)];
	}
	// start stepping
	while (1)
	{
		const uchar cell = mGrid[s.X + s.Y * mObjectSize + s.Z * mObjectSize2];
		if (cell != ray.previousVoxel)
		{
			//we hit something
			if (s.t < ray.t) {
				//the hit is closer than all previously found hits
				ray.t = s.t;
				ray.voxel = cell;
				ray.nearestVoxelObject = (uint*)this;
				CalculateNormal(ray);
				
			}
			break;
		}
		if (s.tmax.x < s.tmax.y)
		{
			if (s.tmax.x < s.tmax.z) { s.t = s.tmax.x, s.X += s.step.x; if (s.X >= mDimensions.x) break; s.tmax.x += s.tdelta.x; }
			else { s.t = s.tmax.z, s.Z += s.step.z; if (s.Z >= mDimensions.z) break; s.tmax.z += s.tdelta.z; }
		}
		else
		{
			if (s.tmax.y < s.tmax.z) { s.t = s.tmax.y, s.Y += s.step.y; if (s.Y >= mDimensions.y) break; s.tmax.y += s.tdelta.y; }
			else { s.t = s.tmax.z, s.Z += s.step.z; if (s.Z >= mDimensions.z) break; s.tmax.z += s.tdelta.z; }
		}
	}

	//ray.O -= worldPos;
	// TODO:
	// - A nested grid will let rays skip empty space much faster.
	// - Coherent rays can traverse the grid faster together.
	// - Perhaps s.X / s.Y / s.Z (the integer grid coordinates) can be stored in a single uint?
	// - Loop-unrolling may speed up the while loop.
	// - This code can be ported to GPU.
}

//This sphere intersection code is partly made by me. 
//I understand all of it however, and improved upon it to fit my use.
//   https://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection
void Sphere::FindNearest(Ray& ray)
{
	// get intersection point.
	float3 oc = ray.O - mWorldPos;
	float a = dot(ray.D, ray.D);
	float b = 2.0 * dot(oc, ray.D);
	float c = dot(oc, oc) - mRadius * mRadius;
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0) return; //no intersection with sphere
	float inv2a = 1 / (2 * a);
	float sqrtfDiscriminant = sqrtf(discriminant);
	float t1 = (-b + sqrtfDiscriminant) * inv2a;
	float t2 = (-b - sqrtfDiscriminant) * inv2a;

	//get closest valid intersection point.
	if (t1 < 0 && t2 < 0) return;
	float closestT = (t1 < t2 || t2 < 0) ? t1 : t2;
	if (closestT > ray.t) return;
	ray.t = closestT;

	if (t1 < 0 || t2 < 0) {
		//hitting air from inside the sphere
		ray.previousVoxel = mMaterialID;
		ray.voxel = 0;
	}
	else {
		//hitting sphere from the air
		ray.voxel = mMaterialID;
	}
	CalculateNormal(ray);
	return;
}


bool Scene::IsOccluded(const Ray& ray)
{
	for (int i = 0; i < mObjects.size(); i++)
	{
   		if(mObjects[i]->IsOccluded(ray))return true;
	}
	return false;
}

bool VoxelObject::IsOccluded(const Ray& ray) const
{
	// setup Amanatides & Woo grid traversal
	DDAState s, bs;
	if (!Setup3DDDA(ray, s)) return false;
	// start stepping
	while (s.t < ray.t)
	{
		const uchar cell = mGrid[s.X + s.Y * mObjectSize + s.Z * mObjectSize2];
		if (cell) {
			/* we hit a solid voxel */ 
			if(mpMaterials->at(cell)->type != Material::glass) return true;
		}
		if (s.tmax.x < s.tmax.y)
		{
			if (s.tmax.x < s.tmax.z) { if ((s.X += s.step.x) >= mDimensions.x) return false; s.t = s.tmax.x, s.tmax.x += s.tdelta.x; }
			else { if ((s.Z += s.step.z) >= mDimensions.z) return false; s.t = s.tmax.z, s.tmax.z += s.tdelta.z; }
		}
		else
		{
			if (s.tmax.y < s.tmax.z) { if ((s.Y += s.step.y) >= mDimensions.y) return false; s.t = s.tmax.y, s.tmax.y += s.tdelta.y; }
			else { if ((s.Z += s.step.z) >= mDimensions.z) return false; s.t = s.tmax.z, s.tmax.z += s.tdelta.z; }
		}
	}
	return false;
}


//This sphere intersection code is not made by me. 
//I understand all of it however, I found it at:
//   https://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection
bool Sphere::IsOccluded(const Ray& ray) const
{
	// test if the ray intersects the sphere
	float3 oc = ray.O - mWorldPos;
	float a = dot(ray.D, ray.D);
	float b = 2.0 * dot(oc, ray.D);
	float c = dot(oc, oc) - mRadius * mRadius;
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0) return false; //definitely no collision
	float inv2a = 1 / (2 * a);
	float sqrtfDiscriminant = sqrtf(discriminant);
	float t1 = (-b + sqrtfDiscriminant) * inv2a;
	float t2 = (-b - sqrtfDiscriminant) * inv2a;
	if (t1 < 0 && t2 < 0) return false;//maybe collision is behind origin
	return (t1 < ray.t || t2 < ray.t);
}