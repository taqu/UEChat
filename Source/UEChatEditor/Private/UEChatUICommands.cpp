#include "UEChatUICommands.h"

#include <Styling/AppStyle.h>

#define LOCTEXT_NAMESPACE "UEChatEditor"

FUEChatUICommands::FUEChatUICommands()
	: TCommands<FUEChatUICommands>(TEXT("UEChatUICommands"),
										LOCTEXT("UEChatUICommands", "UEChat UI Commands"),
										NAME_None, FAppStyle::GetAppStyleSetName())
{
}

void FUEChatUICommands::RegisterCommands()
{
	UI_COMMAND(OpenPanel_, "UEChatUICommands", "Open Chat Panel", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE