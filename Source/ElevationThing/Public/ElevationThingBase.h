// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElevationThingBase.generated.h"

USTRUCT(BlueprintType)
struct ELEVATIONTHING_API FElevationGeoParameters
{
	GENERATED_BODY()
public:
	//Constant per model
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ElevationPivotHeight = 0.f;

	//Constant per model
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BipodPivotOffset = 0.f;
	//Constant per model
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TubeLength = 0.f;// to the pivot center

	//Constant per model computed based on other properties
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PivotToBipodPivot = 0.f;// to the pivot center

	//this is a bit that can extend
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BipodLength = 0.f;// to the pivot center


	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float Elevation = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BipodAngle = 0.f;

	//computed during init
	float PivotToBipodPivotAngle{0};

	void UpdateElevation(float NewElevation);
	void UpdateBipodHeight(float NewBipodHeight);

	//maybe don't need these
protected:
	//used to compute bipod angle when elevation is adjusted
	//these could be locals in the related function made them members for debugging;
	float PivotToGround = 0.f;
	float OffsetRelativeToHeight = 0.f;
	float TubeHeightPivotCenter = 0.f;

	//bottom of the triangle computed when elevation changes
	float PivotToBipodTouchPoint{0};
	float PivotToTouchPointAngle{0}; // ASin(ElevationPivotHeight /PivotToBipodTouchPoint )

};

UCLASS()
class ELEVATIONTHING_API AElevationThingBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AElevationThingBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> ElevationPivot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> TubeBipodPivot;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USceneComponent> TubeBipodCenter;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> BipodMeshParent;

	UPROPERTY(EditAnyWhere)
	float InitElevation {20.f};
	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	FElevationGeoParameters GeoParams;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void InitializeGeoParameters();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateVisuals();

	UFUNCTION(BlueprintCallable)
	void UpdateElevation(float NewElevation);
	UFUNCTION(BlueprintCallable)
	void UpdateBipodHeight(float NewHeight);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
