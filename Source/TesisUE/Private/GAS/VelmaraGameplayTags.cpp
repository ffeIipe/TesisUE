#include "GAS/VelmaraGameplayTags.h"
#include "GameplayTagsManager.h"

FVelmaraGameplayTags FVelmaraGameplayTags::GameplayTags;

void FVelmaraGameplayTags::InitializeGameplayTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	GameplayTags.State_Dead = Manager.AddNativeGameplayTag(FName("State.Dead"));
	GameplayTags.State_Shielded = Manager.AddNativeGameplayTag(FName("State.Shielded"));
	GameplayTags.State_Stunned = Manager.AddNativeGameplayTag(FName("State.Stunned"));
	GameplayTags.State_Attacking = Manager.AddNativeGameplayTag(FName("State.Attacking"));
	GameplayTags.State_Blocking = Manager.AddNativeGameplayTag(FName("State.Blocking"));
	GameplayTags.State_Executing = Manager.AddNativeGameplayTag(FName("State.Executing"));
	GameplayTags.State_Dodging = Manager.AddNativeGameplayTag(FName("State.Dodging"));
	GameplayTags.State_ComboWindow = Manager.AddNativeGameplayTag(FName("State.ComboWindow"));
	
	GameplayTags.Mode_Primary = Manager.AddNativeGameplayTag(FName("Mode.Primary"));
	GameplayTags.Mode_Secondary = Manager.AddNativeGameplayTag(FName("Mode.Secondary"));
}