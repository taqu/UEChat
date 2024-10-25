/**
 */
#include "UEChatEditor.h"
#include "UEChatPanel.h"
#include "UEChatUICommands.h"

#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "UEChatEditor"

DEFINE_LOG_CATEGORY(LogUEChatEditor);

const FName FUEChatEditorModule::TabName("UEChatPanel");

void FUEChatEditorModule::StartupModule()
{
	FUEChatUICommands::Register();

	Panel_ = MakeShareable(new FUEChatPanel);
	UICommandList_ = MakeShareable(new FUICommandList);
	UICommandList_->MapAction(
		FUEChatUICommands::Get().OpenPanel_,
		FExecuteAction::CreateRaw(this, &FUEChatEditorModule::OpenPanel),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUEChatEditorModule::RegisterMenus));

	const FOnSpawnTab& Delegate = FOnSpawnTab::CreateRaw(Panel_.Get(), &FUEChatPanel::OnSpawnTab);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, Delegate)
		.SetDisplayName(FText::FromString(TEXT("UEChat Chat Panel")))
		.SetAutoGenerateMenuEntry(false);
}

void FUEChatEditorModule::ShutdownModule()
{
	FUEChatUICommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
}

void FUEChatEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& Section = Menu->FindOrAddSection(FName("Tools"));
        Section.AddMenuEntryWithCommandList(FUEChatUICommands::Get().OpenPanel_, UICommandList_, FText::FromString(TEXT("Chat Panel")), TAttribute<FText>(), FSlateIcon(FAppStyle::GetAppStyleSetName(), "DeveloperTools.MenuIcon"));
	}
}

void FUEChatEditorModule::OpenPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TabName));
}

IMPLEMENT_MODULE(FUEChatEditorModule, UEChatEditor)

#undef LOCTEXT_NAMESPACE
