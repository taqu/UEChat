/**
*/
#include "UEChatPanel.h"

#include <SlateBasics.h>
#include <Widgets/SCompoundWidget.h>
#include <Widgets/Docking/SDockTab.h>
#include <Framework/Docking/TabManager.h>
#include <Widgets/Input/SButton.h>
#include <Widgets/Layout/SWidgetSwitcher.h>
#include <Widgets/Text/SRichTextBlock.h>
#include <Widgets/Input/SComboBox.h>
#include <Widgets/Text/STextBlock.h>
#include <Widgets/Input/SMultiLineEditableTextBox.h>
#include <Widgets/Input/SNumericEntryBox.h>
#include <Widgets/Text/STextBlock.h>
#include "Styling/SlateStyle.h"
#include "Styling/StyleColors.h"
#include "Styling/AppStyle.h"

//--- FUEChatPanel::SChatView
//------------------------------------------
void FUEChatPanel::SChatView::Construct(const FArguments& InArgs)
{
	ActiveTimerDelegate_.BindRaw(this, &SChatView::Update);
	UEChatAPI_.OnResponse().BindRaw(this, &SChatView::OnComplete);

	//
	SlateStyleSet_ = MakeShareable(new FSlateStyleSet(TEXT("ChatPanel")));

	FTextBlockStyle NormalText = FAppStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");
	const FTextBlockStyle NormalLogText = FTextBlockStyle(NormalText)
											 .SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 14))
											 .SetColorAndOpacity(FStyleColors::Foreground)
											 .SetSelectedBackgroundColor(FStyleColors::Highlight)
											 .SetHighlightColor(FStyleColors::Black);

	SlateStyleSet_->Set("Log.Normal", NormalLogText);
	SlateStyleSet_->Set("Log.Warning", FTextBlockStyle(NormalLogText)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 12))
		.SetColorAndOpacity(FStyleColors::Warning));
	SlateStyleSet_->Set("Log.Error", FTextBlockStyle(NormalLogText)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 12))
		.SetColorAndOpacity(FStyleColors::Error));
	SlateStyleSet_->Set("Log.Highlight", FTextBlockStyle(NormalLogText)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 12))
		.SetColorAndOpacity(FStyleColors::AccentWhite));
	SlateStyleSet_->Set("Log.Success", FTextBlockStyle(NormalLogText)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 12))
		.SetColorAndOpacity(FStyleColors::Success));

	// clang-format off
	ChildSlot[
		SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(0.9f)[
				SAssignNew(ScrollBox_, SScrollBox)
					.Orientation(EOrientation::Orient_Vertical)
					+ SScrollBox::Slot()[
						SNew(SScrollBox)
							.Orientation(EOrientation::Orient_Horizontal)
							+ SScrollBox::Slot()[
								SAssignNew(TextBlock_, SRichTextBlock)
								.TextStyle(&NormalLogText)
								.DecoratorStyleSet(SlateStyleSet_.Get())
							]
					]
			]
			+ SVerticalBox::Slot().FillHeight(0.1f)[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1)[
						SAssignNew(EditBox_, SMultiLineEditableTextBox)
				]
			]
			+ SVerticalBox::Slot().AutoHeight()[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()[
						SNew(SButton)
							.Text(FText::FromString(TEXT("Send")))
							.HAlign(HAlign_Left)
							.OnClicked_Lambda([this]() {
								Send();
								return FReply::Handled();
							})
					]
					+ SHorizontalBox::Slot().AutoWidth()[
						SNew(SButton)
							.Text(FText::FromString(TEXT("Clear")))
							.HAlign(HAlign_Left)
							.OnClicked_Lambda([this]() {
								StringBuilder_.Reset();
								TextBlock_->SetText(FText::FromString(TEXT("")));
								return FReply::Handled();
							})
					]
			]
	];
	// clang-format on
}

