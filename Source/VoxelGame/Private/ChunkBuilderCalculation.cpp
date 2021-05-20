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
	int32 locLakeLevel = 256;

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
				float noise_lake = calculateNoiseLake2d(x, y);

				// Landscape and caves generations
				if (z == 50 + noise_landscape_2d_val) {
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

				if (noise_lake > 0.44) {
					if (51 + noise_landscape_2d_val < locLakeLevel)
					{
						locLakeLevel = 51 + noise_landscape_2d_val;
					}
				}

				// Vegetation generations
				if (RandomStream.FRand() < 0.065 && z == 51 + noise_landscape_2d_val && noise_landscape_2d_val < 10) {
					if ((x > 2) && (x < chunkLineElements - 2) && (y > 2) && (y < chunkLineElements - 2))
						treeCenters.Add(FIntVector(x, y, z));
				}
				if (RandomStream.FRand() < 0.09 && z == 51 + noise_landscape_2d_val) {
					chunkFields[index_3d] = -1;
				}
				if (RandomStream.FRand() < 0.02 && z == 51 + noise_landscape_2d_val) {
					chunkFields[index_3d] = -2;
				}
				if (RandomStream.FRand() < 0.003 && z == 51 + noise_landscape_2d_val) {
					chunkFields[index_3d] = -3;
				}
				if (RandomStream.FRand() < 0.005 && z == 51 + noise_landscape_2d_val) {
					chunkFields[index_3d] = -4;
				}
				if (RandomStream.FRand() < 0.003 && z == 51 + noise_landscape_2d_val) {
					chunkFields[index_3d] = -5;
				}
				if (RandomStream.FRand() < 0.0025 && z == 51 + noise_landscape_2d_val) {
					chunkFields[index_3d] = -6;
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

	LakeBuilder(locLakeLevel);
}

void FChunkBuilderCalculation::LakeBuilder(int32 z_axis_min)
{
	for (int x = 0; x < chunkLineElements; x++)
	{
		for (int y = 0; y < chunkLineElements; y++)
		{
			for (int z = 0; z < chunkZElements; z++)
			{
				int32 noise_landscape_2d_val = calculateNoiseLandscape2d(x, y);
				float noise_lake = calculateNoiseLake2d(x, y);
				int32 index_3d = x + (y * chunkLineElements) + (z * chunkLineElementsP2);

				if (noise_lake > 0.44) {
					if (z >= 41 + (noise_landscape_2d_val / 2) && z < z_axis_min - 1)
					{
						chunkFields[index_3d] = 8;
					}
					else if (z >= z_axis_min - 1)
					{
						chunkFields[index_3d] = 0;
					}
				}
				if (noise_lake >= 0.36 && noise_lake <= 0.44) {
					if (z == 50 + noise_landscape_2d_val) {
						chunkFields[index_3d] = 7;
					}
					else if (z >= 45 + noise_landscape_2d_val && z < 50 + noise_landscape_2d_val) {
						chunkFields[index_3d] = 7;
					}
				}
			}
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
	float noise2 = SimplexNoise::noise(_x * 0.001, _y * 0.001) * 4.0f;
	float noise3 = SimplexNoise::noise(_x * 0.004, _y * 0.004) * 4.0f;
	float noise4 = SimplexNoise::noise(_x * 0.05, _y * 0.05) * 4.0f;
	float noise4_clamp = FMath::Clamp(noise4, 0.0f, 5.0f);

	float noise_value = noise1 + noise4_clamp;

	int32 floor_noise_value = FMath::FloorToInt(noise_value);

	return floor_noise_value;
}

float FChunkBuilderCalculation::calculateNoiseLake2d(int x, int y)
{
	float _x = (chunkXIndex * chunkLineElements + x);
	float _y = (chunkYIndex * chunkLineElements + y);

	const FVector2D CurrentLocation = FVector2D(_x / 25.f, _y / 25.f);
	float noise_lake = FMath::PerlinNoise2D(CurrentLocation);

	return noise_lake;
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

	//float noise = SimplexNoise::noise(_x, _y, _z);
	const FVector CurrentLocation = FVector(_x, _y, _z);
	float noise = FMath::PerlinNoise3D(CurrentLocation);
	float noise_value = noise * 100;

	int32 floor_noise_value = FMath::FloorToInt(noise_value);

	return floor_noise_value;
}

bool FChunkBuilderCalculation::checkRange(int32 value, int32 range)
{
	return (value >= 0 && value < range);
}
