#include "Realm.h"
#include "ModManager.h"
#include "Mod.h"

AModManager::AModManager(const FObjectInitializer& objectInitializer)
: Super(objectInitializer)
{

}

void AModManager::CharacterDamaged(int32 dmgAmount, TSubclassOf<UDamageType> damageType, AGameCharacter* dmgCauser, AActor* actorCauser, FRealmDamage realmDamage)
{
	TArray<AMod*> mods = managedCharacter->GetMods();

	for (int32 i = 0; i < mods.Num(); i++)
		mods[i]->CharacterDamaged(dmgAmount, damageType, dmgCauser, actorCauser, realmDamage);
}

void AModManager::CharacterDealtDamage(int32 dmgAmount, TSubclassOf<UDamageType> damageType, AActor* actorCauser, FRealmDamage realmDamage, AGameCharacter* targetCharacter)
{
	TArray<AMod*> mods = managedCharacter->GetMods();

	for (int32 i = 0; i < mods.Num(); i++)
		mods[i]->CharacterDealtDamage(dmgAmount, damageType, actorCauser, realmDamage, targetCharacter);
}