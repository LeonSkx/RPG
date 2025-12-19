#include "Party/PartySubsystem.h"
#include "Engine/World.h"
#include "Character/RPGCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveMemberIndex = -1;
	PartyMembers.Empty();
	bIsPartySwitchingLocked = false;
	bPartyControlEnabled = true;
}

void UPartySubsystem::Deinitialize()
{
	ClearParty();
	CachedPlayerController = nullptr;
	Super::Deinitialize();
}

bool UPartySubsystem::AddPartyMember(ARPGCharacter* CharacterToAdd)
{
	if (!IsValid(CharacterToAdd) || IsInParty(CharacterToAdd))
	{
		return false;
	}

	PartyMembers.Add(CharacterToAdd);
	CharacterToAdd->bIsInParty = true;
	OnPartyMemberAdded.Broadcast(CharacterToAdd);
	OnPartyMembersChanged.Broadcast();

	if (GetActivePartyMember() == nullptr)
	{
		SwitchToPartyMember(PartyMembers.Num() - 1);
	}

	return true;
}

bool UPartySubsystem::RemovePartyMember(ARPGCharacter* CharacterToRemove)
{
	if (!CharacterToRemove || !IsInParty(CharacterToRemove))
	{
		return false;
	}

	int32 RemovedIndex = PartyMembers.Find(CharacterToRemove);

	if (CharacterToRemove == GetActivePartyMember())
	{
		// Se estamos removendo o membro ativo, precisamos trocar para outro ANTES de remover
		int32 NextIndex = FindNextValidMemberIndex(RemovedIndex, true);
		if(NextIndex == RemovedIndex) NextIndex = FindNextValidMemberIndex(RemovedIndex, false);
		SwitchToPartyMember(NextIndex);
	}
	
	CharacterToRemove->bIsInParty = false;
	PartyMembers.Remove(CharacterToRemove);
	
	// Revalida o indice ativo depois da remoção
	if (ActivePartyMember)
	{
		ActiveMemberIndex = PartyMembers.Find(ActivePartyMember);
	}
	else
	{
		ActiveMemberIndex = -1;
	}

	OnPartyMemberRemoved.Broadcast(CharacterToRemove);
	OnPartyMembersChanged.Broadcast();
	return true;
}

bool UPartySubsystem::SwitchToNextPartyMember()
{
	if (PartyMembers.Num() <= 1) return false;
	const int32 NextIndex = FindNextValidMemberIndex(ActiveMemberIndex, true);
	return SwitchToPartyMember(NextIndex);
}

bool UPartySubsystem::SwitchToNextAlivePartyMember()
{
	if (PartyMembers.Num() <= 1) return false;

	int32 StartIndex = ActiveMemberIndex;
	if (StartIndex < 0) StartIndex = 0;
	
	int32 CurrentIndex = (StartIndex + 1) % PartyMembers.Num();

	for (int32 i = 0; i < PartyMembers.Num(); i++)
	{
		ARPGCharacter* Candidate = PartyMembers[CurrentIndex];
		if (Candidate && !Candidate->IsDead())
		{
			return SwitchToPartyMember(CurrentIndex);
		}
		CurrentIndex = (CurrentIndex + 1) % PartyMembers.Num();
	}

	return false;
}

bool UPartySubsystem::SwitchToPreviousPartyMember()
{
	if (PartyMembers.Num() <= 1) return false;
	const int32 PrevIndex = FindNextValidMemberIndex(ActiveMemberIndex, false);
	return SwitchToPartyMember(PrevIndex);
}

bool UPartySubsystem::SwitchToPartyMember(int32 MemberIndex)
{
	if (bIsPartySwitchingLocked || MemberIndex == ActiveMemberIndex || !PartyMembers.IsValidIndex(MemberIndex))
	{
		return false;
	}
	
	if (ARPGCharacter* CurrentActive = GetActivePartyMember())
	{
		CurrentActive->bIsActivePartyMember = false;
		CurrentActive->OnBecomeInactivePartyMember();
	}

	ActiveMemberIndex = MemberIndex;
	ActivePartyMember = PartyMembers[ActiveMemberIndex];

	if (ActivePartyMember)
	{
		ActivePartyMember->bIsActivePartyMember = true;
		ActivePartyMember->OnBecomeActivePartyMember();
		PossessActivePartyMember();
	}

	OnActivePartyMemberChanged.Broadcast(ActivePartyMember);
	return true;
}

bool UPartySubsystem::SwitchToPartyMemberByReference(ARPGCharacter* TargetCharacter)
{
	const int32 FoundIndex = PartyMembers.Find(TargetCharacter);
	if (FoundIndex != INDEX_NONE)
	{
		return SwitchToPartyMember(FoundIndex);
	}
	return false;
}

ARPGCharacter* UPartySubsystem::GetActivePartyMember() const
{
	if (PartyMembers.IsValidIndex(ActiveMemberIndex))
	{
		return PartyMembers[ActiveMemberIndex];
	}
	return ActivePartyMember;
}

