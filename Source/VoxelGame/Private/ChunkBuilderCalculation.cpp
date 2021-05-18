// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkBuilderCalculation.h"
#include "VoxelActor.h"
#include "SimplexNoise.h"


FChunkBuilderCalculation::FChunkBuilderCalculation(
	AVoxelActor* TestActor,
	const int32 InnerchunkLineElements,
	const int32 InnerchunkZElements,
	const int32 InnerchunkLineElementsP2,
	const int32 InnerchunkTotalElements,
	const int32 InnerchunkXIndex,
	const int32 InnerchunkYIndex
)
{
	if (TestActor)
	{
		CurrentThreadActor = TestActor;
		chunkLineElements = InnerchunkLineElements;
		chunkZElements = InnerchunkZElements;
		chunkLineElementsP2 = InnerchunkLineElementsP2;
		chunkTotalElements = InnerchunkTotalElements;
		chunkXIndex = InnerchunkXIndex;
		chunkYIndex = InnerchunkYIndex;
	}
}

bool FChunkBuilderCalculation::Init()
{
	bStopThread = false;

	return true;
}

void FChunkBuilderCalculation::GenerateChunk()
{
	chunkFields.SetNumUninitialized(chunkTotalElements);

	int32 index_3d = 0;
	int32 noise_3d_val = 0;
	int32 noise_landscape_2d_val = 0;
	int32 noise_bedrock_2d_val = 0;

	for (int x = 0; x < chunkLineElements; x++)
	{
		for (int y = 0; y < chunkLineElements; y++)
		{
			for (int z = 0; z < chunkZElements; z++)
			{
				index_3d = x + (y * chunkLineElements) + (z * chunkLineElementsP2);
				noise_3d_val = calculateNoiseCaves3d(x, y, z);
				noise_landscape_2d_val = calculateNoiseLandscape2d(x, y);
				noise_bedrock_2d_val = calculateNoiseBedrock2d(x, y);

				if (RandomStream.FRand() < 0.039 && z == 51 + noise_landscape_2d_val && noise_landscape_2d_val < 10) {
					treeCenters.Add(FIntVector(x, y, z));
				}
				else if (z == 50 + noise_landscape_2d_val) {
					chunkFields[index_3d] = 3;
				}
				else if (z >= 45 + noise_landscape_2d_val && z < 50 + noise_landscape_2d_val) {
					chunkFields[index_3d] = 4;
				}
				else if (z <= 2 + noise_bedrock_2d_val) {
					chunkFields[index_3d] = 6;
				}
				else if (z >= 35 + noise_landscape_2d_val && z < 45 + noise_landscape_2d_val) {
					chunkFields[index_3d] = 5;
				}
				else if (z < 35 + noise_landscape_2d_val && noise_3d_val < 45) {
					chunkFields[index_3d] = 5;
				}
				else {
					chunkFields[index_3d] = 0;

				}
			}
		}
	}

	for (FIntVector treeCenter : treeCenters)
	{
		int32 tree_heigh = RandomStream.RandRange(3, 6);
		int32 randomX = RandomStream.RandRange(0, 2);
		int32 randomY = RandomStream.RandRange(0, 2);
		int32 randomZ = RandomStream.RandRange(0, 2);

		for (int32 tree_x = -2; tree_x < 2; tree_x++)
		{
			for (int32 tree_y = -2; tree_y < 2; tree_y++)
			{
				for (int32 tree_z = -2; tree_z < 2; tree_z++)
				{
					if (checkRange(tree_x + treeCenter.X, chunkLineElements) && checkRange(tree_y + treeCenter.Y, chunkLineElements) && checkRange(tree_z + treeCenter.Z, chunkZElements))
					{
						float radius = FVector(tree_x * randomX, tree_y * randomY, tree_z * randomZ).Size();

						if (radius <= 2.8)
						{
							if (RandomStream.FRand() < 0.5 || radius <= 1.2)
							{
								int32 index_3ddd = (treeCenter.X + tree_x) + ((treeCenter.Y + tree_y) * chunkLineElements) + ((treeCenter.Z + tree_z + tree_heigh) * chunkLineElementsP2);
								chunkFields[index_3ddd] = 1;
							}
						}
					}
				}
			}
		}

		for (int32 th_z = 0; th_z < tree_heigh; th_z++) {
			int32 index_3dd = treeCenter.X + (treeCenter.Y * chunkLineElements) + ((treeCenter.Z + th_z) * chunkLineElementsP2);
			chunkFields[index_3dd] = 2;
		}
	}
}

bool FChunkBuilderCalculation::IsThreadActive()
{
	return bStopThread;
}

uint32 FChunkBuilderCalculation::Run()
{
	GenerateChunk();
	bStopThread = true;

	return 0;
}

void FChunkBuilderCalculation::Stop()
{
	bStopThread = true;
}

int32 FChunkBuilderCalculation::calculateNoiseLandscape2d(int x, int y)
{
	float _x = (chunkXIndex * chunkLineElements + x);
	float _y = (chunkYIndex * chunkLineElements + y);

	float noise1 = SimplexNoise::noise(_x * 0.01, _y * 0.01) * 4.0f;
	float noise2 = SimplexNoise::noise(_x * 0.01, _y * 0.01) * 8.0f;
	float noise3 = SimplexNoise::noise(_x * 0.004, _y * 0.004) * 16.0f;
	float noise4 = SimplexNoise::noise(_x * 0.05, _y * 0.05) * 4.0f;
	float noise4_clamp = FMath::Clamp(noise4, 0.0f, 5.0f);

	float noise_value = noise1 + noise2 + noise3 + noise4_clamp;

	int32 floor_noise_value = FMath::FloorToInt(noise_value);

	return floor_noise_value;
}

int32 FChunkBuilderCalculation::calculateNoiseBedrock2d(int x, int y)
{
	float _x = (chunkXIndex * chunkLineElements + x) * 0.5;
	float _y = (chunkYIndex * chunkLineElements + y) * 0.5;

	float noise = SimplexNoise::noise(_x, _y);
	float noise_value = noise * 2;

	int32 floor_noise_value = FMath::FloorToInt(noise_value);

	return floor_noise_value;
}


int32 FChunkBuilderCalculation::calculateNoiseCaves3d(int x, int y, int z)
{
	float _x = (chunkXIndex * chunkLineElements + x) * 0.12;
	float _y = (chunkYIndex * chunkLineElements + y) * 0.12;
	float _z = z * 0.12;

	float noise = SimplexNoise::noise(_x, _y, _z);
	float noise_value = noise * 100;

	int32 floor_noise_value = FMath::FloorToInt(noise_value);

	return floor_noise_value;
}


bool FChunkBuilderCalculation::checkRange(int32 value, int32 range)
{
	return (value >= 0 && value < range);
}