EActiveTimerReturnType FUEChatPanel::SChatView::Update(double InCurrentTime, float InDeltaTime)
{
#if 0
	UUnrealEditorSubsystem* UnrealEditorSubsystem = GEditor->GetEditorSubsystem<UUnrealEditorSubsystem>();
	if(!UnrealEditorSubsystem) {
		return EActiveTimerReturnType::Stop;
	}

	UWorld* ActiveWorld = UnrealEditorSubsystem->GetEditorWorld();

	if(!IsValid(ActiveWorld)) {
		return EActiveTimerReturnType::Stop;
	}
	FString Result;
	switch(Step_) {
	case Step::Lighting:
		UUnionSystemEditorBP::ValidateLighting(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		Step_ = Step::Shadowing;
		break;
	case Step::Shadowing:
		UUnionSystemEditorBP::ValidateShadowing(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		Step_ = Step::PostProcess;
		break;
	case Step::PostProcess:
		UUnionSystemEditorBP::ValidatePostProcess(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		Step_ = Step::Geometry;
		break;
	case Step::Geometry:
		UUnionSystemEditorBP::ValidateGeometry(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		Step_ = Step::Functions;
		break;
	case Step::Functions:
		UUnionSystemEditorBP::ValidateFunctions(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		ValidationLogTextBlock_->SetText(FText::FromString(StringBuilder_.ToString()));
		ValidationLogScrollBox_->ScrollToEnd();
		Step_ = Step::Lighting;
		return EActiveTimerReturnType::Stop;
	default:
		return EActiveTimerReturnType::Stop;
	}
	ValidationLogTextBlock_->SetText(FText::FromString(StringBuilder_.ToString()));
	ValidationLogScrollBox_->ScrollToEnd();
#endif
	return EActiveTimerReturnType::Continue;
}

void FUEChatPanel::SChatView::Send()
{
	FText Text = EditBox_->GetText();
	if(Text.IsEmpty()){
		return;
    }
	UEChatAPI_.Send(Text.ToString());
	StringBuilder_.Append(Text.ToString());
	StringBuilder_.AppendChar(TEXT('\n'));
	TextBlock_->SetText(FText::FromString(StringBuilder_.ToString()));
}

void FUEChatPanel::SChatView::OnComplete(bool Status, const FString& Message)
{
    if(!Status) {
        return;
    }
	StringBuilder_.Append(Message);
	StringBuilder_.AppendChar(TEXT('\n'));
	TextBlock_->SetText(FText::FromString(StringBuilder_.ToString()));
}

//--- FUEChatPanel::SChatWithLogView
//------------------------------------------
void FUEChatPanel::SChatWithLogView::Construct(const FArguments& InArgs)
{
	ActiveTimerDelegate_.BindRaw(this, &SChatWithLogView::Update);
	UEChatAPI_.OnResponse().BindRaw(this, &SChatWithLogView::OnComplete);

	//
	SlateStyleSet_ = MakeShareable(new FSlateStyleSet(TEXT("ChatWithLogPanel")));

	FTextBlockStyle NormalText = FAppStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");
	const FTextBlockStyle NormalLogText = FTextBlockStyle(NormalText)
											 .SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 14))
											 .SetColorAndOpacity(FStyleColors::Foreground)
											 .SetSelectedBackgroundColor(FStyleColors::Highlight)
											 .SetHighlightColor(FStyleColors::Black);

	SlateStyleSet_->Set("Log.Normal", NormalLogText);
	SlateStyleSet_->Set("Log.Warning", FTextBlockStyle(NormalLogText)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 12))
		.SetColorAndOpacity(FStyleColors::Warning));
	SlateStyleSet_->Set("Log.Error", FTextBlockStyle(NormalLogText)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 12))
		.SetColorAndOpacity(FStyleColors::Error));
	SlateStyleSet_->Set("Log.Highlight", FTextBlockStyle(NormalLogText)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 12))
		.SetColorAndOpacity(FStyleColors::AccentWhite));
	SlateStyleSet_->Set("Log.Success", FTextBlockStyle(NormalLogText)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 12))
		.SetColorAndOpacity(FStyleColors::Success));

	// clang-format off
	ChildSlot[
		SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(0.9f)[
				SAssignNew(ScrollBox_, SScrollBox)
					.Orientation(EOrientation::Orient_Vertical)
					+ SScrollBox::Slot()[
						SNew(SScrollBox)
							.Orientation(EOrientation::Orient_Horizontal)
							+ SScrollBox::Slot()[
								SAssignNew(TextBlock_, SRichTextBlock)
								.TextStyle(&NormalLogText)
								.DecoratorStyleSet(SlateStyleSet_.Get())
							]
					]
			]
			+ SVerticalBox::Slot().FillHeight(0.1f)[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1)[
						SAssignNew(EditBox_, SMultiLineEditableTextBox)
				]
			]
			+ SVerticalBox::Slot().AutoHeight()[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()[
						SNew(SButton)
							.Text(FText::FromString(TEXT("Send")))
							.HAlign(HAlign_Left)
							.OnClicked_Lambda([this]() {
								SendWithLog();
								return FReply::Handled();
							})
					]
					+ SHorizontalBox::Slot().AutoWidth()[
						SNew(SButton)
							.Text(FText::FromString(TEXT("Clear")))
							.HAlign(HAlign_Left)
							.OnClicked_Lambda([this]() {
								StringBuilder_.Reset();
								TextBlock_->SetText(FText::FromString(TEXT("")));
								return FReply::Handled();
							})
					]
					+ SHorizontalBox::Slot().AutoWidth()[
						SNew(STextBlock)
							.Text(FText::FromString(TEXT("Log Lines")))
					]
					+ SHorizontalBox::Slot().AutoWidth()[
						SAssignNew(Lines_, SNumericEntryBox<int32>)
							.AllowSpin(true)
							.Value(10)
							.MinSliderValue(1)
							.MaxSliderValue(100)
							.MinDesiredValueWidth(75)
							.OnValueChanged(this, &SChatWithLogView::OnLinesChanged)
					]
			]
	];
	// clang-format on
}

