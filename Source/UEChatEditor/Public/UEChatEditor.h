#pragma once
/**
 @author t-sakai
 */
#include <CoreMinimal.h>
#include <Modules/ModuleManager.h>
#include <Misc/OutputDevice.h>

#include "UEChat/Private/RingList.h"

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

	struct FLog
    {
		FLog();
		FLog(ELogVerbosity::Type Verbosity, FString&& Line);
		FLog(const FLog& X);
		FLog& operator=(const FLog& X);
		FLog(FLog&& X);
		FLog& operator=(FLog&& X);

		ELogVerbosity::Type Verbosity_;
		FString Line_;
    };

	/**
	@brief Caputuring Logging Output Device
	*/
	class FLogOutputDevice: public FOutputDevice
	{
	public:
		inline static constexpr uint32 MaxLines = 4096;
		FLogOutputDevice();
		virtual ~FLogOutputDevice() override;
	protected:
		virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
		virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category, double Time) override;
		TRingList<FLog> Logs_;
	};

	FLogOutputDevice LogOutputDevice_;
};

