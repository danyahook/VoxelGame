#include "VoxelActor.h"

const int32 bTriangles[] = { 2, 1, 0, 0, 3, 2 };
const FVector2D bUVs[] = { FVector2D(0.000000, 0.000000), FVector2D(0.00000, 1.00000), FVector2D(1.00000, 1.00000), FVector2D(1.00000, 0.000000) };
const FVector bNormals0[] = { FVector(0, 0, 1), FVector(0, 0, 1), FVector(0, 0, 1), FVector(0, 0, 1) };
const FVector bNormals1[] = { FVector(0, 0, -1), FVector(0, 0, -1), FVector(0, 0, -1), FVector(0, 0, -1) };
const FVector bNormals2[] = { FVector(0, 1, 0),FVector(0, 1, 0) ,FVector(0, 1, 0) ,FVector(0, 1, 0) };
const FVector bNormals3[] = { FVector(0, -1, 0),FVector(0, -1, 0), FVector(0, -1, 0), FVector(0, -1, 0) };
const FVector bNormals4[] = { FVector(1, 0, 0),FVector(1, 0, 0), FVector(1, 0, 0), FVector(1, 0, 0) };
const FVector bNormals5[] = { FVector(-1,0,0),FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0) };
const FVector bMask[] = { FVector(0.00000, 0.00000, 1.00000),FVector(0.00000, 0.00000, -1.00000) ,FVector(0.00000, 1.00000, 0.00000) ,FVector(0.00000, -1.00000, 0.00000), FVector(1.00000, 0.0000, 0.00000),FVector(-1.00000, 0.0000, 0.00000) };

AVoxelActor::AVoxelActor()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AVoxelActor::BeginPlay()
{
	Super::BeginPlay();
}


void AVoxelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVoxelActor::StartThread()
{
	ChunkTimerHandle.Invalidate();
	CalcThread = new FChunkBuilderCalculation(
		this,
		chunkLineElements,
		chunkZElements,
		chunkLineElementsP2,
		chunkTotalElements,
		chunkXIndex,
		chunkYIndex
	);

	CurrentRunningThread = FRunnableThread::Create(CalcThread, TEXT("Calculation Thread"));

	if (CurrentRunningThread == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Chunkbuilder failed"));
		return;
	}

	GetWorldTimerManager().SetTimer(ChunkTimerHandle, this, &AVoxelActor::CheckChunkBuilder, 0.01f, false);
}

void AVoxelActor::CheckChunkBuilder()
{
	ChunkTimerHandle.Invalidate();
	if (CalcThread->IsThreadActive())
	{
		chunkFields = CalcThread->chunkFields;

		if (CurrentRunningThread && CalcThread)
		{
			CurrentRunningThread->Suspend(true);
			CalcThread->bStopThread = true;
			CurrentRunningThread->Suspend(false);
			CurrentRunningThread->Kill(false);
			CurrentRunningThread->WaitForCompletion();
			delete CalcThread;
			CurrentRunningThread = nullptr;
		}

		UpdateMesh();
	}
	else {
		GetWorldTimerManager().SetTimer(ChunkTimerHandle, this, &AVoxelActor::CheckChunkBuilder, 0.01f, false);
	}
}

void AVoxelActor::OnConstruction(const FTransform& Transform)
{
	chunkZElements = 80;
	chunkTotalElements = chunkLineElements * chunkLineElements * chunkZElements;
	chunkLineElementsP2 = chunkLineElements * chunkLineElements;
	voxelSizeHalf = voxelSize / 2;

	FString string = "Voxel_" + FString::FromInt(chunkXIndex) + "_" + FString::FromInt(chunkYIndex);
	FName name = FName(*string);
	proceduralComponent = NewObject<UProceduralMeshComponent>(this, name);
	proceduralComponent->bUseAsyncCooking = true;
	proceduralComponent->RegisterComponent();

	RootComponent = proceduralComponent;
	RootComponent->SetWorldTransform(Transform);

	Super::OnConstruction(Transform);

	StartThread();
}

