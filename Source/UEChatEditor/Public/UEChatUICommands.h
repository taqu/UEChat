#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FUICommandInfo;

class FUEChatUICommands : public TCommands<FUEChatUICommands>
{
public:

	FUEChatUICommands();

	//--- TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPanel_;
};
