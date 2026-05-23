#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatDirectorSubsystem.generated.h"

USTRUCT()
struct FCombatTargetData
{
	GENERATED_BODY()

	TArray<TWeakObjectPtr<AActor>> AttackingEnemies;

	TArray<TWeakObjectPtr<AActor>> RegisteredEnemies;

	int32 MaxConcurrentAttackers = 3;
};

UCLASS()
class TESISUE_API UCombatDirectorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void RegisterEnemy(AActor* Enemy, AActor* TargetActor);
    
    UFUNCTION(BlueprintCallable)
	void UnregisterEnemy(AActor* Enemy, AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	TArray<AActor*> GetAlliesForTarget(AActor* TargetActor) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool RequestAttackToken(AActor* Enemy, AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void ReturnAttackToken(AActor* Enemy, AActor* TargetActor);

protected:
	TMap<TWeakObjectPtr<AActor>, FCombatTargetData> CombatMap;

private:
	void CleanInvalidEntries(FCombatTargetData& Data);
};
