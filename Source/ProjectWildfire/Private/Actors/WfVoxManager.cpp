// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WfVoxManager.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Components/AudioComponent.h"
#include "Statics/WfGameModeBase.h"


FVoxCallout::FVoxCallout()
	: bSpatialAudio(false),
	  bNotifyDelegates(false),
	  AlertLevel(0)
{
}

AWfVoxManager* AWfVoxManager::Instance = nullptr;

AWfVoxManager::AWfVoxManager()
	: VoxAttenuation(nullptr),
	  VoxDataTable(nullptr),
	  AudioComponent(nullptr),
	  CurrentIndex(0)
{
	PrimaryActorTick.bCanEverTick = true;
}

AWfVoxManager* AWfVoxManager::GetInstance(UWorld* World)
{
	if (!Instance)
	{
		// Try to find an existing instance in the world
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(World, AWfVoxManager::StaticClass(), FoundActors);

		if (FoundActors.Num() > 0)
		{
			Instance = Cast<AWfVoxManager>(FoundActors[0]);
		}
		else
		{
			// Spawn a new instance if none exists
			Instance = World->SpawnActor<AWfVoxManager>();
		}
	}

	return Instance;
}

FVoxData AWfVoxManager::GetVoxData(const FName& VoxPhrase)
{
	if (!IsValid(VoxDataTable))
	{
		if (const AWfGameModeBase* GameMode = Cast<AWfGameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			VoxDataTable = GameMode->VoxTable;
		}
	}

	if (!IsValid(VoxDataTable))
	{
		UE_LOGFMT(LogTemp, Error, "Vox Data Table Not Set!");
		return {};
	}

	FName VoxPhraseLower = FName(VoxPhrase.ToString().ToLower());
	// Find the VoxPhrase (FName) in the Data Table
	if (const FVoxSounds* VoxDataRow = VoxDataTable->FindRow<FVoxSounds>(VoxPhraseLower, TEXT("")))
	{
		FVoxData VoxData;
		VoxData.VoxPhrase		 = VoxPhrase;
		VoxData.VoxSound		 = VoxDataRow->VoxSound;
		VoxData.Attenuation		 = VoxAttenuation;
		VoxData.SoundLength		 = IsValid(VoxDataRow->VoxSound) ? VoxDataRow->VoxSound->Duration : 0.25f;
		VoxData.PauseLength		 = 0.06f; // Buffer to prevent overlapping sounds
		return VoxData;
	}
	UE_LOGFMT(LogTemp, Error, "AWfVoxManager({NetMode}): Unable to find vox data for phrase '{VoxPhrase}'."
		, HasAuthority() ? "SRV" : "CLI", VoxPhraseLower.ToString());
	return {};
}

void AWfVoxManager::Server_SpeakSentence(const TArray<FName>& VoxPhrases, bool bSpatialAudio, bool bNotifyDelegates)
{
	SpeakSentence(VoxPhrases, bSpatialAudio, bNotifyDelegates);
}

/**
 * \brief Tells the Vox all the needed information to dispatch a callout
 * \param VoxCallout The data for the callout
 */
void AWfVoxManager::Speak(const FVoxCallout& VoxCallout)
{
	Multicast_Speak(VoxCallout);
}

/**
 * \brief Makes the vox speak a single word or phrase
 * \param VoxPhrase The phrase for the Vox to speak
 * \param bSpatialAudio True for the sound to come from the entity in 3d space, false plays as UI sound
 * \param bNotifyDelegates False plays from this actor only
 */
void AWfVoxManager::SpeakPhrase(const FName& VoxPhrase, bool bSpatialAudio, bool bNotifyDelegates)
{
	const TArray VoxPhrases({VoxPhrase});
	SpeakSentence(VoxPhrases, bSpatialAudio, bNotifyDelegates);
}

/**
 * \brief Makes the Vox speak a collection of phrases as one consecutive statement
 * \param VoxPhrases An array of phrases for the Vox to use, which will be consecutively
 * \param bSpatialAudio True for the sound to come from the entity in 3d space, false plays as UI sound
 * \param bNotifyDelegates False plays from this actor only
 */
void AWfVoxManager::SpeakSentence(const TArray<FName>& VoxPhrases, bool bSpatialAudio, bool bNotifyDelegates)
{
	TArray<FString> VoxStrings;
	for (const auto& VoxStatement : VoxPhrases)
	{
		VoxStrings.Add(VoxStatement.ToString());
	}
	UE_LOGFMT(LogTemp, Display, "AWfVoxManager({NetMode}): SpeakPhrase() = '{VoxPhrases}'"
			  , HasAuthority() ? "SRV" : "CLI", VoxStrings);
	Multicast_SpeakSentence(VoxPhrases, bSpatialAudio, bNotifyDelegates);
}

