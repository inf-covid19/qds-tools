#pragma once

// hcf related
/////////////////////////////////////////////////////////////////////

typedef unsigned long ulong;

struct Schema {
	std::string name, path, loader;

	float fraction{ 1.f };
	ulong leaf{ 32 };
	ulong sql_threshold{ 250 };

	std::vector<ulong> spatial;
	std::vector<std::pair<std::string, ulong>> categorical;
	std::vector<std::pair<std::string, ulong>> temporal;
};

enum DimensionType {
	Spatial,
	Categorical,
	Temporal
};

typedef std::tuple<std::string, DimensionType> token_t;
typedef std::tuple<std::string, std::string, DimensionType> tuple_t;

// nds related
/////////////////////////////////////////////////////////////////////

struct BinaryHeader {
	uint32_t bytes;
	uint32_t records;
};