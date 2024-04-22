// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Observer.generated.h"

class AMinion;

enum class EAction
{
	Move,
	Attack,
	TakeDamage,
	Die
};

USTRUCT()
struct FMinionInfo
{
	GENERATED_BODY()

	int32 TeamID;
	int32 GridPosition;
	int32 HitPoints;
	bool bDead;
	TArray<int32> Movement;
	int32 Opponent;

	UPROPERTY()
	AMinion* Minion = nullptr;
};

UCLASS()
class CODINGCHALLENGE_API AObserver : public APawn
{
	GENERATED_BODY()

public:
	AObserver();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	void CreateGrid();
	void SpawnMinions();

	void InitMinions();

	void SimulateBattle();

	int32 GetPathToOpponent(const FMinionInfo& minionInfo, TArray<int32>& outPath);

	void BFS(const int32 minionPos, const int32 oponnentPos, TArray<int32>& outParents);

protected:
	TArray<FVector2D> Grid;
	TArray<TArray<int32>> GridGraph;

	UPROPERTY(EditDefaultsOnly)
	int32 GridX = 20;
	UPROPERTY(EditDefaultsOnly)
	int32 GridY = 20;

	UPROPERTY(EditDefaultsOnly)
	float GridSquareSize = 10.f;

	UPROPERTY(EditDefaultsOnly)
	int32 NumOfMinions = 2;

	UPROPERTY(EditDefaultsOnly)
	float TimeStep = 1.f;

	UPROPERTY(EditDefaultsOnly)
	int32 MovesPerTimeStep = 1;

	UPROPERTY(EditDefaultsOnly)
	int32 AttackTimeSteps = 2;

	UPROPERTY(EditDefaultsOnly)
	int32 AttackDistance = 2;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMinion> MinionClass;

	UPROPERTY()
	TArray<FMinionInfo> MinionsInfo;

	TArray<TArray<EAction>> MinionsSimulations;
	TArray<int32> SimulationsIndex;

	float CurrentStep = 0.f;
	TArray<int32> TimeSteps;
	bool bGameOver = false;
	bool bStartSimulation = false;
};
