#pragma once
#include "DirectXCore.h"
#include <string>

struct CollisionInfo {
	XMFLOAT3 offset;
};

class Collider
{
public:
	Collider(float height, float radius, XMFLOAT3 offset, bool pushable);
	bool IsIntersecting(const Collider& other, CollisionInfo& info) const;

	inline void		SetWorldPosition(XMFLOAT3 worldPosition) { worldPosition_ = worldPosition; }
	XMFLOAT3		GetTopPoint() const;
	XMFLOAT3		GetBottomPoint() const;
	inline float	GetRadius() const { return radius_; }
	inline bool		IsPushable() const { return pushable_; }
private:
	float			GetDistance(XMFLOAT3 pointOne, XMFLOAT3 pointTwo) const;
	bool			pushable_;
	float			radius_;
	float			height_;
	XMFLOAT3		offset_;
	XMFLOAT3		worldPosition_;
};

