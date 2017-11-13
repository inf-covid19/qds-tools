#include "stdafx.h"

#include "types.h"
#include "date_util.h"
#include "string_util.h"

inline int linearScale(float min, float max, float a, float b, float x) {
  return std::floor((((b - a) * (x - min)) / (max - min)) + a);
  //return value >= b ? (b - 1) : (value < a ? a : value);
}

int main(int argc, char *argv[]) {

  bool header = false;
  uint32_t bins = 4;
  std::string sep = ",";
  std::string input = "input.csv";
  std::string output = "output";

  namespace po = boost::program_options;

  // Declare the supported options.
  po::options_description desc("\nCommand Line Arguments");
  desc.add_options()("help,h", "produce help message");

  desc.add_options()("in,i", po::value<std::string>(&input)->default_value(input),
                     "input file");

  desc.add_options()("sep,s", po::value<std::string>(&sep)->default_value(sep),
                     "csv separator");

  desc.add_options()("output,o", po::value<std::string>(&output)->default_value(output),
                     "output file");

  desc.add_options()("bins,b", po::value<uint32_t >(&bins)->default_value(bins),
                     "bins per dimension");

  desc.add_options()("header", "csv header");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    exit(0);
  }

  if (vm.count("header")) {
    header = true;
  }

  // input
  std::cout << "reading input file" << std::endl;

  std::vector<std::vector<float>> data;
  std::vector<float> data_line;

  std::string line;
  std::ifstream infile(input);

  std::cout << input << std::endl;

  // skip csv header
  if (header) {
    std::getline(infile, line);
  }

  while (!infile.eof()) {

    std::getline(infile, line);

    try {
      if (line.empty()) continue;

      auto record = string_util::split(line, sep);

      data_line.clear();

      for (auto &value : record) {
        data_line.emplace_back(std::stof(value));
      }

      data.emplace_back(data_line);

    } catch (std::invalid_argument) {
      std::cerr << "error: line [" << line << "]" << std::endl;
    }

  }
  infile.close();

  std::cout << "checking sanity" << std::endl;

  // check sanity
  auto columns_per_line = data.front().size();
  for (auto &v: data) {
    if (v.size() != columns_per_line) {
      std::cerr << "error" << std::endl;
      exit(-1);
    }
  }



  struct min_max_t {
    float min{std::numeric_limits<float>::max()};
    float max{std::numeric_limits<float>::min()};

    inline uint8_t get_bin(float &value) {
      float a = 0, b = 3;
      return (uint8_t)std::floor((((b - a) * (value- min)) / (max - min)) + a);
    }
  };

  std::cout << "calculating min/max" << std::endl;

  // min/max
  std::vector<min_max_t> min_max(columns_per_line, min_max_t());

  for (auto d = 0; d < columns_per_line; ++d) {
    for (auto &v: data) {
      min_max[d].min = std::min(min_max[d].min, v[d]);
      min_max[d].max = std::max(min_max[d].max, v[d]);
    }
  }

  std::cout << "outputing binary" << std::endl;

  // binary output
  std::ofstream output_nds(output + ".nds", std::ios::out | std::ios::binary);

  BinaryHeader bin_header;

  bin_header.bytes = (columns_per_line * sizeof(uint8_t));
  bin_header.records = data.size();

  output_nds.write((char*)&bin_header, sizeof(BinaryHeader));

  for (auto d = 0; d < columns_per_line; ++d) {
    for (auto &v: data) {

      uint8_t formated_value = min_max[d].get_bin(v[d]);

      output_nds.write((char *) &formated_value, sizeof(uint8_t));
    }
  }

  output_nds.close();

  std::cout << "outputing xml" << std::endl;

  // xml output

  boost::property_tree::ptree root;

  boost::property_tree::ptree config_node;

  config_node.put("name", output);
  config_node.put("bytes", bin_header.bytes);
  config_node.put("file", output + ".nds");

  boost::property_tree::ptree schema_node;

  uint32_t index = 0;
  uint32_t offset = 0;

  for (auto d = 0; d < columns_per_line; ++d) {
    boost::property_tree::ptree node;

    node.put("index", index++);
    node.put("bin", bins);
    node.put("offset", offset);

    offset += sizeof(uint8_t);

    schema_node.add_child("categorical", node);
  }

  config_node.add_child("schema", schema_node);

  root.add_child("config", config_node);

  std::ofstream output_xml(output + ".nds.xml", std::ios::out);

  boost::property_tree::xml_writer_settings<boost::property_tree::ptree::key_type> settings('\t', 1);
  boost::property_tree::write_xml(output_xml, root, settings);

  output_xml.close();

  return 0;
}
