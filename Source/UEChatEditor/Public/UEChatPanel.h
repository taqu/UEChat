#pragma once
/**
*/
#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>
#include <Types/WidgetActiveTimerDelegate.h>
#include <Misc/StringBuilder.h>
#include <Framework/Text/BaseTextLayoutMarshaller.h>

#include "UEChatAPI.h"

class SWidget;
class SDockTab;
class FSpawnTabArgs;
class SHorizontalBox;
class SRichTextBlock;
class SMultiLineEditableTextBox;
class SScrollBox;
class FSlateStyleSet;
class SWidgetSwitcher;
class SButton;
template<class T> class SNumericEntryBox;

class FUEChatPanel
{
public:
	class SChatView : public SCompoundWidget
	{
		using Super = SCompoundWidget;
	public:
		// clang-format off
		SLATE_BEGIN_ARGS(SChatView){}
		SLATE_END_ARGS()
		// clang-format on

		void Construct(const FArguments& InArgs);

	private:
		EActiveTimerReturnType Update(double InCurrentTime, float InDeltaTime);
		void Send();
		void OnComplete(bool Status, const FString& Message);

		TSharedPtr<FSlateStyleSet> SlateStyleSet_;
		FWidgetActiveTimerDelegate ActiveTimerDelegate_;
		TSharedPtr<SRichTextBlock> TextBlock_;
		TSharedPtr<SScrollBox> ScrollBox_;
		TSharedPtr<SMultiLineEditableTextBox> EditBox_;

		FStringBuilderBase StringBuilder_;
		UEChatAPI UEChatAPI_;
	};

	class SChatWithLogView : public SCompoundWidget
	{
		using Super = SCompoundWidget;
	public:
		// clang-format off
		SLATE_BEGIN_ARGS(SChatWithLogView){}
		SLATE_END_ARGS()
		// clang-format on

		void Construct(const FArguments& InArgs);

	private:
		void OnLinesChanged(int32 X);
		EActiveTimerReturnType Update(double InCurrentTime, float InDeltaTime);
		void SendWithLog();
		void OnComplete(bool Status, const FString& Message);

		TSharedPtr<FSlateStyleSet> SlateStyleSet_;
		FWidgetActiveTimerDelegate ActiveTimerDelegate_;
		TSharedPtr<SRichTextBlock> TextBlock_;
		TSharedPtr<SScrollBox> ScrollBox_;
		TSharedPtr<SMultiLineEditableTextBox> EditBox_;
		TSharedPtr<SNumericEntryBox<int32>> Lines_;
		int32 LogLines_ = 10;

		FStringBuilderBase StringBuilder_;
		UEChatAPI UEChatAPI_;
	};

	TSharedRef<SDockTab> OnSpawnTab(const FSpawnTabArgs& Args);

private:
	TSharedRef<SWidget> GeneratePanelContent();

	void AddTab(const TCHAR* Name, TSharedPtr<SWidget> Widget);
	void OnClickTab(int32 ID);
	void ActivateTabButton(int32 ID);

	TSharedPtr<SHorizontalBox> Tabs_;
	TSharedPtr<SWidgetSwitcher> WidgetsSwitcher_;
	TArray<TSharedPtr<SButton>> TabButtons_;

	TSharedPtr<SChatView> ChatView_;
	TSharedPtr<SChatWithLogView> ChatWithLogView_;
};

