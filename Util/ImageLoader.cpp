#pragma warning(disable: 26451)
#pragma warning(disable: 4005)

#define THREAD_COUNT 6

#ifdef WINDOWS
#include <Shlwapi.h>
#endif

#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dctk.h>

#include <thread>
#include <vector>
#include <algorithm>

#include "ImageLoader.hpp"

using namespace std;
using namespace glm;

string GetExt(const string& path) {
	size_t k = path.rfind('.');
	if (k == string::npos) return "";
	return path.substr((int)k + 1);
}
string GetName(const string& path) {
	char const* str = path.c_str();

	int f = 0;
	int l = 0;
	for (int i = 0; i < path.length(); i++) {
		if (str[i] == '\\' || str[i] == '/')
			f = i + 1;
		else if (str[i] == '.')
			l = i;
	}

	return path.substr(f, l - f);
}
string GetFullPath(const string& str) {
#ifdef WINDOWS
	char buf[MAX_PATH];
	if (GetFullPathName(str.c_str(), 256, buf, nullptr) == 0) {
		printf("Failed to get full file path of %s (%d)", str.c_str(), GetLastError());
		return str;
	}
	return string(buf);
#endif
}

struct Slice {
	DicomImage* image;
	double location;
};

void ReadDicomSlice(DicomImage*& img, double& x, const string& file, double& spacingX, double& spacingY, double& thickness) {
	OFCondition cnd;

	DcmFileFormat fileFormat;
	assert((cnd = fileFormat.loadFile(file.c_str())).good());
	DcmDataset * dataset = fileFormat.getDataset();

	double sx;
	double sy;
	double th;
	cnd = dataset->findAndGetFloat64(DCM_PixelSpacing, sx, 0);
	cnd = dataset->findAndGetFloat64(DCM_PixelSpacing, sy, 1);
	cnd = dataset->findAndGetFloat64(DCM_SliceThickness, th, 0);

	spacingX = std::max(sx, spacingX);
	spacingY = std::max(sy, spacingY);
	thickness = std::max(th, thickness);

	cnd = dataset->findAndGetFloat64(DCM_SliceLocation, x, 0);

	img = new DicomImage(file.c_str());
	assert(img != NULL);
	assert(img->getStatus() == EIS_Normal);
}
void ReadDicomImage(DicomImage* img, uint16_t* slice, int w, int h) {
	img->setMinMaxWindow();
	uint16_t* pixelData = (uint16_t*)img->getOutputData(16);
	int j = 0;
	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++) {
			j = 2 * (x + y * w);
			slice[j] = pixelData[x + y * w];
		}
}
void ReadDicomImages(uint16_t* data, vector<Slice>& images, int j, int k, int w, int h) {
	for (int i = j; i < k; i++) {
		if (i >= (int)images.size()) break;
		ReadDicomImage(images[i].image, data + 2 * i * w * h, w, h);
	}
}

shared_ptr<Texture> LoadDicomImage(const string& path, vec3& size) {
	// Get information
	DicomImage* image = nullptr;
	double x = 0.0;
	double spacingX = 0.0;
	double spacingY = 0.0;
	double thickness = 0.0;
	ReadDicomSlice(image, x, path, spacingX, spacingY, thickness);
	
	unsigned int w = image->getWidth();
	unsigned int h = image->getHeight();
	unsigned int d = 1;

	// volume size in meters
	size.x = .001f * (float)spacingX * w;
	size.y = .001f * (float)spacingY * h;
	size.z = .001f * (float)thickness;

	uint16_t * data = new uint16_t[w * h * d * 2];
	memset(data, 0xFF, w * h * d * sizeof(uint16_t) * 2);
	ReadDicomImage(image, data, w, h);

	auto tex = shared_ptr<Texture>(new Texture(w, h, d, GL_RG16, GL_RG, GL_UNSIGNED_SHORT, GL_LINEAR, data));
	delete[] data;

	return tex;
}

