#include "Realm.h"
#include "RealmMoveController.h"
#include "GameCharacter.h"
#include "Navigation/CrowdFollowingComponent.h"

ARealmMoveController::ARealmMoveController(const FObjectInitializer& objectInitializer)
: Super(objectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	targetRadius = objectInitializer.CreateDefaultSubobject<UPawnSensingComponent>(this, TEXT("pawnSensor"));
	targetRadius->OnSeePawn.AddDynamic(this, &ARealmMoveController::OnTargetEnterRadius);
	targetRadius->SightRadius = 1000.f;
	targetRadius->bOnlySensePlayers = false;
	targetRadius->SensingInterval = 0.25f;
	targetRadius->SetPeripheralVisionAngle(180.f);
}

void ARealmMoveController::OnTargetEnterRadius(class APawn* pawn)
{
	AGameCharacter* gc = Cast<AGameCharacter>(pawn);
	AGameCharacter* mgc = Cast<AGameCharacter>(GetCharacter());

	if (!IsValid(gc) || !IsValid(mgc) || !gc->IsAlive() || gc->GetTeamIndex() == mgc->GetTeamIndex())
		return;

	inRangeTargets.AddUnique(gc);
}

void ARealmMoveController::NeedsNewCommand()
{
	AGameCharacter* mgc = Cast<AGameCharacter>(GetCharacter());

	if (!IsValid(mgc))
		return;

	float leastDistance = -1.f;
	int32 leastInd = -1;

	for (int32 i = 0; i < inRangeTargets.Num(); i++)
	{
		if (!IsValid(inRangeTargets[i]))
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		float distanceSq = (inRangeTargets[i]->GetActorLocation() - mgc->GetActorLocation()).SizeSquared2D();
		if (!inRangeTargets[i]->IsAlive() || distanceSq > FMath::Square(targetRadius->SightRadius))
		{
			inRangeTargets.RemoveAt(i);
			continue;
		}

		if (leastDistance == -1.f)
		{
			leastDistance = distanceSq;
			leastInd = i;
			continue;
		}

		if (distanceSq < leastDistance)
		{
			leastDistance = distanceSq;
			leastInd = i;
		}
	}

	if (leastInd >= 0)
		mgc->SetCurrentTarget(inRangeTargets[leastInd]);
	else
		mgc->StopAutoAttack();
}

void ARealmMoveController::CharacterDamaged(AGameCharacter* damager)
{
	AGameCharacter* mc = Cast<AGameCharacter>(GetCharacter());
	if (!IsValid(mc) || !IsValid(damager))
		return;

	if (damager->GetTeamIndex() != mc->GetTeamIndex())
	{
		//get all nearby friendly units
		for (TActorIterator<AGameCharacter> minionitr(GetWorld()); minionitr; ++minionitr)
		{
			AGameCharacter* mic = (*minionitr);
			if (IsValid(mic) && mic->GetTeamIndex() == mc->GetTeamIndex())
			{
				float distancesq = (mic->GetActorLocation() - mc->GetActorLocation()).SizeSquared2D();
				if (distancesq <= FMath::Square(710.f))
					mic->ReceiveCallForHelp(mc, damager);
			}
		}
	}
}