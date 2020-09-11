
#include "ElevatingActionPistol.h"
#include "ElevatingActionSecretAgent.h"
#include "Sound/SoundCue.h"

AElevatingActionPistol::AElevatingActionPistol()
{
    PrimaryActorTick.bCanEverTick = false;
    
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> PistolMesh(TEXT("SkeletalMesh'/Game/PolygonSpy/Meshes/Weapons/SK_Wep_Pistol_01.SK_Wep_Pistol_01'"));
    if (PistolMesh.Succeeded())
    {
        GetSkeletalMeshComponent()->SetSkeletalMesh(PistolMesh.Object);
        GetSkeletalMeshComponent()->SetCollisionProfileName(TEXT("NoCollision"));
        GetSkeletalMeshComponent()->SetSimulatePhysics(false);

        ConstructorHelpers::FObjectFinder<UClass> DefaultProjectile(TEXT("Blueprint'/Game/ToonProjectilesVol1/Blueprints/BP_Projectile_Bullet01Red.BP_Projectile_Bullet01Red_C'"));
        if (DefaultProjectile.Succeeded())
            Projectile = DefaultProjectile.Object;
    }

    ProjectileShotAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ProjectileShotAudioComponent"));
    ProjectileShotAudioComponent->SetUISound(true);
    ProjectileShotAudioComponent->SetAutoActivate(false);

    static ConstructorHelpers::FObjectFinder<USoundCue> ProjectileShotSound(TEXT("SoundCue'/Game/ElevatingActionAudio/GameMasterAudio/PistolTriggerPulledSounds/S_ProjectileShot_Cue.S_ProjectileShot_Cue'"));
    if (ProjectileShotSound.Succeeded())
        ProjectileShotAudioComponent->SetSound(ProjectileShotSound.Object);
    
}

void AElevatingActionPistol::PullTrigger()
{
    AElevatingActionSecretAgent* SecretAgent = Cast<AElevatingActionSecretAgent>(GetOwner());
    if (SecretAgent)
    {
        FActorSpawnParameters ProjectileSpawnParameters;
        ProjectileSpawnParameters.Instigator = SecretAgent;
        GetWorld()->SpawnActor<AActor>(Projectile, GetSkeletalMeshComponent()->GetSocketTransform(TEXT("SK_Wep_Projectile_Socket")), ProjectileSpawnParameters);
        ProjectileShotAudioComponent->Play();
        
        SecretAgent->ProjectilesShotCount++;
        SecretAgent->SetShootButtonPressed(false);
    }
}