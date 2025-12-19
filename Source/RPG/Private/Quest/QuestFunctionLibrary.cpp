#include "Quest/QuestFunctionLibrary.h"

FText UQuestFunctionLibrary::GetQuestTypeText(EQuestType QuestType)
{
    const UEnum* EnumPtr = StaticEnum<EQuestType>();
    if (!EnumPtr)
    {
        return FText::GetEmpty();
    }
    const int64 Value = static_cast<int64>(QuestType);
    return EnumPtr->GetDisplayNameTextByValue(Value);
}

