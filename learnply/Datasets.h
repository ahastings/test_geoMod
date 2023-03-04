#pragma once
#include <fstream>
#include <filesystem>
#include <iostream>
#include "gl/glew.h"
#include "Structs.h"

using namespace std;
namespace fs = std::filesystem;
namespace ms = myStructs;

void AddLayerToDataset(unsigned char* pixels, int layerIndex, float surface);

void LoadJpeg(const char* imagePath, int layerIndex, float surface);

int GetNumberOfFiles(fs::path path);

void InitDataset(fs::path dataPath, int depth);

bool OpenDataset(string datasetName, float surface);

ms::Dataset GetCurrentDataset();