/**
*/
#include "UEChat.h"
#include "UEChatSettings.h"
#if WITH_EDITOR
#    include "ISettingsModule.h"
#endif

DEFINE_LOG_CATEGORY(LogUEChat);
#define LOCTEXT_NAMESPACE "UEChat"
void FUEChatModule::StartupModule()
{
#if WITH_EDITOR
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if(nullptr != SettingsModule) {
        SettingsModule->RegisterSettings(
            TEXT("Project"),
            TEXT("Plugins"),
            TEXT("UEChat"),
            LOCTEXT("UEChatSettingName", "UEChat"),
            LOCTEXT("UEChatSettingDescription", "UEChat description"),
            GetMutableDefault<UUEChatSettings>());
    }
#endif
}

void FUEChatModule::ShutdownModule()
{
#if WITH_EDITOR
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if(nullptr != SettingsModule) {
        SettingsModule->UnregisterSettings(
            TEXT("Project"),
            TEXT("Plugins"),
            TEXT("UEChat"));
    }
#endif
}

IMPLEMENT_MODULE(FUEChatModule, UEChat)
#undef LOCTEXT_NAMESPACE

