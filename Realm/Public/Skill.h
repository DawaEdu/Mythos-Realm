#pragma once

#include "Skill.generated.h"

class AGameCharacter;

UENUM()
enum class ESkillState : uint8
{
	NoOwner,
	Ready,
	NotLearned,
	Disabled,
	OnCooldown,
	Performing,
	MAX
};

UENUM()
enum class ESkillInterruptReason : uint8
{
	SIR_Damaged UMETA(DisplayName = "Damaged While Performing"),
	SIR_CC UMETA(DisplayName = "CC'd while Performing"),
	SIR_Knock UMETA(DisplayName = "Knocked Up/Back while Performing"),
	SIR_Died UMETA(DisplayName = "Character Died while Performing"),
	SIR_UserCancelled UMETA(DisplayName = "User Cancelled while Performing"),
	SIR_Max UMETA(Hidden),
};

UCLASS()
class ASkill : public AActor
{
	friend class AGameCharacter;

	GENERATED_UCLASS_BODY()

protected:

	/* what current state the skill is in */
	UPROPERTY(replicated)
	TEnumAsByte<ESkillState> skillState;

	/* character using this skill */
	UPROPERTY(BlueprintReadWrite, replicated, Category = Character)
	AGameCharacter* characterOwner;

	/* amount of skill points this skill has */
	UPROPERTY(replicated, BlueprintReadOnly, Category = Skill)
	int32 skillPoints;

	/* max amount of skill points the skill can have */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Skill)
	int32 skillPointsMax;

	/* timer for cooldowns */
	FTimerHandle cooldownTimer;

	/* next state to go to after cooldown is finished */
	ESkillState afterCooldownState;

	/* for when the cooldown timer is set */
	UPROPERTY(ReplicatedUsing = OnCooldownTimerSet)
	float cooldownTime;

	/* whether or not this skill automatically enters the performing state on use */
	UPROPERTY(EditDefaultsOnly, Category = Skill)
	bool bAutoPerform;

	/* minimum amount of time for cooldown for this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	float cooldownMin;

	/* max amount of time for cooldown for this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	float cooldownMax;

	/* minimum amount of cost for this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	float cost;

	/* name of this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	FText skillName;

	/* description of the skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill)
	FText skillDesc;

	/* whether or not to start cooldown immediately as this skill is interrupted */
	UPROPERTY(EditDefaultsOnly, Category = Skill)
	bool bAutoCooldownOnInterrupt;

	/* called when cooldown is finished */
	void CooldownFinished();

	UFUNCTION()
	void OnCooldownTimerSet();

public:

	/* sphere trace with a certain radius */
	UFUNCTION(BlueprintCallable, Category = Trace)
	static bool SphereTrace(AActor* actorToIgnore, const FVector& start, const FVector& end, const float radius, TArray<FHitResult>& hitOut, ECollisionChannel traceChannel = ECC_Pawn);

	/*since there's no native cone collision detection, cheat by creating a set of increasing radius spheres in a line */
	UFUNCTION(BlueprintCallable, Category = Trace)
	static bool ConeTrace(AActor* actorToIgnore, const FVector& start, const FVector& dir, float coneHeight, TArray<AGameCharacter*>& hitsOut, ECollisionChannel traceChannel = ECC_Pawn);

	/* initialize this skill to a character specified */
	void InitializeSkill(AGameCharacter* owner);

	/* [CLIENT] skill specific logic that happens on every client */
	UFUNCTION(BlueprintImplementableEvent, Category = Skill)
		void ClientSkillPerformed(FVector mouseHitLoc, AGameCharacter* targetUnit = NULL);

	/* [SERVER] skill specific logic that happens when this skill is activated on the server side (important logic here) */
	UFUNCTION(BlueprintNativeEvent, Category = Skill)
		void ServerSkillPerformed(FVector mouseHitLoc, AGameCharacter* targetUnit = NULL);

	/* start cooldown for the skill */
	UFUNCTION(BlueprintCallable, Category = Skill)
	void StartCooldown(float manualCooldown = -1.f, ESkillState cdfState = ESkillState::Ready);

	/* [SERVER] add a skill point */
	UFUNCTION(BlueprintCallable, Category = Skill)
		void AddSkillPoint();

	/* gets a value scaled by this skill's level */
	UFUNCTION(BlueprintCallable, Category = Skill)
		float SkillLevelScale(float min, float max, bool bIncreasing) const;

	/* creates a damage event struct */
	UFUNCTION(BlueprintCallable, Category = Damage)
		static FDamageEvent CreateDamageEvent(TSubclassOf<UDamageType> damageType);

	/* gets the percentage of cooldown progress */
	UFUNCTION(BlueprintCallable, Category = Cooldown)
		float GetCooldownProgressPercent();

	/* gets the amount of time left in the cooldown */
	UFUNCTION(BlueprintCallable, Category = Cooldown)
		float GetCooldownRemaining();

	/* gets the current cost of this skill */
	UFUNCTION(BlueprintCallable, Category = Cooldown)
		float GetCost();

	/* called after the skill is finished being performed to use flare and start cooldowns */
	UFUNCTION(BlueprintCallable, Category = Cooldown)
	void SkillFinished(float manualCooldown = -1.f);

	/* gets the current skill state */
	UFUNCTION(BlueprintCallable, Category = SkillState)
	ESkillState GetSkillState() const;

	/* sets the current skill state */
	UFUNCTION(BlueprintCallable, Category=Skill)
	void SetSkillState(ESkillState newState);

	/* gets the skill's name */
	UFUNCTION(BlueprintCallable, Category = CharacterName)
	FText GetSkillName() const
	{
		return skillName;
	}

	/* gets the skill's desc */
	UFUNCTION(BlueprintCallable, Category = CharacterName)
	FText GetSkillDesc() const
	{
		return skillDesc;
	}

	/* called whenever this skill should be interrupted (if its being performed) */
	UFUNCTION(BlueprintCallable, Category = Skill)
	void InterruptSkill(ESkillInterruptReason interruptReason, FVector mousePos = FVector::ZeroVector, AGameCharacter* targetUnit = nullptr);

	/* event called whenever this skill is interrupted for blueprints */
	UFUNCTION(BlueprintImplementableEvent, Category = Skill)
	void OnCanInterruptSkill(ESkillInterruptReason interruptReason, FVector mousePos = FVector::ZeroVector, AGameCharacter* targetUnit = nullptr);

	/* whether or not this skill can upgrade its skill point count. returns false if this skill is too far more upgraded than the rest of the player's skills */
	UFUNCTION(BlueprintCallable, Category = Skill)
	bool CanSkillUpgrade() const;

	/* logic to reenable the skill after it has been disabled. tests to make sure this skill can actually be reenabled and determines what skill state to go to */
	UFUNCTION(BlueprintCallable, Category = Skill)
	void ReenableSkill();

	/* get the ground location beneath the specified point */
	UFUNCTION(BlueprintCallable, Category = Skill)
	FVector GetGroundLocationBeneathPoint(FVector point);

	/* event that the player controller calls when the player is providing movement input while this skill is performing (useful for aiming, etc.) */
	UFUNCTION(BlueprintImplementableEvent, Category = Skill)
	void TargetInputReceivedWhilePerforming(const FHitResult& hitData);
};