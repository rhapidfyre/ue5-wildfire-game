// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Actor.h"
#include "Statics/WfGlobalData.h"

#include "WfVoxManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireTone, const FName&, ToneName);

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FFireTone
{
	GENERATED_BODY()
	FFireTone() {}

};

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FVoxData
{
	GENERATED_BODY()
	FVoxData(): VoxPhrase("None"),
				bSpatialAudio(false),
	            bNotifyDelegates(false),
	            VoxSound(nullptr),
	            SoundLength(0),
	            PauseLength(0),
	            Volume(1),
	            Pitch(1),
	            StartOffset(0),
	            Attenuation(nullptr)
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") FName VoxPhrase;

	// False means the sound plays from the UI
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") bool bSpatialAudio;

	// Whether event listeners will trigger
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") bool bNotifyDelegates;

	// The actual phrase to be spoken by the Vox
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") USoundBase* VoxSound;

	// The time it takes to speak the phrase
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") float SoundLength;

	// Minimum time before next phrase (in addition to SoundLength)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") float PauseLength;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") float Volume;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") float Pitch;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") float StartOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") USoundAttenuation* Attenuation;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoxAnnouncement, const FVoxData&, VoxData);

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FVoxCallout
{
	GENERATED_BODY()

	FVoxCallout();

	// If true, vox is only heard if nearby
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") bool bSpatialAudio;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") bool bNotifyDelegates;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") int AlertLevel;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") FName AreaName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") FName CallType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") FStreetAddress StreetAddress;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") TArray<FResourceNeeds> CalloutUnits;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox") FGameplayTagContainer CalloutFlags;
};


UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API AWfVoxManager : public AActor
{
	GENERATED_BODY()

public:

	AWfVoxManager();

	UFUNCTION(BlueprintCallable)
    static AWfVoxManager* GetInstance(UWorld* World);

	UFUNCTION(BlueprintPure)
	FVoxData GetVoxData(const FName& VoxPhrase);

	UFUNCTION(BlueprintCallable)
	void Server_SpeakSentence(
		const TArray<FName>& VoxPhrases, bool bSpatialAudio = false, bool bNotifyDelegates = false);

	// Called on the server to perform a Vox Announcement
	UFUNCTION(BlueprintCallable)
	void Speak(const FVoxCallout& VoxCallout);

	UFUNCTION(BlueprintCallable)
	void SpeakPhrase(
		const FName& VoxPhrase, bool bSpatialAudio = false, bool bNotifyDelegates = false);

	UFUNCTION(BlueprintCallable)
	void SpeakSentence(
		const TArray<FName>& VoxPhrases, bool bSpatialAudio = false, bool bNotifyDelegates = false);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Speak(const FVoxCallout& VoxCallout);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpeakSentence(
		const TArray<FName>& VoxPhrases, bool bSpatialAudio = false, bool bNotifyDelegates = false);

	UFUNCTION(BlueprintCallable)
	void SpeakRadio(const FString& RadioSentence);

	UFUNCTION(Server, Reliable)
	void Server_RadioCall(const TArray<FVoxData>& RadioPhrase);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_RadioCall(const TArray<FVoxData>& RadioPhrase);

protected:

	// Run on clients only
	void Speak_Internal(const TArray<FVoxData>& VoxAnnouncement);

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UFUNCTION()
	void OnAudioFinished();

    void Initialize();

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox Settings") USoundAttenuation* VoxAttenuation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox Settings") UDataTable* VoxDataTable;
	UPROPERTY(BlueprintAssignable) FOnVoxAnnouncement OnVoxAnnouncement;
	UPROPERTY(BlueprintAssignable) FOnFireTone		  OnFireTone;


private:

	void ProcessNextInQueue();
	void PlayNextPhrase();
	void LoadSoundClips(const TArray<FVoxData>& VoxAnnouncement);
	void UnloadSoundClips();

	TQueue< TArray<FVoxData> > VoxQueue;
	UPROPERTY() TMap<FName, USoundBase*> PreloadedSounds;


	static AWfVoxManager* Instance;

	UPROPERTY() UAudioComponent* AudioComponent;
	TArray<FVoxData> CurrentVox;
	int CurrentIndex;

	FTimerHandle SpeakTimer;

};
