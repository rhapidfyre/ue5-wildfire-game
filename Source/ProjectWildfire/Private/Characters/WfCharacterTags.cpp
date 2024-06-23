// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/WfCharacterTags.h"


UE_DEFINE_GAMEPLAY_TAG(TAG_Gender,				"Game.Character.Sex")
UE_DEFINE_GAMEPLAY_TAG(TAG_Gender_Male,			"Game.Character.Sex.Male")
UE_DEFINE_GAMEPLAY_TAG(TAG_Gender_Female,		"Game.Character.Sex.Female")
UE_DEFINE_GAMEPLAY_TAG(TAG_Gender_Non_Binary,   "Game.Character.Sex.NonBinary")

UE_DEFINE_GAMEPLAY_TAG(TAG_Ethnicity,					"Game.Character.Race");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ethnicity_White,			    "Game.Character.Race.White");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ethnicity_Black,			    "Game.Character.Race.Black");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ethnicity_Hispanic,          "Game.Character.Race.Hispanic");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ethnicity_Asian,				"Game.Character.Race.Asian");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ethnicity_NativeAmerican,    "Game.Character.Race.NativeAmerican");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ethnicity_PacificIslander,   "Game.Character.Race.PacificIslander");

// Primary Role Categories

UE_DEFINE_GAMEPLAY_TAG(TAG_Role,				"Game.Character.Role")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Player,		    "Game.Character.Role.Player")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Police,		    "Game.Character.Role.Police")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Civilian,		"Game.Character.Role.Civilian")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire,			"Game.Character.Role.Fire")

// Sub-roles; More specified roles describing a primary category

UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_Arson,		"Game.Character.Role.Fire.Arson")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_Pilot,		"Game.Character.Role.Fire.Pilot")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_Div,		"Game.Character.Role.Fire.Division")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_Chief,		"Game.Character.Role.Fire.Chief")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_Cpt,	    "Game.Character.Role.Fire.Captain")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_Eng,		"Game.Character.Role.Fire.Engineer")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_Two,		"Game.Character.Role.Fire.Firefighter_II")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_One,		"Game.Character.Role.Fire.Firefighter_I")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_New,		"Game.Character.Role.Fire.Candidate")
UE_DEFINE_GAMEPLAY_TAG(TAG_Role_Fire_Inmate,	"Game.Character.Role.Fire.Inmate")
