#include "MapChipField.h"
#include <cassert>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <cmath>

namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
    {"2", MapChipType::kBlockAbove},
};
}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal, MapChipType::kBlank);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	ResetMapChipData();
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());
	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();
	file.close();

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		std::string line;
		getline(mapChipCsv, line);
		std::istringstream line_stream(line);
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			getline(line_stream, word, ',');
			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { 
	return Vector3(kBlockWidth * static_cast<float>(xIndex), 0.0f, kBlockHeight * (static_cast<float>(kNumBlockVirtical) - 1.0f - static_cast<float>(yIndex))); 
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	// インデックスが範囲外ならすべて Blank（進めない場所）として扱う
	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVirtical) {
		return MapChipType::kBlank;
	}
	return mapChipData_.data[yIndex][xIndex];
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	IndexSet indexSet = {};
	
	// 座標からインデックスを計算（floorを使って負の数にも対応）
	indexSet.xIndex = static_cast<int32_t>(std::floor((position.x + kBlockWidth / 2.0f) / kBlockWidth));
	
	float zRel = position.z / kBlockHeight;
	indexSet.yIndex = static_cast<int32_t>(std::floor(static_cast<float>(kNumBlockVirtical) - 0.5f - zRel));

	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {
	// 符号付きインデックスを考慮した中心座標の計算
	Vector3 center = Vector3(
		kBlockWidth * static_cast<float>(static_cast<int32_t>(xIndex)), 
		0.0f, 
		kBlockHeight * (static_cast<float>(kNumBlockVirtical) - 1.0f - static_cast<float>(static_cast<int32_t>(yIndex)))
	);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.z - kBlockHeight / 2.0f;
	rect.top = center.z + kBlockHeight / 2.0f;
	return rect;
}
