#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

/**
 * Funções utilitárias para UI
 */
namespace UIUtilities
{
    /**
     * Obtém o nome de exibição de um valor enum
     * @param EnumValue O valor do enum
     * @return String com o nome de exibição do enum
     */
    template<typename EnumType>
    static FString GetEnumDisplayName(EnumType EnumValue)
    {
        UEnum* Enum = StaticEnum<EnumType>();
        if (Enum)
        {
            int32 Index = static_cast<int32>(EnumValue);
            return Enum->GetDisplayNameTextByValue(Index).ToString();
        }
        return TEXT("Unknown");
    }

    /**
     * Obtém todos os valores de um enum como array de strings
     * @return Array com todos os valores do enum
     */
    template<typename EnumType>
    static TArray<FString> GetEnumValues()
    {
        TArray<FString> Values;
        UEnum* Enum = StaticEnum<EnumType>();
        if (Enum)
        {
            for (int32 i = 0; i < Enum->NumEnums(); i++)
            {
                // Pular o valor "None" (geralmente é o primeiro)
                if (i > 0)
                {
                    FString DisplayName = Enum->GetDisplayNameTextByValue(i).ToString();
                    if (DisplayName != TEXT("None"))
                    {
                        Values.Add(DisplayName);
                    }
                }
            }
        }
        return Values;
    }
}
