
#include "ElevatingActionOfficeDoor.h"

UElevatingActionOfficeDoor::UElevatingActionOfficeDoor()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> OfficeDoorFrameRef(TEXT("/Game/ElevatingActionOffice/Materials/M_ElevatingActionOffice_Wall"));
	if (OfficeDoorFrameRef.Succeeded())
		FrameMaterialInstance = OfficeDoorFrameRef.Object;
	
	bIsClosed = true;
	DoorRotationSpeed = 150.0f;
}

void UElevatingActionOfficeDoor::PostInitProperties()
{
	Super::PostInitProperties();

	if (FrameMaterialInstance)
	{
		FrameMaterial = UMaterialInstanceDynamic::Create(FrameMaterialInstance, GetOwner(), TEXT("M_ElevatingActionOffice_DoorFrame"));
		FrameMaterial->GetScalarParameterValue(TEXT("Base_EmissiveMultiplier"), FrameBrightness);
		SetMaterial(1, FrameMaterial);
	}
}

void UElevatingActionOfficeDoor::OnComponentCreated()
{
	if (GetOwner())
	{
		if (GetOwner()->GetActorLocation().X > 0.0f)
			SetRelativeLocationAndRotation(FVector(75.0f, 20.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
		else
			SetRelativeLocationAndRotation(FVector(175.0f, 10.0f, 0.0f), FRotator(0.0f, 180.0f, 0.0f));
	}
}

void UElevatingActionOfficeDoor::BeginPlay()
{
	DoorClosedRotation = GetRelativeRotation();
	DoorTargetRotation = DoorClosedRotation;
}

void UElevatingActionOfficeDoor::SetFrameBrightness(float Brightness)
{
	if (FrameMaterial)
	{
		FrameBrightness = Brightness;

		FrameMaterial->SetScalarParameterValue(TEXT("Base_EmissiveMultiplier"), FrameBrightness);
		SetMaterial(1, FrameMaterial);
	}
}

float UElevatingActionOfficeDoor::GetFrameBrightness()
{
	return FrameBrightness;
}

bool UElevatingActionOfficeDoor::IsClosed() const
{
	return bIsClosed;
}

bool UElevatingActionOfficeDoor::IsLocked() const
{
	return GetRelativeRotation().Equals(DoorTargetRotation);
}

void UElevatingActionOfficeDoor::Open()
{
	DoorTargetRotation = FRotator(0.0f, -90.0f, 0.0f);
}

void UElevatingActionOfficeDoor::Close()
{
	DoorTargetRotation = DoorClosedRotation;
}

void UElevatingActionOfficeDoor::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	FRotator DoorRotation = FMath::RInterpConstantTo(GetRelativeRotation(), DoorTargetRotation, DeltaTime, DoorRotationSpeed);
	SetRelativeRotation(DoorRotation);

	if (IsLocked())
		bIsClosed = DoorTargetRotation.Equals(DoorClosedRotation);
}


