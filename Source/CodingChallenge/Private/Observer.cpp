// Fill out your copyright notice in the Description page of Project Settings.


#include "Observer.h"
#include "Minion.h"

AObserver::AObserver()
{ 
	PrimaryActorTick.bCanEverTick = true;
}

void AObserver::BeginPlay()
{
	Super::BeginPlay();
	
	CreateGrid();
	SpawnMinions();

	InitMinions();

	SimulateBattle();
}

void AObserver::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bStartSimulation || bGameOver) return;

	CurrentStep += DeltaTime;

	if (CurrentStep >= TimeStep)
	{
		for (auto const& i : MinionsInfo)
		{
			if (bGameOver) break;
			TimeSteps[i.TeamID]++;

			if (SimulationsIndex[i.TeamID] < MinionsSimulations[i.TeamID].Num())
			{
				if (MinionsSimulations[i.TeamID][SimulationsIndex[i.TeamID]] == EAction::Move && TimeSteps[i.TeamID] == 1)
				{
					i.Minion->MoveTo(Grid[i.Movement[SimulationsIndex[i.TeamID]]]);
					TimeSteps[i.TeamID] = 0;
					SimulationsIndex[i.TeamID]++;
				}
				else if ((MinionsSimulations[i.TeamID][SimulationsIndex[i.TeamID]] == EAction::Attack ||
					MinionsSimulations[i.TeamID][SimulationsIndex[i.TeamID]] == EAction::TakeDamage) && TimeSteps[i.TeamID] == 2)
				{
					MinionsSimulations[i.TeamID][SimulationsIndex[i.TeamID]] == EAction::Attack ? i.Minion->Attack() : i.Minion->TakeDamage();
					TimeSteps[i.TeamID] = 0;
					SimulationsIndex[i.TeamID]++;
				}
				else if (MinionsSimulations[i.TeamID][SimulationsIndex[i.TeamID]] == EAction::Die)
				{
					i.Minion->Die();
					bGameOver = true;
				}

			}
		}
		CurrentStep = 0;
	}
}

void AObserver::CreateGrid()
{
	Grid.AddDefaulted(GridX * GridY);
	GridGraph.AddDefaulted(GridX * GridY);

	const int32 startPosX = -(GridX / 2) * GridSquareSize;
	int32 posY = (GridY / 2) * GridSquareSize;

	for (int32 i = 0; i < Grid.Num(); i++)
	{
		int32 posX = (i % GridX) * GridSquareSize + startPosX;

		if (i % GridX == 0 && i != 0) posY -= GridSquareSize;

		Grid[i] = FVector2D(posX, posY);

		//Create the graph
		//left
		if (i % GridX)
			GridGraph[i].Add(i - 1);
		//right
		if ((i + 1) % GridX)
			GridGraph[i].Add(i + 1);
		//up
		if (i >= GridX)
			GridGraph[i].Add(i - GridX);
		//down
		if (i < GridX * (GridY - 1))
			GridGraph[i].Add(i + GridX);
	}
}

void AObserver::SpawnMinions()
{
	UWorld* world = GetWorld();
	if (!world) return;

	for (int32 i = 0; i < NumOfMinions; i++)
	{
		FMinionInfo minionInfo;
		minionInfo.Minion = world->SpawnActor<AMinion>(MinionClass, FTransform());
		if (minionInfo.Minion) MinionsInfo.Add(minionInfo);
	}
}

void AObserver::InitMinions()
{
	if (!MinionsInfo.Num()) return;

	TArray<int32> usedPos;
	usedPos.Init(-1, MinionsInfo.Num());

	for (int i = 0; i < MinionsInfo.Num(); i++)
	{
		MinionsInfo[i].TeamID = i % 2;

		int32 startPos;
		do startPos = FMath::RandRange(0, Grid.Num() - 1);
		while (usedPos.Contains(startPos));
		usedPos.Add(startPos);
		MinionsInfo[i].Minion->SetActorLocation(FVector(Grid[startPos].X, Grid[startPos].Y, GridSquareSize / 2));
		MinionsInfo[i].GridPosition = startPos;

		MinionsInfo[i].HitPoints = FMath::RandRange(2, 5);

		MinionsInfo[i].bDead = false;

		MinionsInfo[i].Minion->Init(TimeStep, MovesPerTimeStep, AttackTimeSteps, MinionsInfo[i].TeamID);
	}
}

void AObserver::SimulateBattle()
{
	if (!MinionsInfo.Num()) return;
	
	TArray<int32> path;
	MinionsInfo[0].Opponent = GetPathToOpponent(MinionsInfo[0], path);
	MinionsInfo[1].Opponent = 0;
	FColor color;
	MinionsInfo[0].TeamID ? color = FColor::Blue : color = FColor::Red;

	MinionsSimulations.AddDefaulted(MinionsInfo.Num());
	SimulationsIndex.Init(0, MinionsInfo.Num());
	TimeSteps.Init(0, MinionsInfo.Num());

	bool bSimulate = true;

	int32 pathPos = 0;

	while (bSimulate)
	{
		for (auto& i : MinionsInfo)
		{
			if (i.HitPoints <= 0) i.bDead = true;

			if (i.bDead)
			{
				bSimulate = false;
				MinionsSimulations[i.TeamID].Add(EAction::Die);
				break;
			}

			if ((path.Num() - pathPos * 2) <= AttackDistance)
			{
				MinionsSimulations[i.TeamID].Add(EAction::Attack);
				MinionsSimulations[i.Opponent].Add(EAction::TakeDamage);
				MinionsInfo[i.Opponent].HitPoints--;
				continue;
			}

			int32 newPos;
			newPos = i.TeamID ? path[pathPos] : path[path.Num() - 1 - pathPos];
			i.Movement.Add(newPos);
			MinionsSimulations[i.TeamID].Add(EAction::Move);
		}
		pathPos++;
	}

	bStartSimulation = true;
}

//Looking for the closest opponent
int32 AObserver::GetPathToOpponent(const FMinionInfo& minionInfo, TArray<int32>& outPath)
{
	TArray<int32> parents;
	parents.Init(-1, GridGraph.Num());

	for (auto const& i : MinionsInfo)
	{
		if (!i.bDead && i.Minion != minionInfo.Minion && i.TeamID != minionInfo.TeamID)
		{
			BFS(minionInfo.GridPosition, i.GridPosition, parents);

			int32 node = i.GridPosition;
			outPath.Add(node);

			while (parents[node] != -1)
			{
				outPath.Add(parents[node]);
				node = parents[node];
			}

			return i.TeamID;
		}
	}

	return -1;
}

void AObserver::BFS(const int32 minionPos, const int32 oponnentPos, TArray<int32>& outParents)
{
	TQueue<int32> queue;
	queue.Enqueue(minionPos);

	TArray<bool> visited;
	visited.Init(false, outParents.Num());

	while (!queue.IsEmpty())
	{
		int32 node; 
		queue.Dequeue(node);

		if (node == oponnentPos) break;

		visited[node] = true;

		for (auto const i : GridGraph[node])
		{
			if (visited[i]) continue;

			outParents[i] = node;
			queue.Enqueue(i);
		}
	}
}
