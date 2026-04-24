#include "Framework/Interaction/AtlasInteractorComponent.h"

#include "Framework/Interaction/AtlasInteractableInterface.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UAtlasInteractorComponent::UAtlasInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAtlasInteractorComponent::TryInteract()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (AActor* TargetActor = FindBestInteractableActor())
	{
		IAtlasInteractableInterface::Execute_Interact(TargetActor, OwnerActor);
		return true;
	}

	return false;
}

AActor* UAtlasInteractorComponent::FindBestInteractableActor() const
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World)
	{
		return nullptr;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;
	if (!ResolveInteractionViewPoint(ViewLocation, ViewDirection))
	{
		return nullptr;
	}

	const FVector TraceEnd = ViewLocation + ViewDirection * FMath::Max(InteractionDistance, 0.0f);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AtlasInteractionTrace), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	FHitResult LineHit;
	FVector SweepEnd = TraceEnd;
	if (World->LineTraceSingleByChannel(LineHit, ViewLocation, TraceEnd, InteractionTraceChannel, QueryParams))
	{
		if (AActor* DirectTarget = ResolveInteractableActorFromHit(LineHit, OwnerActor))
		{
			return DirectTarget;
		}

		SweepEnd = LineHit.ImpactPoint;
	}

	return FindBestInteractableActorBySweep(ViewLocation, SweepEnd, OwnerActor);
}

bool UAtlasInteractorComponent::ResolveInteractionViewPoint(FVector& OutLocation, FVector& OutDirection) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (bUseControllerViewPoint)
	{
		if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			if (const AController* Controller = OwnerPawn->GetController())
			{
				FRotator ViewRotation = FRotator::ZeroRotator;
				Controller->GetPlayerViewPoint(OutLocation, ViewRotation);
				OutDirection = ViewRotation.Vector();
				return true;
			}
		}
	}

	FRotator ViewRotation = FRotator::ZeroRotator;
	OwnerActor->GetActorEyesViewPoint(OutLocation, ViewRotation);
	OutDirection = ViewRotation.Vector();
	return true;
}

AActor* UAtlasInteractorComponent::ResolveInteractableActorFromHit(const FHitResult& HitResult, AActor* Interactor) const
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return nullptr;
	}

	return CanInteractWithActor(HitActor, Interactor) ? HitActor : nullptr;
}

AActor* UAtlasInteractorComponent::FindBestInteractableActorBySweep(const FVector& Start, const FVector& End, AActor* Interactor) const
{
	UWorld* World = GetWorld();
	if (!World || InteractionSweepRadius <= 0.0f)
	{
		return nullptr;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AtlasInteractionSweep), false, Interactor);
	QueryParams.AddIgnoredActor(Interactor);

	TArray<FHitResult> SweepHits;
	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(InteractionSweepRadius);
	World->SweepMultiByChannel(SweepHits, Start, End, FQuat::Identity, InteractionTraceChannel, SweepShape, QueryParams);

	const FVector TraceDirection = (End - Start).GetSafeNormal();
	float BestScore = -BIG_NUMBER;
	AActor* BestActor = nullptr;
	TSet<const AActor*> VisitedActors;

	for (const FHitResult& Hit : SweepHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || VisitedActors.Contains(HitActor))
		{
			continue;
		}

		VisitedActors.Add(HitActor);
		if (!CanInteractWithActor(HitActor, Interactor))
		{
			continue;
		}

		const FVector CandidateLocation = HitActor->GetActorLocation();
		const FVector ToCandidate = CandidateLocation - Start;
		const float Distance = ToCandidate.Size();
		if (Distance <= KINDA_SMALL_NUMBER)
		{
			BestActor = HitActor;
			break;
		}

		const float Alignment = FVector::DotProduct(ToCandidate.GetSafeNormal(), TraceDirection);
		if (Alignment <= 0.0f)
		{
			continue;
		}

		const float Score = Alignment * 10000.0f - Distance;
		if (Score > BestScore)
		{
			BestScore = Score;
			BestActor = HitActor;
		}
	}

	return BestActor;
}

bool UAtlasInteractorComponent::CanInteractWithActor(AActor* CandidateActor, AActor* Interactor) const
{
	if (!CandidateActor || !CandidateActor->GetClass()->ImplementsInterface(UAtlasInteractableInterface::StaticClass()))
	{
		return false;
	}

	return IAtlasInteractableInterface::Execute_CanInteract(CandidateActor, Interactor);
}
