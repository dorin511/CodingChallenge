// Fill out your copyright notice in the Description page of Project Settings.


#include "Minion.h"
#include "Kismet/KismetMathLibrary.h"

AMinion::AMinion()
{ 
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<UStaticMeshComponent>("SphereComponent");
	SetRootComponent(SphereComponent);
}

void AMinion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsMoving && !bIsAttacking && !bIsTakingDamage) return;

	CurrentTimeStep += DeltaTime;

	if (bIsMoving)
	{
		const FVector loc = UKismetMathLibrary::VLerp(GetActorLocation(), MoveToLoc, DeltaTime);
		SetActorLocation(loc);
	}

	if (bIsTakingDamage)
	{
		if (TakeDamageSpec < 1.f) TakeDamageSpec += DeltaTime * 100.f;
		else TakeDamageSpec -= DeltaTime * 100.f;

		DynamicMaterial->SetScalarParameterValue("Specular", TakeDamageSpec);

		if (TakeDamageSpec <= 0.f) bIsTakingDamage = false;
	}

	if (CurrentTimeStep >= TimeStep)
	{
		TimeSteps++;
		if (TimeSteps == 1 && bIsMoving) bIsMoving = false;

		CurrentTimeStep = 0.f;
	}

	
}

void AMinion::Init(const float timeStep, const int32 movesPerTimeStep, const int32 attackTimeSteps, const int32 teamID)
{
	TimeStep = timeStep;
	MovesPerTimeStep = movesPerTimeStep;
	AttackTimeSteps = attackTimeSteps;
	
	ChangeColor(teamID);

	if (SphereComponent) SphereComponent->SetVisibility(true);
}

void AMinion::MoveTo(FVector2D newPos)
{
	MoveToLoc = FVector(newPos.X, newPos.Y, 5.f);
	bIsMoving = true;
}

void AMinion::Attack()
{
	bIsAttacking = true;
}

void AMinion::Die()
{
	if (SphereComponent) SphereComponent->SetVisibility(false);
}

void AMinion::TakeDamage()
{
	bIsTakingDamage = true;
}

void AMinion::ChangeColor(const int32 teamID)
{
	if (!SphereComponent || !SphereComponent->GetStaticMesh()) return;

	DynamicMaterial =  UMaterialInstanceDynamic::Create(SphereComponent->GetMaterial(0), NULL);
	SphereComponent->SetMaterial(0, DynamicMaterial);

	const FColor color = teamID ? FColor::Blue : FColor::Red;
	DynamicMaterial->SetVectorParameterValue("Color", color);
}

