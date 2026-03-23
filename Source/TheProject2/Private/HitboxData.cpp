// Fill out your copyright notice in the Description page of Project Settings.


#include "HitboxData.h"

UHitboxData::UHitboxData()
{
	HitboxType = EHitboxType::WeaponSocketSweep;
	HitboxRadius = 10.0f;
	FollowSphereOffset = FVector::ZeroVector;
}
