#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ChunkBuilderCalculation.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "VoxelActor.generated.h"

struct FMeshSection {
    TArray<FVector>Vertics;
    TArray<int32>Triangles;
    TArray<FVector>Normals;
    TArray<FVector2D>UVs;
    TArray<FProcMeshTangent>Tangents;
    TArray<FColor>VertexColor;
    int32 elementId = 0;
};

UCLASS()
class VOXELGAME_API AVoxelActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AVoxelActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray <UMaterialInterface*> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EkjxposeOnSpawn = true))
        int32 randomSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
        int32 voxelSize = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
        int32 chunkLineElements = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
        int32 chunkXIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
        int32 chunkYIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float xMult = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float yMult = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float zMult = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float weight = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float freq = 1;

    UPROPERTY()
        int32 chunkTotalElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
        int32 chunkZElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
        int32 chunkLineElementsP2;

    UPROPERTY()
        int32 voxelSizeHalf;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EkjxposeOnSpawn = true))
        TArray <int32> chunkFields;

    UPROPERTY()
        UProceduralMeshComponent* proceduralComponent;

    UFUNCTION(BlueprintCallable)
        void setVoxel(FVector localPos, int32 value);

    UFUNCTION(BlueprintImplementableEvent)
        void AddInstanceVoxel(FVector instanceLocation, int32 meshType);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    int32 ProcessedCalculation;

    class FChunkBuilderCalculation* CalcThread = nullptr;

    FRunnableThread* CurrentRunningThread = nullptr;

    FTimerHandle ChunkTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    virtual void OnConstruction(const FTransform& Transform) override;

    void GenerateChunk();

    void UpdateMesh();

    void StartThread();

    void CheckChunkBuilder();
};
