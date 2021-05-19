#pragma once

#include "CoreMinimal.h"
#include "Core/Public/HAL/Runnable.h"


class FRunnableThread;
class AVoxelActor;


class FChunkBuilderCalculation : public FRunnable
{

public:
	FChunkBuilderCalculation(
		AVoxelActor* TestActor,
		const int32 InnerchunkLineElements,
		const int32 InnerchunkZElements,
		const int32 InnerchunkLineElementsP2,
		const int32 InnerchunkTotalElements,
		const int32 InnerchunkXIndex,
		const int32 InnerchunkYIndex
	);

	UPROPERTY()
		TArray<int32> chunkFields;

	UPROPERTY()
		TArray<FIntVector> treeCenters;

	bool bStopThread;

	int32 chunkLineElements;
	int32 chunkZElements;
	int32 chunkLineElementsP2;
	int32 chunkTotalElements;
	int32 chunkXIndex;
	int32 chunkYIndex;

	FRandomStream RandomStream = FRandomStream(12);

	virtual bool Init();

	virtual uint32 Run();

	virtual void Stop();

	bool IsThreadActive();

	void GenerateChunk();

	int32 calculateNoiseLandscape2d(int x, int y);
	int32 calculateNoiseBedrock2d(int x, int y);
	int32 calculateNoiseCaves3d(int x, int y, int z);

	void LakeBuilder(int32 z_axis_min);

	float calculateNoiseLake2d(int x, int y);

	bool checkRange(int32 value, int32 range);

private:
	AVoxelActor* CurrentThreadActor;
};