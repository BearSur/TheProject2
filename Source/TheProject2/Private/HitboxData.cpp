// Fill out your copyright notice in the Description page of Project Settings.


#include "HitboxData.h"

UHitboxData::UHitboxData()
{
	HitboxType = EHitboxType::WeaponSocketSweep;
	HitboxRadius = 10.0f;
	OwnerFollowShape = EOwnerFollowHitboxShape::Sphere;
	FollowShapeOffset = FVector::ZeroVector;
	BoxHalfExtent = FVector(30.0f, 30.0f, 30.0f);
	CylinderRadius = 30.0f;
	CylinderHalfHeight = 60.0f;
}
