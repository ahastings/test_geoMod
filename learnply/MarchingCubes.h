#pragma once
#include "Structs.h"

namespace ms = myStructs;

void GenerateTestData(float radius, ms::Vector3 center, int noiseLayers);

float noise(float n);

ms::Vector4* SphereDataset(int radius, int pointsPerAxis[3], ms::Vector3 center, int noiseLayers = 0);

ms::Mesh TestMarchingCubesSphere(float surface);

ms::Mesh MarchDataset(ms::Dataset dataset, float surface);

ms::Mesh March(ms::Vector4* values, int pointsPerAxis[3], float surface);

int indexFromCoord(int x, int y, int z, int pointsPerAxis[3]);

ms::Vector3 interpolateVertices(ms::Vector4 v1, ms::Vector4 v2, float surface);