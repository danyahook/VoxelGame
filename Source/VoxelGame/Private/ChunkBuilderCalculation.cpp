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

/*
	Инициализрующая функция
*/
bool FChunkBuilderCalculation::Init()
{
	bStopThread = false;

	return true;
}
/*
	Генерация данных чанка
*/
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
				noise_landscape_2d_val = calculateNoiseLandscape2d(x, y, chunkXIndex, chunkYIndex);
				noise_bedrock_2d_val = calculateNoiseBedrock2d(x, y);
				float noise_lake = calculateNoiseLake2d(x, y, chunkXIndex, chunkYIndex);

				// Генерация ландшафта и пещер
				if (z == 50 + noise_landscape_2d_val) {
					chunkFields[index_3d] = 11;
				}
				else if (z >= 45 + noise_landscape_2d_val && z < 50 + noise_landscape_2d_val) {
					chunkFields[index_3d] = 12;
				}
				else if (z <= 2 + noise_bedrock_2d_val) {
					chunkFields[index_3d] = 14;
				}
				else if (z >= 35 + noise_landscape_2d_val && z < 45 + noise_landscape_2d_val) {
					if (noise_3d_val > 21 && noise_3d_val < 24)
						chunkFields[index_3d] = 18;
					else if (noise_3d_val < -43 && noise_3d_val > -63)
						chunkFields[index_3d] = 17;
					else
						chunkFields[index_3d] = 13;
				}
				else if (z < 35 + noise_landscape_2d_val && noise_3d_val < 45) {
					if (noise_3d_val > 17 && noise_3d_val < 24)
						chunkFields[index_3d] = 17;
					else if (noise_3d_val < -47 && noise_3d_val > -63)
						chunkFields[index_3d] = 18;
					else
						chunkFields[index_3d] = 13;
				}
				else {
					chunkFields[index_3d] = 0;

				}

				// Генерация ростительности
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

				if (RandomStream.FRand() < 0.0005 && z == 51 + noise_landscape_2d_val) {
					chunkFields[index_3d] = -8;
				}
			}
		}
	}

	// Генерация деревьев
	for (FIntVector treeCenter : treeCenters)
	{
		float type_chance = RandomStream.FRand();

		int32 tree_heigh = RandomStream.RandRange(3, 6);
		int32 tree_type = RandomStream.RandRange(1, 4);
		int32 randomX = RandomStream.RandRange(0, 2);
		int32 randomY = RandomStream.RandRange(0, 2);
		int32 randomZ = RandomStream.RandRange(0, 2);

		if (type_chance > 0.95)
			tree_type = 4;
		else if (type_chance > 0.75)
			tree_type = 3;
		else if (type_chance > 0.65)
			tree_type = 2;
		else
			tree_type = 1;

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
								chunkFields[index_3ddd] = tree_type;
							}
						}
					}
				}
			}
		}

		for (int32 th_z = 0; th_z < tree_heigh; th_z++) {
			int32 index_3dd = treeCenter.X + (treeCenter.Y * chunkLineElements) + ((treeCenter.Z + th_z) * chunkLineElementsP2);
			chunkFields[index_3dd] = 10;
		}

		int32 part_index = treeCenter.X + (treeCenter.Y * chunkLineElements) + ((treeCenter.Z + tree_heigh + 3) * chunkLineElementsP2);
		chunkFields[part_index] = -7;
	}

	LakeBuilder();
}

/*
	Функция генерации озер
*/
void FChunkBuilderCalculation::LakeBuilder()
{
	int32 z_axis_min = findLakeLavel(chunkXIndex, chunkYIndex);

	for (int x = 0; x < chunkLineElements; x++)
	{
		for (int y = 0; y < chunkLineElements; y++)
		{
			for (int z = 0; z < chunkZElements; z++)
			{
				int32 noise_landscape_2d_val = calculateNoiseLandscape2d(x, y, chunkXIndex, chunkYIndex);
				float noise_lake = calculateNoiseLake2d(x, y, chunkXIndex, chunkYIndex);
				int32 index_3d = x + (y * chunkLineElements) + (z * chunkLineElementsP2);

				if (noise_lake > 0.51) {
					if (z >= 41 + (noise_landscape_2d_val / 2) && z < z_axis_min - 1)
					{
						chunkFields[index_3d] = 16;
					}
					else if (z >= z_axis_min - 1)
					{
						chunkFields[index_3d] = 0;
					}
				}
				if (noise_lake >= 0.46 && noise_lake <= 0.51) {
					if (z == 50 + noise_landscape_2d_val) {
						chunkFields[index_3d] = 15;
					}
					else if (z >= 45 + noise_landscape_2d_val && z < 50 + noise_landscape_2d_val) {
						chunkFields[index_3d] = 15;
					}
				}
			}
		}
	}
}
/*
	Проверка активен ли поток
*/
bool FChunkBuilderCalculation::IsThreadActive()
{
	return bStopThread;
}

