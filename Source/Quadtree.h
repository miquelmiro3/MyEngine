#pragma once
#include "GameObject.h"

#include <list>

#define MAX_GAME_OBJECTS 8
#define MIN_QUADTREENODE_AREA 100.0f;

class QuadtreeNode
{
public:
	QuadtreeNode(AABB _aabb, QuadtreeNode* _parent);
	~QuadtreeNode();

	void InsertGO(GameObject* go);
	void EraseGO(GameObject* go);
	void RedistributeChildren();

	void CreateChildren();
	
	bool IsFull();
	bool IsMin();
	bool IsLeaf();

	void SetObejctsInFrustum(Plane planes[6]);
	bool Intersects(Plane planes[6], float3 cornerPoints[8]);

	void DrawAABB(int& numberOfNodes);

	AABB GetAABB() { return m_nodeAABB; };
	const QuadtreeNode* GetParent() const { return m_parent; };
	QuadtreeNode** GetChildren() { return m_children; }; 
	std::list<GameObject*> GetGameObjectsInThisNode() const { return gameObjects; };

private:
	AABB m_nodeAABB;
	QuadtreeNode* m_parent = nullptr;
	QuadtreeNode* m_children[4]; // 0 = NW, 1 = NE, 2 = SW, 3 = SE.
	std::list<GameObject*> gameObjects;
};

class Quadtree
{
public:
	Quadtree();
	~Quadtree();

	void InsertGO(GameObject* go);
	void EraseGO(GameObject* go);

	void SetBoundaries(AABB aabb);

	void SetObejctsInFrustum(Frustum* frustum);

	void DrawAABB(int& numberOfNodes);

	void Clear();

	QuadtreeNode* GetRoot() const { return m_root; };

private:
	QuadtreeNode* m_root = nullptr;
	bool m_drawAABB = false;
};