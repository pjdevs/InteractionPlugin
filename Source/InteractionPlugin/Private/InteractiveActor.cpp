// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveActor.h"
#include "Net/UnrealNetwork.h"

AInteractiveActor::AInteractiveActor()
{
	SetReplicates(true);

	InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Inetraction Trigger"));
	InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = InteractionTrigger;

	State = EInteractiveState::Ready;
	InteractMultipleTimes = true;
	InteractionWidgetClass = nullptr;
	InteractionDescription = "Interact";
}

void AInteractiveActor::HandleTriggerBeginOverlap(UPrimitiveComponent* PrimitiveComponent, AActor* Actor,
	UPrimitiveComponent* PrimitiveComponent1, int I, bool bArg, const FHitResult& HitResult)
{
	if (!Actor) return;
	auto Interactor = Cast<UInteractorComponent>(Actor->GetComponentByClass(UInteractorComponent::StaticClass()));
	if (!Interactor) return;
	PossibleInteractors.Add(Interactor);
	Interactor->AddInteractive(this);
}

void AInteractiveActor::HandleTriggerEndOverlap(UPrimitiveComponent* PrimitiveComponent, AActor* Actor,
	UPrimitiveComponent* PrimitiveComponent1, int I)
{
	if (!Actor) return;
	auto Interactor = Cast<UInteractorComponent>(Actor->GetComponentByClass(UInteractorComponent::StaticClass()));
	if (!Interactor) return;
	PossibleInteractors.Remove(Interactor);
	Interactor->RemoveInteractive(this);
}

void AInteractiveActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AInteractiveActor::HandleTriggerBeginOverlap);
	InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AInteractiveActor::HandleTriggerEndOverlap);
}

void AInteractiveActor::Interact(AActor* InteractionInstigator)
{
	if (!HasAuthority()) return;
	if (!CanBeInteracted(InteractionInstigator)) return;

	DoInteract(InteractionInstigator);
	BP_DoInteract(InteractionInstigator);
	
	if (InteractMultipleTimes)
		State = State == EInteractiveState::Interacted ? EInteractiveState::Ready : EInteractiveState::Interacted;
	else
		State = EInteractiveState::Interacted;
	
	OnRep_State();
}

bool AInteractiveActor::CanBeInteracted(AActor* InteractionInstigator)
{
	if (InteractMultipleTimes)
		return true;
	else
		return State == EInteractiveState::Ready;
}

FString AInteractiveActor::GetInteractionDescription() const
{
	return InteractionDescription;
}

void AInteractiveActor::OnRep_State()
{
	if (State == EInteractiveState::Interacted || InteractMultipleTimes)
	{
		if (!InteractMultipleTimes)
		{
			for (auto* Interactor : PossibleInteractors)
			{
				Interactor->RemoveInteractive(this);
			}
		}
		
		DoFeedback();
		BP_DoFeedback();
	}
}

void AInteractiveActor::DoInteract(AActor* InteractionInstigator)
{
}

void AInteractiveActor::DoFeedback()
{
}

void AInteractiveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractiveActor, State);
}