shared_ptr<Texture> LoadDicomVolume(const vector<string>& files, vec3& size) {
	vector<Slice> images;

	// Get information
	double spacingX = 0.0;
	double spacingY = 0.0;
	double thickness = 0.0;

	for (unsigned int i = 0; i < (int)files.size(); i++) {
		DicomImage* img;
		double x;
		ReadDicomSlice(img, x, files[i], spacingX, spacingY, thickness);
		images.push_back({ img, x });
	}

	std::sort(images.begin(), images.end(), [](const Slice& a, const Slice& b) {
		return a.location < b.location;
	});

	unsigned int w = images[0].image->getWidth();
	unsigned int h = images[0].image->getHeight();
	unsigned int d = (unsigned int)images.size();

	// volume size in meters
	size.x = .001f * (float)spacingX * w;
	size.y = .001f * (float)spacingY * h;
	size.z = .001f * (float)thickness * images.size();

	printf("%fm x %fm x %fm\n", size.x, size.y, size.z);

	uint16_t* data = new uint16_t[w * h * d * 2];
	memset(data, 0xFFFF, w * h * d * sizeof(uint16_t) * 2);

	if (THREAD_COUNT > 1) {
		printf("reading %d slices\n", d);
		vector<thread> threads;
		int s = ((int)images.size() + THREAD_COUNT - 1) / THREAD_COUNT;
		for (int i = 0; i < (int)images.size(); i += s)
			threads.push_back(thread(ReadDicomImages, data, images, i, i + s, w, h));
		for (int i = 0; i < (int)threads.size(); i++)
			threads[i].join();
	} else {
		ReadDicomImages(data, images, 0, (int)images.size(), w, h);
	}

	auto tex = shared_ptr<Texture>(new Texture(w, h, d, GL_RG16, GL_RG, GL_UNSIGNED_SHORT, GL_LINEAR, data));
	delete[] data;

	return tex;
}

shared_ptr<Texture> ImageLoader::LoadImage(const string& path, vec3& size) {
#ifdef WINDOWS
	if (!PathFileExists(path.c_str())) {
#endif
		printf("%s Does not exist!\n", path.c_str());
		return 0;
	}

	string ext = GetExt(path.c_str());
	if (ext == "dcm")
		return LoadDicomImage(path, size);
	else
		return 0;
}

void GetFiles(const string& path, vector<string>& files) {
#ifdef WINDOWS
	string d = path + "\\*";

	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFile(d.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		assert(false);
		return;
	}

	do {
		if (ffd.cFileName[0] == L'.') continue;

		string c = path + "\\" + ffd.cFileName;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// file is a directory
		}
		else {
			string ext = GetExt(c);
			if (ext == "dcm" || ext == "raw" || ext == "png")
				files.push_back(GetFullPath(c));
		}
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);
#endif
}
shared_ptr<Texture> ImageLoader::LoadVolume(const string& path, vec3& size) {
#ifdef WINDOWS
	if (!PathFileExists(path.c_str())) {
#endif
		printf("%s Does not exist!\n", path.c_str());
		return 0;
	}

	vector<string> files;
	GetFiles(path, files);

	if (files.size() == 0) return 0;

	string ext = GetExt(files[0]);
	if (ext == "dcm")
		return LoadDicomVolume(files, size);
	else
		return 0;
}

void ImageLoader::LoadMask(const string& path, const shared_ptr<Texture>& texture) {
#ifdef WINDOWS
	if (!PathFileExists(path.c_str())) {
#endif
		printf("%s Does not exist!\n", path.c_str());
		return;
	}

	vector<string> files;
	GetFiles(path, files);

	if (files.size() != texture->Depth()) {
		printf("Incorrect slice count! (%u != %u)\n", (unsigned int)files.size(), texture->Depth());
		return;
	}

	std::sort(files.data(), files.data() + files.size(), [](const string & a, const string & b) {
		return atoi(GetName(a).c_str()) > atoi(GetName(b).c_str());
	});

	for (unsigned int i = 0; i < files.size(); i++) {
		// TODO: load into new texture and blit over
	}
}