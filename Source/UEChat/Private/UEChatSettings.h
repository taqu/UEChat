#pragma once
#include <CoreMinimal.h>

/**
*/
#include "UEChatSettings.generated.h"

/**
 * 
 */
UCLASS(config=UEChat, defaultconfig)
class UUEChatSettings : public UObject
{
    GENERATED_UCLASS_BODY()
public:
    UPROPERTY(EditAnywhere, config, Category = "System")
	FString EndPoint = TEXT("");

    UPROPERTY(EditAnywhere, config, Category = "System")
	FString APIKey = TEXT("");

    UPROPERTY(EditAnywhere, config, Category = "System")
	FString Model = TEXT("gpt-4o");
};

