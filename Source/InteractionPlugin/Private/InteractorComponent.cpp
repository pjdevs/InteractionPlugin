// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractorComponent.h"
#include "Interactive.h"


UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionDistance = 100.0f;
	InteractionTraceChannel = ECC_Visibility;
}

void UInteractorComponent::Interact()
{	
	if (!GetOwner()->HasAuthority())
	{
		Server_Interact();
		return;
	}
	
	auto* Interactive = Cast<IInteractive>(MostRelevantActor);
	
	if (!Interactive) return;

	Interactive->Interact(GetOwner());
}

void UInteractorComponent::AddInteractive(IInteractive* Interactive)
{
	if (!Interactive) return;
	if (!Interactive->CanBeInteracted(GetOwner())) return;
	if (PossibleInteractives.Contains(Interactive)) return;

	PossibleInteractives.Add(Interactive);
	RecomputeInteractiveRelevancy();
}

void UInteractorComponent::RemoveInteractive(IInteractive* Interactive)
{
	if (!Interactive) return;
	PossibleInteractives.Remove(Interactive);
	RecomputeInteractiveRelevancy();
}

void UInteractorComponent::SetInteractionTraceDelegate(FInteractionTraceDelegate Delegate)
{
	InteractionTraceDelegate = Delegate;
}

void UInteractorComponent::SetCreateInteractionWidgetDelegate(FCreateInteractionWidgetDelegate Delegate)
{
	CreateInteractionWidgetDelegate = Delegate;
}

void UInteractorComponent::Server_Interact_Implementation()
{
	Interact();
}

void UInteractorComponent::RecomputeInteractiveRelevancy()
{
	if (!ensureMsgf(InteractionTraceDelegate.IsBound(), TEXT("InteractionLineSweepDelegate was not boud. Did you forget to call SetInteractionLineSweepDelegate?"))) return;

	const auto* PreviousMostRelevantActor = MostRelevantActor;
	MostRelevantActor = PossibleInteractives.IsEmpty() ? nullptr : Cast<AActor>(PossibleInteractives[0]);
	
	if (TArray<FHitResult> Results; InteractionTraceDelegate.Execute(InteractionDistance, InteractionTraceChannel, Results))
	{
		for (const auto& Result : Results)
		{
			if (!Result.GetActor()) continue;
			if (!Result.GetActor()->Implements<UInteractive>()) break; // Make walls etc block interaction
			const auto* InteractiveActorHit = Cast<IInteractive>(Result.GetActor());
			const bool ExistInList = PossibleInteractives.ContainsByPredicate([&](const IInteractive* Interactive)
			{
				return Interactive == InteractiveActorHit;
			});

			if (!ExistInList) break; // Take only first it into account
			MostRelevantActor = Result.GetActor();

			break; // Take only first it into account
		}
	}

	if (GetOwner()->HasAuthority() && MostRelevantActor != PreviousMostRelevantActor)
	{
		HideInteractionWidget_Client();

		if (MostRelevantActor)
		{
			ShowInteractionWidget_Client(MostRelevantActor);
		}
	}

	PossibleInteractives.Sort([&](const IInteractive& A, const IInteractive& B)
	{
		const auto OwnerLocation = GetOwner()->GetActorLocation();
		return FVector::DistSquared(OwnerLocation, A.GetInteractiveLocation())
			< FVector::DistSquared(OwnerLocation, B.GetInteractiveLocation());
	});
}

// void UInteractorComponent::ShowInteractionWidget(AActor* Interactive)
// {
// 	if (GetOwner()->HasAuthority()) return;
// 	
// 	if (!CreateInteractionWidgetDelegate.IsBound()) return;
//
// 	InteractionWidget = CreateInteractionWidgetDelegate.Execute(Interactive);
//
// 	if (InteractionWidget)
// 	{
// 		InteractionWidget->AddToViewport();
// 	}
// }
//
// void UInteractorComponent::HideInteractionWidget()
// {
// 	if (GetOwner()->HasAuthority()) return;
//
// 	if (InteractionWidget)
// 	{
// 		InteractionWidget->RemoveFromParent();
// 	}
//
// 	InteractionWidget = nullptr;
// }

void UInteractorComponent::HideInteractionWidget_Client_Implementation()
{
	if (InteractionWidget)
	{
		InteractionWidget->RemoveFromParent();
	}

	InteractionWidget = nullptr;
}

void UInteractorComponent::ShowInteractionWidget_Client_Implementation(AActor* Interactive)
{
	if (!CreateInteractionWidgetDelegate.IsBound()) return;

	InteractionWidget = CreateInteractionWidgetDelegate.Execute(Interactive);

	if (InteractionWidget)
	{
		InteractionWidget->AddToViewport();
	}
}