void AWfVoxManager::SpeakRadio(const FString& RadioSentence)
{
	FString NewSentence = RadioSentence;

	// Replace blanks, commas, and period
	FString FinalSentence = NewSentence
		.Replace(TEXT(","), TEXT("_comma"))		// replace commas
		.Replace(TEXT("-"), TEXT(" _blank "))	// replace hyphens
		.Replace(TEXT(" _ "), TEXT(" _blank "))	// replace underscores
		.Replace(TEXT("."), TEXT("_period"))	// Replace periods
	;

	// Replace special characters with a blank space
	FString ProcessedSentence;
	bool bPreviousWhitespace = false;
	for (TCHAR& Char : FinalSentence)
	{
		if (FChar::IsWhitespace(Char) || (!FChar::IsAlnum(Char) && Char != '_' && Char != ' '))
		{
			if (!bPreviousWhitespace)
			{
				ProcessedSentence.AppendChar(' ');
				bPreviousWhitespace = true;
			}
		}
		else
		{
			ProcessedSentence.AppendChar(Char);
			bPreviousWhitespace = false;
		}
	}

	// Build Transmission
	TArray<FString> Words;
	ProcessedSentence.ParseIntoArray(Words, TEXT(" "), true);
	Words.Insert("start_tx", 0);
	Words.Add("stop_tx");

	// Convert numbers to be read properly
	TArray<FVoxData> VoxDataArray;
    for (FString& Word : Words)
    {
        if (Word.IsNumeric())
        {
            int32 Number = FCString::Atoi(*Word);
            FString NumberStr = FString::FromInt(Number);
            int32 Length = NumberStr.Len();
            bool FirstChunk = true;

            // Process the number from right to left in chunks of two digits
            for (int32 i = Length; i > 0; i -= 2)
            {
                int32 EndIndex = i;
                int32 StartIndex = FMath::Max(0, i - 2);
                FString Chunk = NumberStr.Mid(StartIndex, EndIndex - StartIndex);

                // Convert to names with leading zeros if needed (to ensure "05" is "zero five")
                if (Chunk.Len() == 1 && !FirstChunk)
                {
                    Chunk = FString("0") + Chunk;
                }

                // Handle tens and units separately for pronunciation
                int32 ChunkNumber = FCString::Atoi(*Chunk);
                if (ChunkNumber < 10 && !FirstChunk)
                {
                    // Single digit
                    VoxDataArray.Add(GetVoxData(FName(*FString::Printf(TEXT("0%d"), ChunkNumber))));
                }
                else if (ChunkNumber < 20)
                {
                    // Numbers 10-19
                    for (TCHAR Digit : Chunk)
                    {
                        FString DigitStr(1, &Digit);
                        FName DigitName(*DigitStr);
                        VoxDataArray.Add(GetVoxData(DigitName));
                    }
                }
                else
                {
                    // Tens and units
                    int32 Tens = ChunkNumber / 10;
                    int32 Units = ChunkNumber % 10;
                    VoxDataArray.Add(GetVoxData(FName(*FString::Printf(TEXT("%d0"), Tens))));
                    if (Units > 0)
                    {
                        VoxDataArray.Add(GetVoxData(FName(*FString::Printf(TEXT("%d"), Units))));
                    }
                }

                FirstChunk = false;
            }
        }
        else
        {
            // Process non-numeric words
            VoxDataArray.Add(GetVoxData(FName(*Word)));
        }
    }

	FString RadioMessage;
	for (int i = 0; i < VoxDataArray.Num(); ++i)
	{
		RadioMessage += VoxDataArray[i].VoxPhrase.ToString();
		if (i + 1 < VoxDataArray.Num())
			RadioMessage += " ";
	}
	UE_LOGFMT(LogTemp, Display, "VoxManager({NetMode}): RADIO '{RadioMessage}'"
		, HasAuthority() ? "SRV" : "CLI", RadioMessage);
	Multicast_RadioCall(VoxDataArray);
}

/**
 * \brief Sents callout data to the Vox for automatic handling
 * \param VoxCallout The callout data for the Vox to dispatch
 */
void AWfVoxManager::Multicast_Speak_Implementation(const FVoxCallout& VoxCallout)
{

}

/**
 * \brief Makes the Vox speak a collection of phrases as one consecutive statement
 * \param VoxPhrases An array of phrases for the Vox to use, which will be consecutively
 * \param bSpatialAudio True for the sound to come from the entity in 3d space, false plays as UI sound
 * \param bNotifyDelegates False plays from this actor only
 */
void AWfVoxManager::Multicast_SpeakSentence_Implementation(
	const TArray<FName>& VoxPhrases, bool bSpatialAudio, bool bNotifyDelegates)
{
	TArray<FVoxData> VoxAnnouncement;
	for (const auto& VoxPhrase : VoxPhrases)
	{
		FVoxData VoxData = GetVoxData(VoxPhrase);
		VoxData.bSpatialAudio = bSpatialAudio;
		VoxData.bNotifyDelegates = bNotifyDelegates;
		VoxAnnouncement.Add(VoxData);
	}

	FString FullVoxPhrase = "";
	for (int i = 0; i < VoxPhrases.Num(); ++i)
	{
		FullVoxPhrase += VoxPhrases[i].ToString();
		if (i + 1 < VoxPhrases.Num())
			FullVoxPhrase += " ";
	}
	Speak_Internal(VoxAnnouncement);
}

