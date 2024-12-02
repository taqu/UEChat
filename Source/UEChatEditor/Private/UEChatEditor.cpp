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

	GLog->AddOutputDevice(&LogOutputDevice_);
}

void FUEChatEditorModule::ShutdownModule()
{
	GLog->RemoveOutputDevice(&LogOutputDevice_);

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

FUEChatEditorModule::FLog::FLog()
    :Verbosity_(ELogVerbosity::NoLogging)
{
}

FUEChatEditorModule::FLog::FLog(ELogVerbosity::Type Verbosity, FString&& Line)
:Verbosity_(Verbosity)
	,Line_(MoveTemp(Line))

{
}

FUEChatEditorModule::FLog::FLog(const FLog& X)
    :Verbosity_(X.Verbosity_)
	,Line_(X.Line_)
{
}

FUEChatEditorModule::FLog& FUEChatEditorModule::FLog::operator=(const FLog& X)
{
	if(this != &X){
		Verbosity_ = X.Verbosity_;
		Line_ = X.Line_;
	}
	return *this;
}

FUEChatEditorModule::FLog::FLog(FLog&& X)
:Verbosity_(X.Verbosity_)
	,Line_(MoveTemp(X.Line_))
{
}

FUEChatEditorModule::FLog& FUEChatEditorModule::FLog::operator=(FLog&& X)
{
	if(this != &X){
		Verbosity_ = X.Verbosity_;
		Line_ = MoveTemp(X.Line_);
	}
	return *this;
}

FUEChatEditorModule::FLogOutputDevice::FLogOutputDevice()
{
}

FUEChatEditorModule::FLogOutputDevice::~FLogOutputDevice()
{
}

void FUEChatEditorModule::FLogOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if(ELogVerbosity::Warning<Verbosity){
		return;
	}
	if(MaxLines<=Logs_.Size()){
		Logs_.PopHead();
	}
	FString Line(V);
	FLog Log(Verbosity, MoveTemp(Line));
	Logs_.PushTail(MoveTemp(Log));
}

void FUEChatEditorModule::FLogOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category, double /*Time*/)
{
	if(ELogVerbosity::Warning<Verbosity){
		return;
	}
	if(MaxLines<=Logs_.Size()){
		Logs_.PopHead();
	}
	FString Line(V);
	FLog Log(Verbosity, MoveTemp(Line));
	Logs_.PushTail(MoveTemp(Log));
}

#undef LOCTEXT_NAMESPACE
