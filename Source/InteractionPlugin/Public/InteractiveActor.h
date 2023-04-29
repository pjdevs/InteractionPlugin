// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactive.h"
#include "InteractorComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/Object.h"
#include "InteractiveActor.generated.h"

UENUM()
enum class EInteractiveState : uint8
{
	Ready,
	Interacted
};

/**
 * A base interactive Actor implementing IInteractive interface.
 */
UCLASS(Abstract)
class INTERACTIONPLUGIN_API AInteractiveActor : public AActor, public IInteractive
{
	GENERATED_BODY()

protected:
	/**
	 * @brief Trigger component used for interaction.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = true))
	UBoxComponent* InteractionTrigger;

	/**
	 ** Whether this actor can be interacted one time or multiple times.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = true))
	bool InteractMultipleTimes;

	/**
	 ** Widget to that interactor will use to add to his viewport and describe the interaction.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = true))
	TSubclassOf<UUserWidget> InteractionWidgetClass;

	/**
	 ** Description of the interaction to show in the interaction widget.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = true))
	FString InteractionDescription;
	
	/**
	 * Interaction state of the actor.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Interaction, ReplicatedUsing=OnRep_State)
	EInteractiveState State;

	/**
	 * The array of possible interactors.
	 */
	UPROPERTY()
	TArray<UInteractorComponent*> PossibleInteractors;

	UFUNCTION()
	void OnRep_State();

	/**
	 * Action to execute on the server for interaction.
	 */
	virtual void DoInteract(AActor* InteractionInstigator);
	/**
	 * Action to execute on the server for interaction (Blueprint version).
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void BP_DoInteract(AActor* InteractionInstigator);

	/**
	 * Feeback to execute on the client for interaction.
	 */
	virtual void DoFeedback();
	/**
	 * Feeback to execute on the client for interaction (Blueprint version).
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void BP_DoFeedback();

	UFUNCTION()
	void HandleTriggerBeginOverlap(UPrimitiveComponent* PrimitiveComponent, AActor* Actor, UPrimitiveComponent* PrimitiveComponent1, int I, bool bArg, const FHitResult& HitResult);
	UFUNCTION()
	void HandleTriggerEndOverlap(UPrimitiveComponent* PrimitiveComponent, AActor* Actor, UPrimitiveComponent* PrimitiveComponent1, int I);

	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	AInteractiveActor();

	virtual void Interact(AActor* InteractionInstigator) override;
	virtual bool CanBeInteracted(AActor* InteractionInstigator) override;
	virtual FVector GetInteractiveLocation() const override { return GetActorLocation(); };
	UFUNCTION(BlueprintCallable)
	virtual FString GetInteractionDescription() const override;
};
