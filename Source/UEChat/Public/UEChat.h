#pragma once
/**
 @author t-sakai
 */
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
DECLARE_LOG_CATEGORY_EXTERN(LogUEChat, Log, All);
#else
DECLARE_LOG_CATEGORY_EXTERN(LogUEChat, Verbose, All);
#endif

class FUEChatModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

