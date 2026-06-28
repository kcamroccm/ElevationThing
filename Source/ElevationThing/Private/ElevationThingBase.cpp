#include "ElevationThingBase.h"
#include "DrawDebugHelpers.h"

// Sets default values
AElevationThingBase::AElevationThingBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root"));

	ElevationPivot = CreateDefaultSubobject<USceneComponent>("ElevationPivot");
	ElevationPivot->SetupAttachment(RootComponent);
	TubeBipodPivot = CreateDefaultSubobject<USceneComponent>("Bipod Pivot");
	TubeBipodCenter = CreateDefaultSubobject<USceneComponent>("Tube Center Over Bipod");
	TubeBipodCenter->SetupAttachment(ElevationPivot);
	TubeBipodPivot->SetupAttachment(TubeBipodCenter);
	BipodMeshParent = CreateDefaultSubobject<USceneComponent>("BipodMeshAttach");
	BipodMeshParent->SetupAttachment(TubeBipodPivot);
}

// Called when the game starts or when spawned
void AElevationThingBase::BeginPlay()
{
	Super::BeginPlay(); 
	InitializeGeoParameters();
	UpdateElevation(InitElevation);
}

void AElevationThingBase::InitializeGeoParameters()
{
	GeoParams.ElevationPivotHeight = (ElevationPivot->GetRelativeLocation() - RootComponent->GetRelativeLocation()).Z;
	//because i made this a child of Bipod Center
	// or 
	//GetRootComponent()->GetComponentTransform().InverseTransformPosition(TubeBipodPivot->GetComponentLocation());
	GeoParams.BipodPivotOffset = (TubeBipodPivot->GetRelativeLocation()).Length();
	GeoParams.TubeLength = (TubeBipodCenter->GetRelativeLocation()).Length();
	GeoParams.PivotToBipodPivot = (TubeBipodPivot->GetRelativeLocation() + TubeBipodCenter->GetRelativeLocation()).Length();
	GeoParams.PivotToBipodPivotAngle = FMath::RadiansToDegrees(FMath::Atan(GeoParams.BipodPivotOffset / GeoParams.TubeLength)); 
}

// Called every frame
void AElevationThingBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#define DEBUG_LINES 0
void AElevationThingBase::UpdateBipodHeight(float NewBipodHeight)
{
	GeoParams.UpdateBipodHeight(NewBipodHeight);
	UpdateVisuals();
}

void AElevationThingBase::UpdateElevation(float NewElevation)
{
	GeoParams.UpdateElevation(NewElevation);
	UpdateVisuals();
#if DEBUG_LINES
	if (GetWorld())
	{
		FVector PivotWorldLoc{ TubeBipodPivot->GetComponentLocation() };
		FVector TubeHeightWorldLoc{ TubeBipodCenter->GetComponentLocation() };
		FVector EndLocation{ PivotWorldLoc + (FVector::DownVector * GeoParams.PivotToGround)};
		FVector EndLocation2{ TubeHeightWorldLoc + (FVector::DownVector * GeoParams.OffsetRelativeToHeight)};
		FVector EndLocation3{ TubeHeightWorldLoc + (FVector::DownVector * GeoParams.TubeHeightPivotCenter)};

		DrawDebugLine(
			GetWorld(),
			                 // Current World context
			TubeHeightWorldLoc,              // Start position (FVector)
			EndLocation3,                // End position (FVector)
			FColor::Yellow,                // Line Color
			false,                      // Persistent lines (true = stays forever)
			0.f,                       // Lifetime in seconds (ignored if persistent)
			0,                          // Depth priority (0 = default)
			.5f                        // Thickness of the line in world units
		);

		DrawDebugLine(
			GetWorld(),
			                 // Current World context
			PivotWorldLoc,              // Start position (FVector)
			EndLocation,                // End position (FVector)
			FColor::Purple,                // Line Color
			false,                      // Persistent lines (true = stays forever)
			0.f,                       // Lifetime in seconds (ignored if persistent)
			0,                          // Depth priority (0 = default)
			.5f                        // Thickness of the line in world units
		);
		DrawDebugLine(
			GetWorld(),
			                 // Current World context
			TubeHeightWorldLoc,              // Start position (FVector)
			EndLocation2,                // End position (FVector)
			FColor::Red,                // Line Color
			false,                      // Persistent lines (true = stays forever)
			0.f,                       // Lifetime in seconds (ignored if persistent)
			0,                          // Depth priority (0 = default)
			.5f                        // Thickness of the line in world units
		);
	}
#endif
}

void FElevationGeoParameters::UpdateElevation(float NewElevation)
{
	Elevation = NewElevation;

	TubeHeightPivotCenter = TubeLength * FMath::Sin(FMath::DegreesToRadians(Elevation)) + ElevationPivotHeight;
	OffsetRelativeToHeight = BipodPivotOffset * FMath::Cos(FMath::DegreesToRadians(Elevation));
	//UE_LOG(LogTemp, Warning, TEXT("El: %f, Offset: %f"), Elevation, OffsetRelativeToHeight);
	PivotToGround = TubeHeightPivotCenter - OffsetRelativeToHeight;
	const float AngleFromNormal = FMath::RadiansToDegrees(FMath::Acos(FMath::Min(1.f, PivotToGround / BipodLength)));
	const float OppositeAngle = 180.f - (90.f + Elevation);
	BipodAngle = OppositeAngle + AngleFromNormal;

	PivotToBipodTouchPoint =
		FMath::Sqrt(
			FMath::Square(BipodLength)  +  FMath::Square(PivotToBipodPivot) - (2 * BipodLength * PivotToBipodPivot * FMath::Cos(FMath::DegreesToRadians(BipodAngle + PivotToBipodPivotAngle)))
		);
	PivotToTouchPointAngle = FMath::RadiansToDegrees(FMath::Asin(ElevationPivotHeight / PivotToBipodTouchPoint));

	//UE_LOG(LogTemp, Warning, TEXT("Pivot to bipod Pivot: %f, Pivot to bipod ground %f , bipod height %f"), PivotToBipodPivot, PivotToBipodTouchPoint, BipodLength);
}

void FElevationGeoParameters::UpdateBipodHeight(float NewBipodHeight)
{
	BipodLength = NewBipodHeight;
	float Numerator = (FMath::Square(PivotToBipodPivot) + FMath::Square(PivotToBipodTouchPoint) - FMath::Square(BipodLength));
	float Denominator = (2 * PivotToBipodPivot * PivotToBipodTouchPoint);
	Elevation = FMath::RadiansToDegrees(FMath::Acos(Numerator / Denominator)) + PivotToBipodPivotAngle - PivotToTouchPointAngle;
	Numerator = ( FMath::Square(PivotToBipodPivot) + FMath::Square(BipodLength) - FMath::Square(PivotToBipodTouchPoint));
	Denominator = (2 * BipodLength * PivotToBipodPivot);
	BipodAngle = FMath::RadiansToDegrees(FMath::Acos(Numerator / Denominator)) - PivotToBipodPivotAngle;

	//UE_LOG(LogTemp, Warning, TEXT("New Elevation %f"), Elevation);
	//UpdateElevation(NewElevation);
}