/*
	Запуск потока
*/
uint32 FChunkBuilderCalculation::Run()
{
	GenerateChunk();
	bStopThread = true;

	return 0;
}

/*
	Стоп потока
*/
void FChunkBuilderCalculation::Stop()
{
	bStopThread = true;
}

/*
	Расчет шума Перлина для генерации ландшафта
*/
int32 FChunkBuilderCalculation::calculateNoiseLandscape2d(int x, int y, int locChunkXIndex, int locChunkYIndex)
{
	float _x = (locChunkXIndex * chunkLineElements + x);
	float _y = (locChunkYIndex * chunkLineElements + y);

	float noise1 = SimplexNoise::noise(_x * 0.01, _y * 0.01) * 4.0f;
	float noise2 = SimplexNoise::noise(_x * 0.001, _y * 0.001) * 4.0f;
	float noise3 = SimplexNoise::noise(_x * 0.004, _y * 0.004) * 4.0f;
	float noise4 = SimplexNoise::noise(_x * 0.05, _y * 0.05) * 4.0f;
	float noise4_clamp = FMath::Clamp(noise4, 0.0f, 5.0f);

	float noise_value = noise1 + noise4_clamp;

	int32 floor_noise_value = FMath::FloorToInt(noise_value);

	return floor_noise_value;
}

/*
	Расчет шума Перлина для генерации озер
*/
float FChunkBuilderCalculation::calculateNoiseLake2d(int x, int y, int locChunkXIndex, int locChunkYIndex)
{
	float _x = (locChunkXIndex * chunkLineElements + x);
	float _y = (locChunkYIndex * chunkLineElements + y);

	const FVector2D CurrentLocation = FVector2D(_x / 25.f, _y / 25.f);
	float noise_lake = FMath::PerlinNoise2D(CurrentLocation);

	return noise_lake;
}

/*
	Расчет шума Перлина для генерации нижней части чанка
*/
int32 FChunkBuilderCalculation::calculateNoiseBedrock2d(int x, int y)
{
	float _x = (chunkXIndex * chunkLineElements + x) * 0.5;
	float _y = (chunkYIndex * chunkLineElements + y) * 0.5;

	float noise = SimplexNoise::noise(_x, _y);
	float noise_value = noise * 2;

	int32 floor_noise_value = FMath::FloorToInt(noise_value);

	return floor_noise_value;
}

/*
	Расчет шума Перлина для генерации пещер
*/
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

/*
	Проверка радиуса деревьев
*/
bool FChunkBuilderCalculation::checkRange(int32 value, int32 range)
{
	return (value >= 0 && value < range);
}

/*
	поиск минимального уровня воды
*/
int32 FChunkBuilderCalculation::findLakeLavel(int32 pos_x, int32 pos_y)
{
	int32 z_axis_min = 128;
	int32 loc_x = 0;
	int32 loc_y = 0;

	int32 noise_landscape_2d_val = 0;

	for (int i = -1; i < 2; i++)
	{
		loc_x = pos_x - i;
		for (int j = -1; j < 2; j++)
		{
			loc_y = pos_y - j;
			for (int x = 0; x < chunkLineElements; x++)
			{
				for (int y = 0; y < chunkLineElements; y++)
				{
					float noise_lake = calculateNoiseLake2d(x, y, loc_x, loc_y);
					if (noise_lake >= 0.51)
					{
						noise_landscape_2d_val = calculateNoiseLandscape2d(x, y, loc_x, loc_y);

						if (51 + noise_landscape_2d_val < z_axis_min)
							z_axis_min = 51 + noise_landscape_2d_val;
					}
				}
			}
		}
	}

	return z_axis_min;
}