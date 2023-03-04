#include "Datasets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const string DATASET_PATH = "../datasets/";
ms::Dataset dataset;

ms::Dataset GetCurrentDataset() {
	return dataset;
}

void AddLayerToDataset(unsigned char* pixels, int layerIndex, float surface) {
	//generate texture
	unsigned int texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dataset.Width(), dataset.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	//free raw data
	stbi_image_free(pixels);

	//retrieve layer texture data and add it to dataset
	//pixels = new unsigned char[dataset.width * dataset.height * dataset.channels];//dont know if I need to do this yet
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	dataset.AddLayer(pixels, layerIndex, surface);

	//delete texture
	glDeleteTextures(1, &texture_id);
}

void LoadJpeg(const char* imagePath, int layerIndex, float surface) {
	int width, height, channels;
	unsigned char* data = stbi_load(imagePath, &width, &height, &channels, 0);

	if (data) AddLayerToDataset(data, layerIndex, surface);
	else std::cout << "Failed to load image: " << imagePath << std::endl;
}

int GetNumberOfFiles(fs::path path) {
	using fs::directory_iterator;
	using fp = bool (*)(const fs::path&);
	return std::count_if(directory_iterator(path), directory_iterator{}, (fp)fs::is_regular_file);
}

void InitDataset(fs::path dataPath, int depth) {//opens first image and creates the dataset holder
	for (const auto& entry : fs::directory_iterator(dataPath)) {
		if (fs::is_regular_file(entry.status())) {
			int width, height, channels;
			unsigned char* data = stbi_load(entry.path().string().c_str(), &width, &height, &channels, 0);
			dataset.ClearDataset();
			dataset = ms::Dataset(dataPath.filename().string(), width, height, depth, channels);
			delete data;
			break;
		}
	}
}

bool OpenDataset(string datasetName, float surface) {
	fs::path dataPath(DATASET_PATH + datasetName);
	if (!fs::exists(dataPath) || !fs::is_directory(dataPath)) {
		std::cout << "Error opening directory: " << dataPath.relative_path() << std::endl;
		return false;
	}

	try {
		InitDataset(dataPath, GetNumberOfFiles(dataPath));

		//open all images and add them to the dataset
		int layerIndex = 0;
		for (const auto& entry : fs::directory_iterator(dataPath)) {
			if (fs::is_regular_file(entry.status())) {
				LoadJpeg(entry.path().string().c_str(), layerIndex++, surface);//pass in layer index and then increment it
			}
		}

		//examples of getting values, each layer is its own z value
		/*std::cout << "(0,0,0): " << dataset.GetValue(0, 0, 0) << std::endl;
		std::cout << "(66,111,0): " << dataset.GetValue(66, 111, 0) << std::endl;
		std::cout << "(25,129,0): " << dataset.GetValue(25, 129, 0) << std::endl;
		std::cout << "(0,0,1): " << dataset.GetValue(0, 0, 1) << std::endl;
		std::cout << "(66,111,1): " << dataset.GetValue(66, 111, 1) << std::endl;
		std::cout << "(25,129,1): " << dataset.GetValue(25, 129, 1) << std::endl;*/
	}
	catch (fs::filesystem_error err) {
		std::cout << "exception: " << err.what() << std::endl;
		return false;
	}
	return true;
}