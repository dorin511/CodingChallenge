// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Minion.generated.h"

UCLASS()
class CODINGCHALLENGE_API AMinion : public AActor
{
	GENERATED_BODY()
	
public:	
	AMinion();

	virtual void Tick(float DeltaTime) override;

	void Init(const float timeStep, const int32 movesPerTimeStep, const int32 attackTimeSteps, const int32 teamID);

	void MoveTo(FVector2D newPos);
	void Attack();
	void Die();
	void TakeDamage();

protected:
	void ChangeColor(const int32 teamID);

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UStaticMeshComponent* SphereComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	float TimeStep = 1.f;
	int32 MovesPerTimeStep = 1;
	int32 AttackTimeSteps = 2;

	int32 TimeSteps = 0;

	bool bIsMoving = false;
	FVector MoveToLoc;

	bool bIsAttacking = false;
	bool bIsTakingDamage = false;
	float TakeDamageSpec = 0.f;

	float CurrentTimeStep = 0.f;
};
