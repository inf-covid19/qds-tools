#include "stdafx.h"

#include "DataDescriptor.h"

Schema loadConfig(std::string fileName) {
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(fileName, pt);

	Schema e;

	boost::optional<ulong> leaf = pt.get_optional<ulong>("config.leaf");
	if (leaf.is_initialized()) e.leaf = leaf.value();

	boost::optional<ulong> sql_threshold = pt.get_optional<ulong>("config.sql-threshold");
	if (sql_threshold.is_initialized()) e.sql_threshold = sql_threshold.value();

	boost::optional<float> fraction = pt.get_optional<float>("config.fraction");
	if (fraction.is_initialized()) e.fraction = fraction.value() / 100.f;

	e.name = pt.get<std::string>("config.name");
	e.path = pt.get<std::string>("config.path");
	e.loader = pt.get<std::string>("config.loader");

	for (auto &v : pt.get_child("config.schema")) {
		if (v.first == "spatial") {
			e.spatial.emplace_back(v.second.get<ulong>("key"));
		}
		else if (v.first == "categorical") {
			std::string key = v.second.get<std::string>("key");
			ulong size = v.second.get<ulong>("size");
			e.categorical.emplace_back(key, size);
		}
		else if (v.first == "temporal") {
			std::string key = v.second.get<std::string>("key");
			ulong bin = v.second.get<ulong>("bin");
			e.temporal.emplace_back(key, bin);
		}
	}

	return e;
}

void load(const std::string& key , const Schema& schema, const DataDescriptor& descriptor, std::unordered_map<std::string, std::vector<char>>& hcf_data) {
	auto token = descriptor.get(key);

	std::string file = schema.path + "hcf/" + std::get<0>(token);
	std::ifstream input(file, std::ios::in | std::ifstream::binary);

	hcf_data.emplace(key, descriptor.size() * sizeof(uint32_t));
	input.read(&hcf_data[key][0], descriptor.size() * sizeof(uint32_t));

	input.close();
}

int main(int argc, char *argv[]) {

	if (argc != 4) {
		std::cerr << "error: invalid argument" << std::endl;
		std::cout << sizeof(uint64_t) << std::endl;

		exit(-1);
	}

	// read hcf into memory
	/////////////////////////////////////////////////////////////////////

	Schema schema(loadConfig(argv[2]));
	DataDescriptor descriptor(schema);

	std::unordered_map<std::string, std::vector<char>> hcf_data;

	for (const auto& key : schema.spatial) {
		load(std::to_string(key), schema, descriptor, hcf_data);
	}
	for (const auto& key : schema.categorical) {
		load(key.first, schema, descriptor, hcf_data);
	}
	for (const auto& key : schema.temporal) {
		load(key.first, schema, descriptor, hcf_data);
	}

	// write nds to disk
	/////////////////////////////////////////////////////////////////////

	std::ofstream output(argv[3], std::ios::out | std::ios::binary);

	BinaryHeader header;

	header.bytes = schema.spatial.size() * sizeof(float) + schema.categorical.size() * sizeof(uint8_t) + schema.temporal.size() * sizeof(uint64_t);
	header.records = descriptor.size();

	output.write((char*)&header, sizeof(BinaryHeader));

	

    return 0;
}

