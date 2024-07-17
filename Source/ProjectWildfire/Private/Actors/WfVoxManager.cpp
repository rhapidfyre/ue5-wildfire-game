// Fill out your copyright notice in the Description page of Project Settings.


#include "WfVoxManager.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Components/AudioComponent.h"


FVoxCallout::FVoxCallout()
	: bSpatialAudio(false),
	  AlertLevel(0)
{
}

AWfVoxManager::AWfVoxManager()
{
	PrimaryActorTick.bCanEverTick = true;
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
	if (!IsValid(VoxDataTable))
	{
		UE_LOGFMT(LogTemp, Error, "AWfVoxManager(): VoxDataTable not specified. Vox will not work.");
		return;
	}
	TArray<FVoxData> VoxAnnouncement;
	for (const auto& VoxPhrase : VoxPhrases)
	{
		FVoxData VoxData;
		// Find the VoxPhrase (FName) in the Data Table
		if (const FVoxSounds* VoxDataRow = VoxDataTable->FindRow<FVoxSounds>(VoxPhrase, TEXT("")))
		{
			VoxData.VoxPhrase		 = VoxPhrase;
			VoxData.VoxSound		 = VoxDataRow->VoxSound;
			VoxData.Attenuation		 = VoxAttenuation;
			VoxData.bNotifyDelegates = bNotifyDelegates;
			VoxData.bSpatialAudio	 = bSpatialAudio;
			VoxData.SoundLength		 = IsValid(VoxDataRow->VoxSound) ? VoxDataRow->VoxSound->Duration : 0.25f;
			VoxData.PauseLength		 = 0.05f; // Buffer to prevent overlapping sounds
		}
		else
		{
			UE_LOGFMT(LogTemp, Error, "AWfVoxManager({NetMode}): Unable to find vox data for phrase '{VoxPhrase}'."
				, HasAuthority() ? "SRV" : "CLI", VoxPhrase.ToString());
		}
		VoxAnnouncement.Add(VoxData);
	}
	FString FullVoxPhrase = "";
	for (int i = 0; i < VoxPhrases.Num(); ++i)
	{
		FullVoxPhrase += VoxPhrases[i].ToString();
		if (i + 1 < VoxPhrases.Num())
			FullVoxPhrase += " ";
	}
	UE_LOGFMT(LogTemp, Display, "AWfVoxManager({NetMode}): Received Multicast Vox Phrase: '{FullVoxPhrase}'"
		, HasAuthority() ? "SRV" : "CLI", FullVoxPhrase);
	Speak_Internal(VoxAnnouncement);
}

void AWfVoxManager::Speak_Internal(TArray<FVoxData> VoxAnnouncement)
{
	if (VoxAnnouncement.Num() > 0)
	{
		const FVoxData VoxPhrase = VoxAnnouncement[0];
		VoxAnnouncement.RemoveAt(0);

		if (IsValid(VoxPhrase.VoxSound))
		{
			UAudioComponent* AudioComponent = nullptr;
			if (VoxPhrase.bSpatialAudio)
			{
				AudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(),
					VoxPhrase.VoxSound, GetActorLocation(), FRotator(0.0f), VoxPhrase.Volume,
					VoxPhrase.Pitch, VoxPhrase.StartOffset, VoxPhrase.Attenuation);
				UE_LOGFMT(LogTemp, Display, "AWfVoxManager({NetMode}): Playing Spatial Audio -> '{FullVoxPhrase}'"
					, HasAuthority() ? "SRV" : "CLI", VoxPhrase.VoxPhrase);
			}
			else
			{
				AudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(),
				VoxPhrase.VoxSound, VoxPhrase.Volume, VoxPhrase.Pitch, VoxPhrase.StartOffset);
				UE_LOGFMT(LogTemp, Display, "AWfVoxManager({NetMode}): Playing UI Audio -> '{FullVoxPhrase}'"
					, HasAuthority() ? "SRV" : "CLI", VoxPhrase.VoxPhrase);
			}

			if (AudioComponent)
			{
				AudioComponent->OnAudioFinished.AddDynamic(this, &AWfVoxManager::OnAudioFinished);
			}

			if (VoxPhrase.bNotifyDelegates && OnVoxAnnouncement.IsBound())
			{
				OnVoxAnnouncement.Broadcast(VoxPhrase);
			}
		}
		else
		{
			UE_LOGFMT(LogTemp, Error, "AWfVoxManager({NetMode}): Invalid sound asset for phrase {UsingPhrase}"
				, HasAuthority() ? "SRV" : "CLI", VoxPhrase.VoxPhrase.ToString());
		}

		if (VoxAnnouncement.Num() > 0)
		{
			FString FullVoxPhrase = "";
			for (int i = 0; i < VoxAnnouncement.Num(); ++i)
			{
				FullVoxPhrase += VoxAnnouncement[i].VoxPhrase.ToString();
				if (i + 1 < VoxAnnouncement.Num())
				{
					FullVoxPhrase += " ";
				}
			}

			UE_LOGFMT(LogTemp, Display, "AWfVoxManager({NetMode}): Playing remaining audio recursively -> '{RemainingPhrases}'",
				HasAuthority() ? "SRV" : "CLI");

			FTimerDelegate SpeakDelegate;
			SpeakDelegate.BindUObject(this, &AWfVoxManager::Speak_Internal, VoxAnnouncement);
			GetWorld()->GetTimerManager().SetTimer(SpeakTimer, SpeakDelegate, 0.1f, false);
		}
		else
		{
			UE_LOGFMT(LogTemp, Display, "AWfVoxManager({NetMode}): No audio phrases remaining.", HasAuthority() ? "SRV" : "CLI");
		}
		return;
	}
	SpeakTimer.Invalidate();
}

void AWfVoxManager::BeginPlay()
{
	Super::BeginPlay();
}

void AWfVoxManager::OnAudioFinished()
{
	UE_LOGFMT(LogTemp, Display, "AWfVoxManager({NetMode}): Audio playback finished.", HasAuthority() ? "SRV" : "CLI");
}
