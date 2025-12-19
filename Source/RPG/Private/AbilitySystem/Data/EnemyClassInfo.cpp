// Copyright Druid Mechanics

#include "AbilitySystem/Data/EnemyClassInfo.h"

FCharacterClassDefaultInfo UEnemyClassInfo::GetClassDefaultInfo(ECharacterClass CharacterClass)
{
    if (const FCharacterClassDefaultInfo* Found = CharacterClassInformation.Find(CharacterClass))
    {
        return *Found;
    }
    // Retorne um default
    return FCharacterClassDefaultInfo();
}
