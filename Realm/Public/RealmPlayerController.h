#pragma once

#include "Chat.h"
#include "RealmPlayerController.generated.h"

class ARealmMoveController;
class APlayerCharacter;
class ARealmPlayerState;

UCLASS()
class ARealmPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

protected:

	/* timer that calls a movement update function on the server */
	UPROPERTY()
	FTimerHandle movementTimer;

	/* timer for getting in range for auto attacks */
	UPROPERTY()
	FTimerHandle aaRangeTimer;

	/* particle system for the move command */
	UPROPERTY()
	UParticleSystem* moveParticleSystem;

	/* move controller that handles all of the pathing for the player */
	UPROPERTY()
	ARealmMoveController* moveController;

	/* temporary until character select is implemented */
	UPROPERTY()
	TSubclassOf<APlayerCharacter> defaultCharacterClass;

	/* reference to the player character this player is using */
	UPROPERTY(replicated)
	APlayerCharacter* playerCharacter;

	/* whether or not we have the ingame store open */
	bool bIngameStoreOpen;

	/* override begin play */
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/* [CLIENT] open the player's in-game store */
	void OnOpenIngameStore();

	/* [CLIENT] open the player's in-game store */
	void OnCloseIngameStore();

	/* [CLIENT] toggle the players store */
	void OnToggleIngameStore();

	/* [CLIENT] player selects a target */
	void OnTargetSelect();

	/* [CLIENT] sets the client's view target */
	UFUNCTION(reliable, client)
	void ClientSetRTSCameraViewTarget(ASpectatorCharacter* scharacter);

public:

	/* [SERVER] calls the server to send the calculated world position to the move controller */
	UFUNCTION(reliable, server, WithValidation)
	void ServerMoveCommand(FVector targetLocation);

	/* [SERVER] start aa cycle */
	UFUNCTION(reliable, server, WithValidation)
	void ServerStartAutoAttack(AGameCharacter* target);

	/* [SERVER] clear any command timers and stop movement */
	UFUNCTION(reliable, server, WithValidation)
	void ServerClearMoveCommands();

	/* [SERVER] clear any command timers and stop auto attacks */
	UFUNCTION(reliable, server, WithValidation)
	void ServerClearAttackCommands();

	/* [SERVER] called when the player wants to use a skill */
	UFUNCTION(reliable, server, WithValidation)
	void ServerUseSkill(int32 index, const FHitResult& hitInfo);

	/* [SERVER] called when the player wants to use a mod */
	UFUNCTION(reliable, server, WithValidation)
	void ServerUseMod(int32 index, FHitResult const& hit);

	/* [SERVER] sets the chosen character class */
	UFUNCTION(reliable, server, WithValidation, BlueprintCallable, Category=Character)
	void ServerChooseCharacter(TSubclassOf<APlayerCharacter> chosenCharacter);

	/* [CLIENT] initialize the player's in-game store */
	UFUNCTION(reliable, client)
	void ClientInitIngameStore(const TArray<TSubclassOf<AMod> >& modStore);

	/* [CLIENT] open pregame screen */
	UFUNCTION(reliable, client)
	void ClientOpenPregameScreen();

	/* [CLIENT] open character select */
	UFUNCTION(reliable, client)
	void ClientOpenCharacterSelectScreen(const TArray<TSubclassOf<APlayerCharacter> >& availableCharacters);

	/* [CLIENT] initialize and show the player hud */
	UFUNCTION(reliable, client)
	void ClientOpenPlayerHUD();

	/* [CLIENT] try to buy a mod */
	UFUNCTION(reliable, server, WithValidation, BlueprintCallable, Category=Mods)
	void ServerBuyPlayerMod(TSubclassOf<AMod> wantedMod);

	/* [CLIENT] try to sell a mod */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSellPlayerMod(int32 index);

	/* [CLIENT] server needs to be sent an end game user id */
	UFUNCTION(reliable, client)
	void ClientSendEndgameUserID();

	/* [SERVER] receive an endgame userid from the client */
	UFUNCTION(reliable, server, WithValidation)
	void ServerReceiveEndgameUserID(const FString& userid);

	/* [CLIENT] opens the end game screen UI */
	UFUNCTION(reliable, client)
	void ClientOpenEndgameUI(int32 winningTeam);

	/* [SERVER] player is focused on a certain actor */
	UFUNCTION(reliable, server, WithValidation)
	void ServerLockPlayerCamera(AActor* newFocus);

	/* [SERVER] player is done focusing on a certain actor */
	UFUNCTION(reliable, server, WithValidation)
	void ServerUnlockPlayerCamera();

	/* get the hit results under this character's mouse */
	UFUNCTION(BlueprintCallable, Category = Commands)
	bool GetUnitsUnderMouse(ECollisionChannel TraceChannel, bool bTraceComplex, TArray<FHitResult>& hits) const;

	/* filter through and select one single unit under the mouse */
	UFUNCTION(BlueprintCallable, Category = Commands)
	bool SelectUnitUnderMouse(ECollisionChannel TraceChannel, bool bTraceComplex, FHitResult& chosenHit) const;

	/* gets the player character in range for auto attacks */
	void GetPlayerInAutoAttackRange();

	/* gets the class of player character this player wants to spawn with */
	UFUNCTION(BlueprintCallable, Category=PC)
	TSubclassOf<APlayerCharacter> GetDefaultCharacterClass() const;

	/* get the player character */
	UFUNCTION(BlueprintCallable, Category = PC)
	APlayerCharacter* GetPlayerCharacter() const;

	/* override this to call the move controller's possess, if it exists */
	virtual void Possess(APawn* aPawn) override;

	/* get the move controller */
	ARealmMoveController* GetMoveController() const;

	/* [CLIENT] receive data from the server about what characters we can and cant see */
	UFUNCTION(reliable, client)
	void ClientSetVisibleCharacters(const TArray<AGameCharacter*>& characters);

	/* [CLIENT] a player kill happened in the game */
	void OnDeathMessage(ARealmPlayerState* killer, ARealmPlayerState* killed, APawn* killerPawn);

	/* [CLIENT] an objective in the game was destroyed */
	void OnObjectiveDeathMessage(APawn* killerPawn, ARealmObjective* objectiveDestroyed);

	/* [SERVER] player wants to upgrade a skill */
	UFUNCTION(reliable, server, WithValidation)
	void ServerOnUpgradeSkill(int32 index);

	/* [CLIENT] send credit gain to the HUD */
	UFUNCTION(reliable, client)
	void ClientShowCreditGain(const FVector& worldLoc, int32 creditAmt);

	/* [CLIENT] receive a chat from the game mode */
	void ClientReceiveChat(const FRealmChatEntry& incomingChat);

	/* [CLIENT] player toggled their chat mode */
	void ClientToggleChat();

	/* receive a chat from a client to broadcast to everyone */
	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = Chat)
	void ServerReceiveChat(const FRealmChatEntry& broadcastChat);

	/* called when the game ends */
	void GameEnded();
};