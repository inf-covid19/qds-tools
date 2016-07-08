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

   for (auto& v : pt.get_child("config.schema")) {
      if (v.first == "spatial") {
         e.spatial.emplace_back(v.second.get<ulong>("key"));
      } else if (v.first == "categorical") {
         std::string key = v.second.get<std::string>("key");
         ulong size = v.second.get<ulong>("size");
         e.categorical.emplace_back(key, size);
      } else if (v.first == "temporal") {
         std::string key = v.second.get<std::string>("key");
         ulong bin = v.second.get<ulong>("bin");
         e.temporal.emplace_back(key, bin);
      }
   }

   return e;
}

void load(const std::string& key, const Schema& schema, const DataDescriptor& descriptor, std::unordered_map<std::string, std::vector<char>>& hcf_data) {
   auto token = descriptor.get(key);

   std::string file = schema.path + "hcf/" + std::get<0>(token);
   std::ifstream input(file, std::ios::in | std::ifstream::binary);

   hcf_data.emplace(key, descriptor.size() * sizeof(uint32_t));
   input.read(&hcf_data[key][0], descriptor.size() * sizeof(uint32_t));

   input.close();
}

int main(int argc, char* argv[]) {

   std::string xmlfile, outputpath;

   if (argc < 2) {
      std::cerr << "error: invalid argument" << std::endl;
      exit(-1);
   } else if (argc == 2) {
      xmlfile = std::string(argv[1]);
      outputpath = ".\\";
   } else {
      xmlfile = std::string(argv[1]);
      outputpath = std::string(argv[2]);
   }

   Schema schema(loadConfig(xmlfile));
   DataDescriptor descriptor(schema);

   std::unordered_map<std::string, std::vector<char>> hcf_data;

   // nds file
   /////////////////////////////////////////////////////////////

   std::ofstream output_nds(std::string(outputpath) + "/" + schema.name + ".nds", std::ios::out | std::ios::binary);

   BinaryHeader header;

   header.bytes = (schema.spatial.size() * sizeof(float) * 2) + (schema.categorical.size() * sizeof(uint8_t)) + (schema.temporal.size() * sizeof(uint32_t));
   header.records = descriptor.size();

   output_nds.write((char*)&header, sizeof(BinaryHeader));

   for (const auto& key : schema.spatial) {
      load("lat" + std::to_string(key), schema, descriptor, hcf_data);
      load("lon" + std::to_string(key), schema, descriptor, hcf_data);

      struct Coordinates { float lat, lon; } coord;

      for (auto i = 0; i < descriptor.size(); ++i) {
         std::memcpy(&coord.lat, &hcf_data["lat" + std::to_string(key)][i * sizeof(float)], sizeof(float));
         std::memcpy(&coord.lon, &hcf_data["lon" + std::to_string(key)][i * sizeof(float)], sizeof(float));

         output_nds.write((char*)&coord, sizeof(Coordinates));
      }

      hcf_data.clear();
   }

   for (const auto& key : schema.categorical) {
      load(key.first, schema, descriptor, hcf_data);

      int value;
      uint8_t formated_value;

      for (auto i = 0; i < descriptor.size(); ++i) {
         std::memcpy(&value, &hcf_data[key.first][i * sizeof(int)], sizeof(int));

         formated_value = static_cast<uint8_t>(value);

         output_nds.write((char*)&formated_value, sizeof(uint8_t));
      }

      hcf_data.clear();
   }

   for (const auto& key : schema.temporal) {
      load(key.first, schema, descriptor, hcf_data);

      int value;
      uint32_t formated_value;

      for (auto i = 0; i < descriptor.size(); ++i) {
         std::memcpy(&value, &hcf_data[key.first][i * sizeof(int)], sizeof(int));

         formated_value = static_cast<uint32_t>(value);

         output_nds.write((char*)&formated_value, sizeof(uint32_t));
      }

      hcf_data.clear();
   }

   output_nds.close();

   // xml file
   /////////////////////////////////////////////////////////////

   boost::property_tree::ptree root;

   boost::property_tree::ptree config_node;

   config_node.put("name", schema.name);
   config_node.put("bytes", header.bytes);
   config_node.put("file", schema.name + ".nds");

   boost::property_tree::ptree schema_node;

   uint32_t offset = 0;

   for (const auto& key : schema.spatial) {
      boost::property_tree::ptree node;

      node.put("key", key);
      node.put("bin", schema.leaf);
      node.put("offset", offset);

      offset += sizeof(float) * 2;

      schema_node.add_child("spatial", node);
   }

   for (const auto& key : schema.categorical) {
      boost::property_tree::ptree node;

      node.put("key", key.first);
      node.put("bin", key.second);
      node.put("offset", offset);

      offset += sizeof(uint8_t);

      schema_node.add_child("categorical", node);
   }

   for (const auto& key : schema.temporal) {

      boost::property_tree::ptree node;

      node.put("key", key.first);
      node.put("bin", key.second);
      node.put("offset", offset);

      offset += sizeof(uint32_t);

      schema_node.add_child("temporal", node);
   }

   config_node.add_child("schema", schema_node);

   root.add_child("config", config_node);

   std::ofstream output_xml(std::string(outputpath) + "/" + schema.name + ".nds.xml", std::ios::out);

   boost::property_tree::xml_writer_settings<boost::property_tree::ptree::key_type> settings('\t', 1);
   boost::property_tree::write_xml(output_xml, root, settings);

   output_xml.close();

   return 0;
}

