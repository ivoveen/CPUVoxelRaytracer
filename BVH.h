//#pragma once
//class BVH
//{
//public:
//	void ConstructBVH(Object* mObjects) {
//		//create index array
//		indices = new uint[N];
//		for (int i = 0; i < N; i++) indices[i] = i;
//
//		//allocate BVH root node
//		pool = new BVHNode[N * 2 - 1];
//		root = &pool[0];
//		poolPtr = 2;
//
//		//subdivide root node
//		root->leftFirst = 0;
//		root->count = N;
//		root->bounds = CalculateBounds(mObjects, root->leftFirst, root->count);
//		root->Subdivide(indices, pool, poolPtr);
//	}
//
//	aabb CalculateBounds(Object* mObjects, int first, int count) {
//
//	}
//
//	const int N = 100; //number of objects.
//	int poolPtr = 0;// pointer pointing to the next free node in the pool.
//	BVHNode* pool; //a pool of premade bvhnodes that are to be reused.
//	BVHNode* root; //the root node covering all
//	uint* indices;// indeces to a list of objects
//};
//
//
//
//
//struct BVHNode
//{
//	aabb bounds; //24
//	int leftFirst; //4
//	int count; //4, total 32 bytes
//
//	void Subdivide(uint* indices, BVHNode* pool, int poolPtr) {
//		if (count < 3) return;
//		this->leftFirst = (int)&pool[poolPtr++];
//		Partition(indices);
//		((BVHNode*)(this->leftFirst))->Subdivide(indices, pool, poolPtr++); //left
//		((BVHNode*)(this->leftFirst + 1))->Subdivide(indices, pool, poolPtr++); //right
//	}
//
//	void Partition(uint* indices) {
//		int axis = bounds.LongestAxis();
//		float splitPos = bounds.bmin[axis] + bounds.Extend(axis) * 0.5f;
//
//		//quicksort
//		int i = leftFirst;
//		int j = i + count - 1;
//		while (i <= j)
//		{
//			if (indices[i].centroid[axis] < splitPos)
//				i++;
//			else
//				swap(tri[i], tri[j--]);
//		}
//	}
//
//	void Traverse(Ray r) {
//		if (!r.Intersect(bounds)) return;
//		if (isLeaf()) {
//			IntersectObject();
//		}
//		else
//		{
//			pool[leftFirst].Traverse(r);
//			pool[leftFirst + 1].Traverse(r);
//		}
//	}
//};
