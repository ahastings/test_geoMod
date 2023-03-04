#include "MarchingCubes.h"
#include "MarchingTables.h"

#include <iostream>

int _pointsPerAxis[3];
ms::Vector4* _values;

void GenerateTestData(float radius, ms::Vector3 center, int noiseLayers) {
	_values = SphereDataset(radius, _pointsPerAxis, center, noiseLayers);
}

float noise(float n) {
	float value = sin(n) * 100000;
	float iv = (int)value;
	return value - iv;
}

ms::Vector4* SphereDataset(int radius, int pointsPerAxis[3], ms::Vector3 center, int noiseLayers) {
	pointsPerAxis[0] = (radius + 1) * 2;
	pointsPerAxis[1] = (radius + 1) * 2;
	pointsPerAxis[2] = (radius + 1) * 2;
	ms::Vector4* values = new ms::Vector4[pointsPerAxis[0] * pointsPerAxis[1] * pointsPerAxis[2]];

	float offset =  (pointsPerAxis[0] / 2.0) - 0.5;
	center.x += (pointsPerAxis[0] / 2.0) - 0.5;
	center.y += (pointsPerAxis[1] / 2.0) - 0.5;
	center.z += (pointsPerAxis[2] / 2.0) - 0.5;

	std::cout << "Generating a " << pointsPerAxis[0] << " x " << pointsPerAxis[1] << " x " << pointsPerAxis[2] << " cube scalar field. Generating " << (pointsPerAxis[0] * pointsPerAxis[1] * pointsPerAxis[2]) << " points." << std::endl;
	for (int z = 0; z < pointsPerAxis[2]; z++)
	{
		for (int y = 0; y < pointsPerAxis[1]; y++)
		{
			for (int x = 0; x < pointsPerAxis[0]; x++)
			{
				float value = center.Distance(x, y, z) - radius;
				float frequency = 0.01;
				float amplitude = 1;
				for (int i = 0; i < noiseLayers; i++)
				{
					value += noise(x * frequency) * amplitude;
					value += noise(y * frequency) * amplitude;
					value += noise(z * frequency) * amplitude;
					amplitude *= 0.5;
					frequency *= 2;
				}

				values[indexFromCoord(x, y, z, pointsPerAxis)] = ms::Vector4(offset - x, offset - y, offset - z, value);
			}
		}
	}
	std::cout << "Done." << std::endl;

	return values;
}

ms::Mesh TestMarchingCubesSphere(float surface) {
	ms::Mesh mesh = March(_values, _pointsPerAxis, surface);

	return mesh;
}

ms::Mesh MarchDataset(ms::Dataset dataset, float surface) {
	int arr[3] = { dataset.Width(), dataset.Height(), dataset.Depth() };
	return March(dataset.Values(), arr, surface);
}

int indexFromCoord(int x, int y, int z, int pointsPerAxis[3]) {
	return x + y * pointsPerAxis[0] + z * pointsPerAxis[0] * pointsPerAxis[1];
}

ms::Vector3 interpolateVertices(ms::Vector4 v1, ms::Vector4 v2, float surface) {
	float t = surface;
	//if no part of the lerp equation will be 0, calculate t, else just use surface
	if ((v1.w - v2.w) != 0 && (surface - v1.w) != 0) {
		t = (surface - v1.w) / (v2.w - v1.w);
		t = t < 0 ? 0 : (t > 1 ? 1 : t);//clamp t to 0-1
		//t = (int)(t * 100 + 0.5);//limit t to two decimal places
		//t = (float)t / 100;
		//std::cout << "v1: " << v1.toString() << ", v2: " << v2.toString() << std::endl;
		//std::cout << "t: " << t << std::endl;
	}
	//std::cout << "t: " << t << std::endl;
	ms::Vector3 v = ms::Vector3(v1.x + t * (v2.x - v1.x),
		v1.y + t * (v2.y - v1.y),
		v1.z + t * (v2.z - v1.z));
	//std::cout << v.toString() << std::endl;
	return v;
}

ms::Mesh March(ms::Vector4* values, int pointsPerAxis[3], float surface) {
	ms::Mesh mesh;
	for (int z = 0; z < pointsPerAxis[2]; z++)
	{
		for (int y = 0; y < pointsPerAxis[1]; y++)
		{
			for (int x = 0; x < pointsPerAxis[0]; x++)
			{
				if (x >= pointsPerAxis[0] - 1 || y >= pointsPerAxis[1] - 1 || z >= pointsPerAxis[2] - 1) continue;//continue if in a position without cube
				//get 8 corners of current cube
				ms::Vector4 cubeCorners[8] = {
					values[indexFromCoord(x	   , y    , z    , pointsPerAxis)],
					values[indexFromCoord(x + 1, y    , z    , pointsPerAxis)],
					values[indexFromCoord(x + 1, y    , z + 1, pointsPerAxis)],
					values[indexFromCoord(x    , y    , z + 1, pointsPerAxis)],
					values[indexFromCoord(x    , y + 1, z    , pointsPerAxis)],
					values[indexFromCoord(x + 1, y + 1, z    , pointsPerAxis)],
					values[indexFromCoord(x + 1, y + 1, z + 1, pointsPerAxis)],
					values[indexFromCoord(x    , y + 1, z + 1, pointsPerAxis)]
				};

				/*std::cout << "(" << x << ", " << y << ", " << z << "): ";
				for (int i = 0; i < 8; i++) {
					std::cout << cubeCorners[i].toString() << ", ";
				}
				std::cout << "\n" << std::endl;
				continue;*/

				//calculate index using bit shifting
				int cubeIndex = 0;
				if (cubeCorners[0].w < surface) cubeIndex |= 1;// this is equal to: cubeIndex = cubeIndex | 1;
				if (cubeCorners[1].w < surface) cubeIndex |= 2;
				if (cubeCorners[2].w < surface) cubeIndex |= 4;
				if (cubeCorners[3].w < surface) cubeIndex |= 8;
				if (cubeCorners[4].w < surface) cubeIndex |= 16;
				if (cubeCorners[5].w < surface) cubeIndex |= 32;
				if (cubeCorners[6].w < surface) cubeIndex |= 64;
				if (cubeCorners[7].w < surface) cubeIndex |= 128;

				//use the index and lookup table to construct the required triangles
				for (int i = 0; triangleTable[cubeIndex][i] != -1; i += 3)
				{
					//Get the indices of the three cube edges that join to form the triangle
					int a0 = edgeConnections[triangleTable[cubeIndex][i]][0];
					int b0 = edgeConnections[triangleTable[cubeIndex][i]][1];

					int a1 = edgeConnections[triangleTable[cubeIndex][i+1]][0];
					int b1 = edgeConnections[triangleTable[cubeIndex][i+1]][1];

					int a2 = edgeConnections[triangleTable[cubeIndex][i+2]][0];
					int b2 = edgeConnections[triangleTable[cubeIndex][i+2]][1];

					//construct the triangle
					ms::Triangle tri = ms::Triangle(
						interpolateVertices(cubeCorners[a0], cubeCorners[b0], surface),
						interpolateVertices(cubeCorners[a1], cubeCorners[b1], surface),
						interpolateVertices(cubeCorners[a2], cubeCorners[b2], surface)
					);

					//std::cout << "making triangle " << mesh.vertices.size() / 3 << ": " << tri.v0.toString() << ", " << tri.v1.toString() << ", " << tri.v2.toString() << std::endl;
					mesh.AddTriangle(tri);
					if (i >= 14) break;
				}
			}
		}
	}
	
	return mesh;
}