const TArray<ARPGCharacter*>& UPartySubsystem::GetPartyMembers() const
{
	return PartyMembers;
}

TArray<FString> UPartySubsystem::GetAllPartyMemberNames() const
{
	TArray<FString> Names;
	for (const ARPGCharacter* Member : PartyMembers)
	{
		if (Member)
		{
			Names.Add(Member->GetName());
		}
	}
	return Names;
}

int32 UPartySubsystem::GetActiveMemberIndex() const
{
	return ActiveMemberIndex;
}

bool UPartySubsystem::IsInParty(const ARPGCharacter* CharacterToCheck) const
{
	return PartyMembers.Contains(CharacterToCheck);
}

int32 UPartySubsystem::GetPartySize() const
{
	return PartyMembers.Num();
}

void UPartySubsystem::SetPartyMembers(const TArray<ARPGCharacter*>& NewPartyMembers)
{
	ClearParty();
	for (ARPGCharacter* Character : NewPartyMembers)
	{
		// Lógica de AddPartyMember inlined aqui
		if (IsValid(Character) && !IsInParty(Character))
		{
			PartyMembers.Add(Character);
			Character->bIsInParty = true;
			OnPartyMemberAdded.Broadcast(Character);

			if (GetActivePartyMember() == nullptr)
			{
				SwitchToPartyMember(PartyMembers.Num() - 1);
			}
		}
	}
	OnPartyMembersChanged.Broadcast();
}

void UPartySubsystem::SetActiveCharacter(ARPGCharacter* NewActiveCharacter)
{
	if (!IsInParty(NewActiveCharacter))
	{
		// Lógica de AddPartyMember inlined aqui
		if (IsValid(NewActiveCharacter))
		{
			PartyMembers.Add(NewActiveCharacter);
			NewActiveCharacter->bIsInParty = true;
			OnPartyMemberAdded.Broadcast(NewActiveCharacter);
			OnPartyMembersChanged.Broadcast(); // Broadcast de mudança na lista
		}
	}
	const int32 NewIndex = PartyMembers.Find(NewActiveCharacter);
	SwitchToPartyMember(NewIndex);
}

ARPGCharacter* UPartySubsystem::GetPartyMemberAtIndex(int32 Index) const
{
	if (PartyMembers.IsValidIndex(Index))
	{
		return PartyMembers[Index];
	}
	return nullptr;
}

void UPartySubsystem::SetPartySwitchingLocked(bool bLocked)
{
	bIsPartySwitchingLocked = bLocked;
}

bool UPartySubsystem::IsPartySwitchingLocked() const
{
	return bIsPartySwitchingLocked;
}

void UPartySubsystem::SetPossessedPartyMembers(const TArray<ARPGCharacter*>& NewPossessedMembers)
{
	PossessedPartyMembers = NewPossessedMembers;
}

void UPartySubsystem::SetPartyControlEnabled(bool bEnabled)
{
	bPartyControlEnabled = bEnabled;
}

void UPartySubsystem::PossessActivePartyMember()
{
	if (!bPartyControlEnabled) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && ActivePartyMember && PC->GetPawn() != ActivePartyMember)
	{
		// Permitir possess em todos os modos de rede para troca de personagens
		// O GAS e outros sistemas cuidarão da sincronização
		PC->Possess(ActivePartyMember);
	}
}

void UPartySubsystem::ClearParty()
{
	if (ARPGCharacter* CurrentActive = GetActivePartyMember())
	{
		CurrentActive->bIsActivePartyMember = false;
		CurrentActive->OnBecomeInactivePartyMember();
	}
	for (ARPGCharacter* Member : PartyMembers)
	{
		if (Member)
		{
			Member->bIsInParty = false;
		}
	}
	PartyMembers.Empty();
	ActiveMemberIndex = -1;
	ActivePartyMember = nullptr;
	OnPartyMembersChanged.Broadcast();
}

int32 UPartySubsystem::FindNextValidMemberIndex(int32 StartIndex, bool bForward) const
{
	const int32 PartyNum = PartyMembers.Num();
	if (PartyNum <= 1)
	{
		return StartIndex;
	}

	int32 CurrentIndex = (StartIndex < 0) ? 0 : StartIndex;
	
	for (int32 i = 0; i < PartyNum; ++i)
	{
		if (bForward)
		{
			CurrentIndex = (CurrentIndex + 1) % PartyNum;
		}
		else
		{
			CurrentIndex = (CurrentIndex - 1 + PartyNum) % PartyNum;
		}

		if (PartyMembers.IsValidIndex(CurrentIndex) && IsValid(PartyMembers[CurrentIndex]))
		{
			return CurrentIndex;
		}
	}

	// Se não encontrar nenhum outro válido, retorna o índice atual se ele for válido
	if (PartyMembers.IsValidIndex(StartIndex) && IsValid(PartyMembers[StartIndex]))
	{
		return StartIndex;
	}

	return -1; // Nenhum membro válido encontrado
}