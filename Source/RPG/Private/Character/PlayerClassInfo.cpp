#include "Character/PlayerClassInfo.h"

FPlayerClassData UPlayerClassInfo::GetPlayerClassInfo(EPlayerClass PlayerClass)
{
    if (const FPlayerClassData* Found = PlayerClasses.Find(PlayerClass))
    {
        return *Found;
    }
    return FPlayerClassData();
}
