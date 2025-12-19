#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Character/RPGCharacter.h"
#include "Engine/DataTable.h"
#include "PartySubsystem.generated.h"

// Forward declarations
class UAbilitySystemComponent;
class UAttributeSet;
class URPGAbilitySystemComponent;
class URPGAttributeSet;

class ARPGCharacter;

// Declare os delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyMemberAddedSignature, ARPGCharacter*, NewMember);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyMemberRemovedSignature, ARPGCharacter*, RemovedMember);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActivePartyMemberChangedSignature, ARPGCharacter*, NewActiveMember);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPartyMembersChanged);

// Estrutura que associa UniqueID à classe de personagem unlocked
USTRUCT()
struct FPartyCharacterEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName UniqueID;

	UPROPERTY()
	TSoftClassPtr<ARPGCharacter> CharacterClass;
};

/**
 * Sistema global de gerenciamento de party implementado como um Subsystem
 * Controla a troca entre personagens jogáveis, mantém a lista de membros da party,
 * e gerencia o personagem atualmente controlado pelo jogador
 */
UCLASS()
class RPG_API UPartySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Delegates para eventos da party
	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyMemberAddedSignature OnPartyMemberAdded;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyMemberRemovedSignature OnPartyMemberRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnActivePartyMemberChangedSignature OnActivePartyMemberChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnPartyMembersChanged OnPartyMembersChanged;

	// Inicializa o subsistema
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Finaliza o subsistema
	virtual void Deinitialize() override;

	// Getter para PartyMembers
	const TArray<ARPGCharacter*>& GetCurrentPartyMembers() const { return PartyMembers; }

	// Adiciona um personagem à party
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool AddPartyMember(ARPGCharacter* CharacterToAdd);

	// Remove um personagem da party
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool RemovePartyMember(ARPGCharacter* CharacterToRemove);

	// Alterna para o próximo membro da party
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool SwitchToNextPartyMember();

	// Alterna para o próximo membro VIVO da party (usado quando personagem morre)
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool SwitchToNextAlivePartyMember();

	// Alterna para o membro anterior da party
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool SwitchToPreviousPartyMember();

	// Alterna para um membro específico da party pelo índice
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool SwitchToPartyMember(int32 MemberIndex);

	// Alterna para um membro específico diretamente
	UFUNCTION(BlueprintCallable, Category = "Party")
	bool SwitchToPartyMemberByReference(ARPGCharacter* TargetCharacter);

	// Obtém o membro ativo da party
	UFUNCTION(BlueprintPure, Category = "Party")
	ARPGCharacter* GetActivePartyMember() const;

	// Obtém todos os membros da party
	UFUNCTION(BlueprintPure, Category = "Party")
	const TArray<ARPGCharacter*>& GetPartyMembers() const;
	
	// Obtém todos os membros da party (alternativo)
	UFUNCTION(BlueprintPure, Category = "Party")
	TArray<ARPGCharacter*> GetAllPartyMembers() const { return PartyMembers; }

	// Obtém os nomes de todos os membros da party
	UFUNCTION(BlueprintPure, Category = "Party")
	TArray<FString> GetAllPartyMemberNames() const;

	// Obtém o índice do membro ativo
	UFUNCTION(BlueprintPure, Category = "Party")
	int32 GetActiveMemberIndex() const;

	// Alias para GetActiveMemberIndex para compatibilidade com código existente
	UFUNCTION(BlueprintPure, Category = "Party")
	int32 GetActivePartyMemberIndex() const { return GetActiveMemberIndex(); }

	// Obtém o número de membros na party
	UFUNCTION(BlueprintPure, Category = "Party")
	int32 GetPartyMemberCount() const { return PartyMembers.Num(); }

	// Obtém um membro da party pelo índice
	UFUNCTION(BlueprintPure, Category = "Party")
	ARPGCharacter* GetPartyMemberAt(int32 Index) const { return (Index >= 0 && Index < PartyMembers.Num()) ? PartyMembers[Index] : nullptr; }

	// Verifica se o personagem está na party
	UFUNCTION(BlueprintPure, Category = "Party")
	bool IsInParty(const ARPGCharacter* CharacterToCheck) const;

	// Obtém o tamanho atual da party
	UFUNCTION(BlueprintPure, Category = "Party")
	int32 GetPartySize() const;

	// Define a lista completa de membros da party
	UFUNCTION(BlueprintCallable, Category = "Party")
	void SetPartyMembers(const TArray<ARPGCharacter*>& NewPartyMembers);

	// Define diretamente o personagem ativo
	UFUNCTION(BlueprintCallable, Category = "Party")
	void SetActiveCharacter(ARPGCharacter* NewActiveCharacter);

	// Função para obter um membro do grupo pelo índice
	UFUNCTION(BlueprintCallable, Category = "Party")
	ARPGCharacter* GetPartyMemberAtIndex(int32 Index) const;
	
	// Bloqueia a troca de membros da party (ex: durante cutscenes)
	UFUNCTION(BlueprintCallable, Category = "Party")
	void SetPartySwitchingLocked(bool bLocked);

	// Verifica se a troca de membros da party está bloqueada
	UFUNCTION(BlueprintPure, Category = "Party")
	bool IsPartySwitchingLocked() const;

	// Define a lista de membros da party em posse
	UFUNCTION(BlueprintCallable, Category = "Party")
	void SetPossessedPartyMembers(const TArray<ARPGCharacter*>& NewPossessedMembers);

	// Getter para o personagem ativo
	ARPGCharacter* GetActiveCharacter() const { return ActivePartyMember; }

	// Desabilita controle de party (usado em menus, etc)
	void SetPartyControlEnabled(bool bEnabled);



protected:
	// Membros da party atualmente no grupo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	TArray<ARPGCharacter*> PartyMembers;

	// Membro da party atualmente controlado pelo jogador
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	TObjectPtr<ARPGCharacter> ActivePartyMember;

	// Índice do membro ativo na lista
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	int32 ActiveMemberIndex = -1;

	// Lista de membros da party em posse do jogador
	UPROPERTY()
	TArray<TObjectPtr<ARPGCharacter>> PossessedPartyMembers;



	// Flag que indica se a troca de party está bloqueada
	bool bIsPartySwitchingLocked = false;

	// Flag para controle do input
	bool bPartyControlEnabled = true;

	// Cache do PlayerController
	UPROPERTY()
	APlayerController* CachedPlayerController;

	// Função auxiliar para possuir o personagem ativo
	void PossessActivePartyMember();

	// Limpa o estado atual da party
	void ClearParty();

	// Encontra o próximo membro válido para troca
	int32 FindNextValidMemberIndex(int32 StartIndex, bool bForward) const;
}; 
