#include "stdafx.h"

std::default_random_engine _random_engine;
std::uniform_real_distribution<double> _uniform_dist {0.0, 1.0};

inline double trunc(double d) {
  return std::trunc(d * 100000.0) / 100000.0;
}

inline int linearScale(float min, float max, float a, float b, float x) {
  return std::floor((((b - a) * (x - min)) / (max - min)) + a);
  //return value >= b ? (b - 1) : (value < a ? a : value);
}

float randUniform(int min, int max) {
  float delta = max - min;
  return (float)(min + delta * _uniform_dist(_random_engine));
}

float randNormal(float mean, float stdev) {
  float x = 0, y = 0, rds, c;
  do {
    x = (float)(_uniform_dist(_random_engine) * 2 - 1);
    y = (float)(_uniform_dist(_random_engine) * 2 - 1);
    rds = x * x + y * y;
  } while (rds == 0 || rds > 1);
  c = (float)sqrt(-2 * log(rds) / rds);
  return mean + x * c * stdev;
}

using splom_elt = uint8_t;

int main(int argc, char *argv[]) {

  struct opts {
    std::string file{"output"};
    uint32_t bins{10};
  } nds_opts;

  namespace po = boost::program_options;

  // declare the supported options.
  po::options_description desc("\nCommand Line Arguments");

  desc.add_options()(
      "b",
      po::value<uint32_t>(&nds_opts.bins)->default_value(nds_opts.bins),
      "bins per dimension"
  );

  desc.add_options()(
      "f",
      po::value<std::string>(&nds_opts.file)->default_value(nds_opts.file),
      "output file"
  );

  // nds file
  /////////////////////////////////////////////////////////////

  std::ofstream output_nds(nds_opts.file + ".nds", std::ios::out | std::ios::binary);

  BinaryHeader header;

  uint32_t n_elts = 1000000000;
  //uint32_t n_elts = 100;

  std::vector<splom_elt> data(n_elts);

  header.bytes = 5 * sizeof(uint8_t);
  header.records = n_elts;

  output_nds.write((char *) &header, sizeof(BinaryHeader));

  // generating data
  for (auto l = 0; l < n_elts; ++l) {
    float d0 = randNormal(10, 10);
    data[l] = linearScale(-40.f, 60.f, 0, nds_opts.bins, d0);
  }

  uint8_t formated_value;
  for (auto l = 0; l < n_elts; ++l) {
    formated_value = static_cast<uint8_t>(data[l]);
    output_nds.write((char*)&formated_value, sizeof(uint8_t));
  }

  for (auto l = 0; l < n_elts; ++l) {
    formated_value = static_cast<uint8_t>(linearScale(-40.f, 60.f, 0, nds_opts.bins, randNormal(10, 10)));
    output_nds.write((char*)&formated_value, sizeof(uint8_t));
  }

  for (auto l = 0; l < n_elts; ++l) {
    formated_value = static_cast<uint8_t>(linearScale(-70.f, 80.f, 0, nds_opts.bins, randNormal(data[l], 10)));
    output_nds.write((char*)&formated_value, sizeof(uint8_t));
  }

  for (auto l = 0; l < n_elts; ++l) {
    formated_value = static_cast<uint8_t>(linearScale(1.f, 7.f, 0, nds_opts.bins, (log(fabs(data[l]) + 1) + randUniform(3, 1))));
    output_nds.write((char*)&formated_value, sizeof(uint8_t));
  }

  for (auto l = 0; l < n_elts; ++l) {
    formated_value = static_cast<uint8_t>(linearScale(-40.f, 60.f, 0, nds_opts.bins, randNormal(10, 10)));
    output_nds.write((char*)&formated_value, sizeof(uint8_t));
  }

  output_nds.close();


  // xml file
  /////////////////////////////////////////////////////////////

  boost::property_tree::ptree root;

  boost::property_tree::ptree config_node;

  config_node.put("name", nds_opts.file);
  config_node.put("bytes", header.bytes);
  config_node.put("file", nds_opts.file + ".nds");

  boost::property_tree::ptree schema_node;

  uint32_t index = 0;
  uint32_t offset = 0;

  for (auto d = 0; d < 5; ++d) {
    boost::property_tree::ptree node;

    node.put("index", index++);
    node.put("bin", nds_opts.bins);
    node.put("offset", offset);

    offset += sizeof(uint8_t);

    schema_node.add_child("categorical", node);
  }

  config_node.add_child("schema", schema_node);

  root.add_child("config", config_node);

  std::ofstream output_xml(nds_opts.file + ".nds.xml", std::ios::out);

  boost::property_tree::xml_writer_settings<boost::property_tree::ptree::key_type> settings('\t', 1);
  boost::property_tree::write_xml(output_xml, root, settings);

  output_xml.close();

  return 0;
}

