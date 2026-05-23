#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SignalReceiver.generated.h"

UINTERFACE()
class USignalReceiver : public UInterface
{
	GENERATED_BODY()
};

class TESISUE_API ISignalReceiver
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceiveSignal(bool bActive, AActor* Activator);
};