void FUEChatPanel::SChatWithLogView::OnLinesChanged(int32 X)
{
	LogLines_ = X;
}

EActiveTimerReturnType FUEChatPanel::SChatWithLogView::Update(double InCurrentTime, float InDeltaTime)
{
#if 0
	UUnrealEditorSubsystem* UnrealEditorSubsystem = GEditor->GetEditorSubsystem<UUnrealEditorSubsystem>();
	if(!UnrealEditorSubsystem) {
		return EActiveTimerReturnType::Stop;
	}

	UWorld* ActiveWorld = UnrealEditorSubsystem->GetEditorWorld();

	if(!IsValid(ActiveWorld)) {
		return EActiveTimerReturnType::Stop;
	}
	FString Result;
	switch(Step_) {
	case Step::Lighting:
		UUnionSystemEditorBP::ValidateLighting(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		Step_ = Step::Shadowing;
		break;
	case Step::Shadowing:
		UUnionSystemEditorBP::ValidateShadowing(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		Step_ = Step::PostProcess;
		break;
	case Step::PostProcess:
		UUnionSystemEditorBP::ValidatePostProcess(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		Step_ = Step::Geometry;
		break;
	case Step::Geometry:
		UUnionSystemEditorBP::ValidateGeometry(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		Step_ = Step::Functions;
		break;
	case Step::Functions:
		UUnionSystemEditorBP::ValidateFunctions(Result, ActiveWorld);
		StringBuilder_.Append(Result);
		ValidationLogTextBlock_->SetText(FText::FromString(StringBuilder_.ToString()));
		ValidationLogScrollBox_->ScrollToEnd();
		Step_ = Step::Lighting;
		return EActiveTimerReturnType::Stop;
	default:
		return EActiveTimerReturnType::Stop;
	}
	ValidationLogTextBlock_->SetText(FText::FromString(StringBuilder_.ToString()));
	ValidationLogScrollBox_->ScrollToEnd();
#endif
	return EActiveTimerReturnType::Continue;
}

void FUEChatPanel::SChatWithLogView::SendWithLog()
{
}

void FUEChatPanel::SChatWithLogView::OnComplete(bool Status, const FString& Message)
{
}

//--------------------------------------------------
TSharedRef<SDockTab> FUEChatPanel::OnSpawnTab(const FSpawnTabArgs& Args)
{
	// clang-format off
	TSharedRef<SDockTab> Tab = SNew(SDockTab)
	.ShouldAutosize(false)
	.TabRole(NomadTab)
	[
		GeneratePanelContent()
	];
	// clang-format on

	return Tab;
}

TSharedRef<SWidget> FUEChatPanel::GeneratePanelContent()
{
	// clang-format off
	TSharedRef<SWidget> ContentWidget = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[
			SAssignNew(Tabs_, SHorizontalBox) + SHorizontalBox::Slot().AutoWidth()
        ]
        + SVerticalBox::Slot().FillHeight(1.0f)[
            SAssignNew(WidgetsSwitcher_, SWidgetSwitcher)
        ];
	// clang-format on

	{
		// clang-format off
		// coverity[uninit_use_in_call]
		TSharedPtr<SWidget> Widget = SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1)[
				SAssignNew(ChatView_, SChatView)
			];
		// clang-format on
		AddTab(TEXT("Chat"), Widget);
	}

	{
		// clang-format off
		// coverity[uninit_use_in_call]
		TSharedPtr<SWidget> Widget = SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1)[
				SAssignNew(ChatWithLogView_, SChatWithLogView)
			];
		// clang-format on
		AddTab(TEXT("ChatWithLog"), Widget);
	}
	OnClickTab(0);
	return ContentWidget;
}

void FUEChatPanel::AddTab(const TCHAR* Name, TSharedPtr<SWidget> Widget)
{
	int32 ID = Tabs_->NumSlots() - 1;
	// clang-format off
	// coverity[uninit_use_in_call]
	TSharedPtr<SButton> TabButton = SNew(SButton)
		.Text(FText::FromString(Name))
		.ButtonStyle(FAppStyle::Get(), "FlatButton")
		.OnClicked_Lambda([this, ID]() {
			OnClickTab(ID);
			return FReply::Handled();
		});
	// clang-format on

	Tabs_->AddSlot().AutoWidth().HAlign(EHorizontalAlignment::HAlign_Center).AttachWidget(TabButton.ToSharedRef());
	WidgetsSwitcher_->AddSlot().AttachWidget(Widget.ToSharedRef());
	TabButtons_.Add(TabButton);
}

void FUEChatPanel::OnClickTab(int32 ID)
{
	WidgetsSwitcher_->SetActiveWidgetIndex(ID);
	ActivateTabButton(ID);
}

void FUEChatPanel::ActivateTabButton(int32 ID)
{
	for (TSharedPtr<SButton>& Button : TabButtons_) {
		Button->SetButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("FlatButton.Dark")));
	}
	if (0 <= ID && ID < TabButtons_.Num()) {
		TabButtons_[ID]->SetButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("FlatButton")));
	}
}

