#include "Collider.h"

Collider::Collider(float height, float radius, XMFLOAT3 offset, bool pushable) :
	height_(height),
	radius_(radius),
	offset_(offset),
	pushable_(pushable)
{
}

bool Collider::IsIntersecting(const Collider& other, CollisionInfo& info) const
{
	XMFLOAT3 topPoint =			GetTopPoint();
	XMFLOAT3 bottomPoint =		GetBottomPoint();
	XMFLOAT3 otherTopPoint =	other.GetTopPoint();
	XMFLOAT3 otherBottomPoint =	other.GetBottomPoint();
	float radiusSum =			other.GetRadius() + radius_;

	XMFLOAT3 intersectionPoint;
	XMFLOAT3 otherIntersectionPoint;

	float dist;
	if (otherBottomPoint.y > topPoint.y) {
		// the body of the other collider is above us,
		// do a sphere test between our closest points.
		intersectionPoint = topPoint;
		otherIntersectionPoint = otherBottomPoint;
	}
	else if (bottomPoint.y > otherTopPoint.y) {
		// same as above, but switched around.
		intersectionPoint = bottomPoint;
		otherIntersectionPoint = otherTopPoint;
	}
	else {
		otherIntersectionPoint = otherBottomPoint;
		intersectionPoint = topPoint;

		// intersect on a plane
		float averageY = (intersectionPoint.y + otherIntersectionPoint.y) / 2.0f;
		intersectionPoint.y = averageY;
		otherIntersectionPoint.y = averageY;
	}

	dist = GetDistance(intersectionPoint, otherIntersectionPoint);

	bool colliding = (dist < radiusSum);
	if (!colliding) return false;

	// if we're colliding, gather more info
	XMVECTOR difference = XMVectorSet(
		intersectionPoint.x - otherIntersectionPoint.x,
		intersectionPoint.y - otherIntersectionPoint.y,
		intersectionPoint.z - otherIntersectionPoint.z,
		0.0f
	);

	XMVector4Normalize(difference);
	difference *= (radiusSum - dist) * 0.25f;

	XMStoreFloat3(&info.offset, difference);

	return true;
}

XMFLOAT3 Collider::GetTopPoint() const
{
	return XMFLOAT3(
		worldPosition_.x + offset_.x,
		worldPosition_.y + offset_.y + height_,
		worldPosition_.z + offset_.z
	);
}

XMFLOAT3 Collider::GetBottomPoint() const
{
	return XMFLOAT3(
		worldPosition_.x + offset_.x,
		worldPosition_.y + offset_.y,
		worldPosition_.z + offset_.z
	);
}

float Collider::GetDistance(XMFLOAT3 pointOne, XMFLOAT3 pointTwo) const
{
	float dx = pointOne.x - pointTwo.x;
	float dy = pointOne.y - pointTwo.y;
	float dz = pointOne.z - pointTwo.z;

	return sqrtf(dx * dx + dy * dy + dz * dz);
}