void AWfVoxManager::Multicast_RadioCall_Implementation(const TArray<FVoxData>& RadioPhrase)
{
	Speak_Internal(RadioPhrase);
}

void AWfVoxManager::Server_RadioCall_Implementation(const TArray<FVoxData>& RadioPhrase)
{
	// TODO - For now, just allow it.
	Multicast_RadioCall(RadioPhrase);
}

void AWfVoxManager::Speak_Internal(const TArray<FVoxData>& VoxAnnouncement)
{
	if (AudioComponent && AudioComponent->IsPlaying())
	{
		VoxQueue.Enqueue(VoxAnnouncement);
		UE_LOGFMT(LogTemp, Warning, "VoxManager(): Received Vox request while playing. Placed into queue.");
	}
	else if (VoxAnnouncement.Num() > 0)
	{
		LoadSoundClips(VoxAnnouncement);
		CurrentVox = VoxAnnouncement;
		CurrentIndex = 0;
		PlayNextPhrase();
	}
}

void AWfVoxManager::BeginPlay()
{
	Super::BeginPlay();
	if (!Instance)
	{
		Initialize();
	}
}

void AWfVoxManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (Instance == this)
	{
		Instance = nullptr;
	}
}

void AWfVoxManager::OnAudioFinished()
{
	CurrentIndex++;
	if (CurrentIndex < CurrentVox.Num())
	{
		const FVoxData& VoxPhrase = CurrentVox[CurrentIndex];
		if (VoxPhrase.PauseLength > 0)
		{
			GetWorld()->GetTimerManager().SetTimer(SpeakTimer, this,
				&AWfVoxManager::PlayNextPhrase, VoxPhrase.PauseLength, false);
		}
		else
		{
			PlayNextPhrase();
		}
	}
	else
	{
		UnloadSoundClips();

		// Wait 3 seconds before executing the next announcement in the queue
		GetWorld()->GetTimerManager().SetTimer(SpeakTimer, this,
			&AWfVoxManager::ProcessNextInQueue, 3.0f, false);
	}
}

void AWfVoxManager::ProcessNextInQueue()
{
	if (!VoxQueue.IsEmpty())
	{
		TArray<FVoxData> NextVoxAnnouncement;
		VoxQueue.Dequeue(NextVoxAnnouncement);
		Speak_Internal(NextVoxAnnouncement);
	}
}

void AWfVoxManager::Initialize()
{
    Instance = this;
}

void AWfVoxManager::PlayNextPhrase()
{
	if (CurrentIndex < CurrentVox.Num())
	{
		const FVoxData& VoxPhrase = CurrentVox[CurrentIndex];

		if (IsValid(VoxPhrase.VoxSound))
		{
			if (!AudioComponent)
			{
				AudioComponent = NewObject<UAudioComponent>(this);
				AudioComponent->OnAudioFinished.AddDynamic(this, &AWfVoxManager::OnAudioFinished);
			}

			AudioComponent->SetSound(VoxPhrase.VoxSound);
			AudioComponent->SetVolumeMultiplier(VoxPhrase.Volume);
			AudioComponent->SetPitchMultiplier(VoxPhrase.Pitch);
			AudioComponent->Play(VoxPhrase.StartOffset);

			UE_LOG(LogTemp, Display, TEXT("AWfVoxManager(%s): Playing %s Audio -> '%s' (%f s)"),
				HasAuthority() ? TEXT("SRV") : TEXT("CLI"),
				VoxPhrase.bSpatialAudio ? TEXT("Spatial") : TEXT("UI"),
				*VoxPhrase.VoxPhrase.ToString(), VoxPhrase.SoundLength);

			if (VoxPhrase.bNotifyDelegates && OnVoxAnnouncement.IsBound())
			{
				OnVoxAnnouncement.Broadcast(VoxPhrase);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AWfVoxManager(%s): Invalid sound asset for phrase %s"),
				HasAuthority() ? TEXT("SRV") : TEXT("CLI"), *VoxPhrase.VoxPhrase.ToString());
			OnAudioFinished(); // Skip to the next phrase
		}
	}
}

void AWfVoxManager::LoadSoundClips(const TArray<FVoxData>& VoxAnnouncement)
{
	for (const FVoxData& VoxPhrase : VoxAnnouncement)
	{
		if (VoxPhrase.VoxSound)
		{
			FName SoundName = VoxPhrase.VoxSound->GetFName();
			if (!PreloadedSounds.Contains(SoundName))
			{
				PreloadedSounds.Add(SoundName, VoxPhrase.VoxSound);
				VoxPhrase.VoxSound->AddToRoot(); // Prevents garbage collection
			}
		}
	}
}

void AWfVoxManager::UnloadSoundClips()
{
	for (auto& Elem : PreloadedSounds)
	{
		if (Elem.Value)
		{
			Elem.Value->RemoveFromRoot();
		}
	}
	PreloadedSounds.Empty();
}