void AVoxelActor::UpdateMesh()
{
	TArray<FMeshSection> meshSections;
	meshSections.SetNum(Materials.Num());

	int el_num = 0;

	for (int32 x = 0; x < chunkLineElements; x++)
	{
		for (int32 y = 0; y < chunkLineElements; y++)
		{
			for (int32 z = 0; z < chunkZElements; z++)
			{
				int32 index = x + (chunkLineElements * y) + (chunkLineElementsP2 * z);
				int32 meshIndex = chunkFields[index];

				if (meshIndex > Materials.Num()) {
					return;
				}

				if (meshIndex > 0) {
					meshIndex--;

					TArray<FVector>& Vertics = meshSections[meshIndex].Vertics;
					TArray<int32>& Triangles = meshSections[meshIndex].Triangles;
					TArray<FVector>& Normals = meshSections[meshIndex].Normals;
					TArray<FVector2D>& UVs = meshSections[meshIndex].UVs;
					TArray<FProcMeshTangent>& Tangents = meshSections[meshIndex].Tangents;
					TArray<FColor>& VertexColor = meshSections[meshIndex].VertexColor;
					int32 elementId = meshSections[meshIndex].elementId;

					int triangle_num = 0;
					for (int i = 0; i < 6; i++)
					{
						int newIndex = index + bMask[i].X + (bMask[i].Y * chunkLineElements) + (bMask[i].Z * chunkLineElementsP2);
						bool flag = false;


						if (meshIndex >= 20)
							flag = true;

						else if ((x + bMask[i].X < chunkLineElements) && (x + bMask[i].X >= 0) && (y + bMask[i].Y < chunkLineElements) && (y + bMask[i].Y >= 0) && (z + bMask[i].Z >= 0) && (z + bMask[i].Z < chunkZElements))
						{
							if (newIndex < chunkFields.Num() && newIndex >= 0)
								if (chunkFields[newIndex] < 2) flag = true;
						}

						else flag = true;

						if (flag)
						{
							Triangles.Add(bTriangles[0] + triangle_num + elementId);
							Triangles.Add(bTriangles[1] + triangle_num + elementId);
							Triangles.Add(bTriangles[2] + triangle_num + elementId);
							Triangles.Add(bTriangles[3] + triangle_num + elementId);
							Triangles.Add(bTriangles[4] + triangle_num + elementId);
							Triangles.Add(bTriangles[5] + triangle_num + elementId);
							triangle_num += 4;

							switch (i)
							{
							case 0: {
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));

								Normals.Append(bNormals0, ARRAY_COUNT(bNormals0));
								break;
							}
							case 1: {
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));

								Normals.Append(bNormals1, ARRAY_COUNT(bNormals1));
								break;
							}
							case 2: {
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));

								Normals.Append(bNormals2, ARRAY_COUNT(bNormals2));
								break;
							}
							case 3: {
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));

								Normals.Append(bNormals3, ARRAY_COUNT(bNormals3));
								break;
							}
							case 4: {
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));

								Normals.Append(bNormals5, ARRAY_COUNT(bNormals4));
								break;
							}
							case 5: {
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), -voxelSizeHalf + (z * voxelSize)));
								Vertics.Add(FVector(-voxelSizeHalf + (x * voxelSize), -voxelSizeHalf + (y * voxelSize), voxelSizeHalf + (z * voxelSize)));

								Normals.Append(bNormals4, ARRAY_COUNT(bNormals5));
								break;
							}
							}

							UVs.Append(bUVs, ARRAY_COUNT(bUVs));

							FColor color = FColor(255, 255, 255, i);
							VertexColor.Add(color); VertexColor.Add(color); VertexColor.Add(color); VertexColor.Add(color);
						}
					}

					el_num += triangle_num;
					meshSections[meshIndex].elementId += triangle_num;
				}

			}
		}
	}

	proceduralComponent->ClearAllMeshSections();

	for (int i = 0; i < meshSections.Num(); i++) {
		if (meshSections.Num() > 0) {
			proceduralComponent->CreateMeshSection(i, meshSections[i].Vertics, meshSections[i].Triangles, meshSections[i].Normals, meshSections[i].UVs, meshSections[i].VertexColor, meshSections[i].Tangents, true);
		}
	}

	int s = 0;
	while (s < Materials.Num())
	{
		proceduralComponent->SetMaterial(s, Materials[s]);
		s++;
	}

}
