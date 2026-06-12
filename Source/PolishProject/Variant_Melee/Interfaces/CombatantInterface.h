#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatantInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatantInterface : public UInterface
{
	GENERATED_BODY()
};

class POLISHPROJECT_API ICombatantInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Combat")
	void PerformAttack();
};
