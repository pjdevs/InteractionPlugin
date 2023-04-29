// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blueprint/UserWidget.h"
#include "InteractorComponent.generated.h"

class IInteractive;

/**
 * Delegate for executing a line sweep.
 * @param Distance The distance of the line sweep.
 * @param TraceChannel The collision channel to use.
 * @param Hits The output hit results of the line sweep.
 */
DECLARE_DELEGATE_RetVal_ThreeParams(bool, FInteractionTraceDelegate, float /*Distance*/, ECollisionChannel /*TraceChannel*/, TArray<FHitResult>& /*Hits*/)
DECLARE_DELEGATE_RetVal_OneParam(UUserWidget*, FCreateInteractionWidgetDelegate, AActor* /*Interactive*/)

/**
 * ActorComponent for handling replicated interactions with other interactive actors. 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INTERACTIONPLUGIN_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/**
	 * Distance to trace for interaction.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interaction, meta = (AllowPrivateAccess = true))
	float InteractionDistance;

	/**
	 * Collision channel to use when interaction trace. 
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = true))
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel;
	
	/**
	 * The list of all current possible interactives.
	 */
	TArray<IInteractive*> PossibleInteractives;

	/**
	 * Delegate which will execute the line sweep.
	 */
	FInteractionTraceDelegate InteractionTraceDelegate;

	/**
	* Delegate which will create the interaction widget.
	*/
	FCreateInteractionWidgetDelegate CreateInteractionWidgetDelegate;

	/**
	 * The current most relevant actor to interact with.
	 */
	UPROPERTY(VisibleInstanceOnly)
	AActor* MostRelevantActor;

	/**
	 * Store the current widget for current possible interaction.
	 */
	UPROPERTY()
	UUserWidget* InteractionWidget;
	
public:
	// Sets default values for this component's properties
	UInteractorComponent();

	/** Called for interact input */
	UFUNCTION(BlueprintCallable)
	void Interact();

	/**
	 * Add an interactive to the list of possible interactives.
	 */
	void AddInteractive(IInteractive* Interactive);

	/**
	 * Remove an interactive from the list of possible interactives.
	 */
	void RemoveInteractive(IInteractive* Interactive);

	/**
	 * Set the delegate for executing the interaction line sweep.
	 */
	void SetInteractionTraceDelegate(FInteractionTraceDelegate Delegate);
	
	/**
	 * Set the delegate for creating the interaction widget.
	 */
	void SetCreateInteractionWidgetDelegate(FCreateInteractionWidgetDelegate Delegate);

protected:
	/**
	 * Execute Interact on the server.
	 */
	UFUNCTION(Server, Reliable)
	void Server_Interact();

	/**
	 * Execute the line sweep and update most relevant actor by distance.
	 */
	void RecomputeInteractiveRelevancy();

	/**
	 * Called when a widget should be shown on clients.
	 */
	// void ShowInteractionWidget(AActor* Interactive);

	/**
	 * Called when the current widget should be hidden.
	 */
	// void HideInteractionWidget();
	
	UFUNCTION(Client, Reliable)
	void ShowInteractionWidget_Client(AActor* Interactive);
	
	UFUNCTION(Client, Reliable)
	void HideInteractionWidget_Client();
};
