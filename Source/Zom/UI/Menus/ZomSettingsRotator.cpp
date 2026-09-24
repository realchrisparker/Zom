// Fill out your copyright notice in the Description page of Project Settings.


#include "Zom/UI/Menus/ZomSettingsRotator.h"
#include "Components/Button.h"

#if WITH_EDITOR
#include "Blueprint/WidgetTree.h"
#include "Editor/WidgetCompilerLog.h"
#endif

#define LOCTEXT_NAMESPACE "ZomSettingsRotator"


void UZomSettingsRotator::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	LeftArrowButton->OnClicked.AddDynamic(this, &UZomSettingsRotator::HandleLeftArrowClicked);
	RightArrowButton->OnClicked.AddDynamic(this, &UZomSettingsRotator::HandleRightArrowClicked);
}

#if WITH_EDITOR
void UZomSettingsRotator::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	// BindWidget members aren't assigned yet at this point, so look the arrows up by name in the given tree.
	for (const FName ArrowName : { GET_MEMBER_NAME_CHECKED(UZomSettingsRotator, LeftArrowButton), GET_MEMBER_NAME_CHECKED(UZomSettingsRotator, RightArrowButton) })
	{
		const UButton* Arrow = BlueprintWidgetTree.FindWidget<UButton>(ArrowName);
		if (Arrow && Arrow->GetIsFocusable())
		{
			CompileLog.Error(FText::Format(
				LOCTEXT("ArrowFocusable", "{0} must have Is Focusable unticked, or gamepad focus will land on the arrow instead of the rotator."),
				FText::FromName(ArrowName)));
		}
	}
}
#endif

void UZomSettingsRotator::HandleLeftArrowClicked()
{
	ShiftTextLeft();
}

void UZomSettingsRotator::HandleRightArrowClicked()
{
	ShiftTextRight();
}

#undef LOCTEXT_NAMESPACE
