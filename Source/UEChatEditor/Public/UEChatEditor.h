#pragma once
/**
 @author t-sakai
 */
#include <CoreMinimal.h>
#include <Modules/ModuleManager.h>

#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
DECLARE_LOG_CATEGORY_EXTERN(LogUEChatEditor, Log, All);
#else
DECLARE_LOG_CATEGORY_EXTERN(LogUEChatEditor, Verbose, All);
#endif

class FUICommandList;
class FUEChatPanel;

class FUEChatEditorModule : public IModuleInterface
{
public:
	static const FName TabName;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void OpenPanel();

	TSharedPtr<FUICommandList> UICommandList_;
	TSharedPtr<FUEChatPanel> Panel_;
};

