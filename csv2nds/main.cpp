#include "stdafx.h"
#include "types.h"
#include "date_util.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

void generate_xml(std::shared_ptr<TSchema> &schema, uint32_t bytes) {
  // generate xml info

  pt::ptree root;
  pt::ptree config_node;

  config_node.put("name", schema->output);
  config_node.put("bytes", bytes);
  config_node.put("file", schema->output);

  pt::ptree schema_node;

  uint32_t index = 0;
  uint32_t offset = 0;

  for (auto &pair : schema->dimension_map) {
    switch (pair.second->type) {
      case dimesion_t::CATEGORICAL: {
        auto d = dimesion_t::get<TCategorical>(pair.second.get());

        pt::ptree node;

        node.put("index", index++);
        node.put("bin", d->get_n_bins());
        node.put("offset", offset);

        offset += sizeof(uint8_t);

        schema_node.add_child("categorical", node);
      }
        break;

      case dimesion_t::TEMPORAL: {
        auto d = dimesion_t::get<TTemporal>(pair.second.get());

        pt::ptree node;

        node.put("index", index++);
        node.put("bin", d->interval);
        node.put("offset", offset);

        offset += sizeof(uint32_t);

        schema_node.add_child("temporal", node);
      }
        break;

      case dimesion_t::SPATIAL: {
        auto d = dimesion_t::get<TSpatial>(pair.second.get());

        pt::ptree node;

        node.put("index", index++);
        node.put("bin", 1);
        node.put("offset", offset);

        offset += sizeof(coordinates_t);

        schema_node.add_child("spatial", node);
      }
        break;
    }
  }

  config_node.add_child("schema", schema_node);

  root.add_child("config", config_node);

  std::ofstream output_xml(schema->output + ".xml", std::ios::out);

  pt::xml_writer_settings<boost::property_tree::ptree::key_type> settings('\t', 1);
  pt::write_xml(output_xml, root, settings);

  output_xml.close();
}

std::default_random_engine _random_engine;
std::uniform_real_distribution<double> _uniform_dist{0.0, 1.0};

inline float randUniform(int min, int max) {
  float delta = max - min;
  return (float) (min + delta * _uniform_dist(_random_engine));
}

inline int linearScale(float min, float max, float a, float b, float x) {
  return std::floor((((b - a) * (x - min)) / (max - min)) + a);
  //return value >= b ? (b - 1) : (value < a ? a : value);
}

void generate_dummy_data(std::shared_ptr<TSchema> &schema, uint32_t number_of_elts) {
  //std::vector<uint32_t> gender;
  //std::iota(gender.begin(), gender.end(), 0);

  // TODO add spatial dimension

  // binary output
  std::ofstream binary(schema->output, std::ios::out | std::ios::binary);

  BinaryHeader bin_header;

  // TODO add spatial dimension
  bin_header.bytes = 0;
  bin_header.records = number_of_elts;

  binary.write((char *) &bin_header, sizeof(BinaryHeader));

  for (auto &pair : schema->dimension_map) {

    bin_header.bytes += pair.second->bytes();

    switch (pair.second->type) {

      case dimesion_t::CATEGORICAL: {
        for (auto e = 0; e < number_of_elts; ++e) {
          auto d = dimesion_t::get<TCategorical>(pair.second.get());

          uint8_t formated_value;

          switch (d->bin_type) {
            case TCategorical::DISCRETE: {
              formated_value = std::floor(randUniform(0, d->discrete.size()));
            }
              break;
            case TCategorical::RANGE: {
              formated_value = std::floor(randUniform(0, d->range.size()));
            }
              break;
            case TCategorical::BINARY: {
              //formated_value = std::stoi(unformatted_data[d.column_index]);
              formated_value = randUniform(0, 1) < 0.5f ? 0 : 1;
            }
              break;

            case TCategorical::SEQUENTIAl: {
              uint32_t diff = d->sequential.second - d->sequential.first + 1;
              float rand = randUniform(d->sequential.first, d->sequential.second + 1);

              formated_value = linearScale(d->sequential.first, d->sequential.second + 1, 0, diff, rand);
            }
              break;

          }

          binary.write((char *) &formated_value, sizeof(uint8_t));
        }
      }
        break;

      case dimesion_t::TEMPORAL: {
        for (auto e = 0; e < number_of_elts; ++e) {
          auto d = dimesion_t::get<TTemporal>(pair.second.get());

          uint32_t formated_value;

          formated_value = randUniform(d->min, d->max);

          binary.write((char *) &formated_value, sizeof(uint32_t));
        }
      }
        break;

      case dimesion_t::SPATIAL: {
        for (auto e = 0; e < number_of_elts; ++e) {
          auto d = dimesion_t::get<TSpatial>(pair.second.get());

          coordinates_t formated_value;

          formated_value.lat = randUniform(0, 180) - 90.f;
          formated_value.lon = randUniform(0, 360) - 180.f;

          assert(formated_value.lat > -90.f);
          assert(formated_value.lat < 90.f);

          assert(formated_value.lon > -180.f);
          assert(formated_value.lon < 180.f);

          binary.write((char *) &formated_value, sizeof(coordinates_t));
        }
      }
        break;
    }
  }

  ////////////////////////////////////////////////////////

  // hack to update number of records and bytes

  binary.clear();                 // clear fail and eof bits
  binary.seekp(0, std::ios::beg); // back to the start!

  binary.write((char *) &bin_header, sizeof(BinaryHeader));

  binary.close();

  generate_xml(schema, bin_header.bytes);
}

void gerenate_from_csv(std::shared_ptr<TSchema> &schema) {
  // transform csv to binary

  // binary output
  std::ofstream binary(schema->output, std::ios::out | std::ios::binary);

  BinaryHeader bin_header;

  // sum to record size
  bin_header.bytes = 0;
  bin_header.records = 0;

  binary.write((char *) &bin_header, sizeof(BinaryHeader));

  // source: https://bravenewmethod.com/2016/09/17/quick-and-robust-c-csv-reader-with-boost/

  // used to split the file in lines
  const boost::regex linesregx("\\r\\n|\\n\\r|\\n|\\r");

  // used to split each line to tokens, assuming ',' as column separator
  const boost::regex fieldsregx(",(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))");

  namespace bt = boost::posix_time;
  const bt::ptime timet_start(boost::gregorian::date(1970, 1, 1));

  std::string line;
  std::ifstream infile(schema->input);

  for (auto &pair : schema->dimension_map) {

    bin_header.bytes += pair.second->bytes();

    // reset to begin
    bin_header.records = 0;
    infile.clear();                 // clear fail and eof bits
    infile.seekg(0, std::ios::beg); // back to the start!

    // skip csv header
    if (schema->header) {
      std::getline(infile, line);
    }

    while (!infile.eof()) {
      std::getline(infile, line);

      try {
        // update number of records
        ++bin_header.records;

        // split line to tokens
        boost::sregex_token_iterator ti(line.begin(), line.end(), fieldsregx, -1);
        boost::sregex_token_iterator ti_end;

        std::vector<std::string> unformatted_data(ti, ti_end);

        switch (pair.second->type) {
          case dimesion_t::CATEGORICAL: {
            auto d = dimesion_t::get<TCategorical>(pair.second.get());

            uint8_t formated_value;

            switch (d->bin_type) {
              case TCategorical::DISCRETE: {
                auto it = std::find(d->discrete.begin(), d->discrete.end(), unformatted_data[d->column_index]);
                formated_value = it - d->discrete.begin();
              }
                break;
              case TCategorical::RANGE: {
                auto it =
                    std::lower_bound(d->range.begin(), d->range.end(), std::stof(unformatted_data[d->column_index]));
                formated_value = it - d->range.begin();
              }
                break;

              case TCategorical::BINARY: {
                formated_value = std::stoi(unformatted_data[d->column_index]);
              }
                break;

              case TCategorical::SEQUENTIAl: {
                uint32_t diff = d->sequential.second - d->sequential.first + 1;
                float rand = std::stof(unformatted_data[d->column_index]);

                formated_value = linearScale(d->sequential.first, d->sequential.second + 1, 0, diff, rand);
              }
                break;
            }

            binary.write((char *) &formated_value, sizeof(uint8_t));
          }
            break;

          case dimesion_t::TEMPORAL: {
            auto d = dimesion_t::get<TTemporal>(pair.second.get());

            uint32_t formated_value;

            bt::ptime pt;
            std::locale format(std::locale::classic(), new bt::time_input_facet(d->format.c_str()));

            std::istringstream is(unformatted_data[d->column_index]);
            is.imbue(format);
            is >> pt;

            bt::time_duration diff = pt - timet_start;

            formated_value = diff.total_seconds();

            binary.write((char *) &formated_value, sizeof(uint32_t));
          }
            break;

          case dimesion_t::SPATIAL: {
            auto d = dimesion_t::get<TSpatial>(pair.second.get());

            coordinates_t formated_value;

            // lat
            formated_value.lat = std::stof(unformatted_data[d->column_index]);

            // lon -> lat column + 1
            formated_value.lon = std::stof(unformatted_data[d->column_index + 1]);

            std::cout << formated_value.lat << ":" << formated_value.lon << std::endl;

            binary.write((char *) &formated_value, sizeof(coordinates_t));
          }
            break;
        }

      } catch (std::invalid_argument) {
        std::cerr << "error: line [" << line << "]" << std::endl;
      }
    }
  }

  infile.close();

  ////////////////////////////////////////////////////////

  // hack to update number of records and bytes

  binary.clear();                 // clear fail and eof bits
  binary.seekp(0, std::ios::beg); // back to the start!

  binary.write((char *) &bin_header, sizeof(BinaryHeader));

  binary.close();

  generate_xml(schema, bin_header.bytes);
}

std::shared_ptr<TSchema> read_xml_schema(const std::string &xml_input) {
  // read xml into boost property tree

  auto schema = std::make_shared<TSchema>();

  pt::ptree tree;
  pt::read_xml(xml_input, tree);

  // config
  auto &config = tree.get_child("config");
  schema->input = config.get("input", "");
  schema->header = config.get("input.<xmlattr>.header", false);

  schema->output = config.get("output", "");

  tree.get("<xmlattr>.ver", "0.0");
  auto &cschema = tree.get_child("config.schema");

  for (auto &d : cschema) {
    if (d.first == "categorical") {
      // initialize categorical dimension
      auto dimension = schema->get<TCategorical>(schema->dimension_map.size());

      // read attributes
      dimension->column_index = d.second.get("<xmlattr>.index", 0);

      auto type = d.second.get("<xmlattr>.type", "discrete");
      if (type == "discrete") {
        dimension->bin_type = TCategorical::DISCRETE;
      } else if (type == "range") {
        dimension->bin_type = TCategorical::RANGE;
      } else if (type == "binary") {
        dimension->bin_type = TCategorical::BINARY;
      } else if (type == "sequential") {
        dimension->bin_type = TCategorical::SEQUENTIAl;
      }

      for (auto &bins : d.second.get_child("bins", pt::ptree())) {
        // store bin info
        if (bins.first == "bin") {

          switch (dimension->bin_type) {
            case TCategorical::DISCRETE: {
              dimension->discrete.emplace_back(bins.second.get("key", ""));
            }
              break;
            case TCategorical::RANGE: {
              dimension->range.emplace_back(bins.second.get("max", 0));
            }
              break;
            case TCategorical::BINARY: {
              // nothing
            }
              break;

            case TCategorical::SEQUENTIAl: {
              dimension->sequential = std::make_pair(bins.second.get("min", 0), bins.second.get("max", 0));
            }
              break;
          }
        }
      }

    } else if (d.first == "temporal") {
      // initialize temporal dimension
      auto dimension = schema->get<TTemporal>(schema->dimension_map.size());

      // read attributes
      dimension->column_index = d.second.get("<xmlattr>.index", 0);

      for (auto &bins : d.second.get_child("bins", pt::ptree())) {
        // store bin info
        if (bins.first == "bin") {
          // convert date to epoch
          auto min = string_util::split(bins.second.get("min", ""), "/");
          dimension->min = date_util::mkgmtime(std::stoi(min[2]), std::stoi(min[1]), std::stoi(min[0]));

          // convert date to epoch
          auto max = string_util::split(bins.second.get("max", ""), "/");
          dimension->max = date_util::mkgmtime(std::stoi(max[2]), std::stoi(max[1]), std::stoi(max[0]));

          dimension->interval = std::stoi(bins.second.get("interval", ""));

          dimension->format = bins.second.get("format", "");
        }
      }

    } else if (d.first == "spatial") {
      // initialize temporal dimension
      auto dimension = schema->get<TSpatial>(schema->dimension_map.size());

      // read attributes
      dimension->column_index = d.second.get("<xmlattr>.index", 0);

      for (auto &bins : d.second.get_child("bins", pt::ptree())) {
        // store bin info
        if (bins.first == "bin") {
          // nothing
        }
      }
    }
  }

  return schema;
}

int main(int argc, char *argv[]) {
  bool read_from_csv = true;
  uint32_t number_of_elts = 0;
  std::string xml_input = "schema.xml";

  // declare the supported options
  po::options_description desc("Command Line Arguments");

  desc.add_options()("input,i", po::value<std::string>(&xml_input)->default_value(xml_input),
                     "xml input file");

  desc.add_options()("csv,c", po::value<bool>(&read_from_csv)->default_value(read_from_csv),
                     "read from csv");

  desc.add_options()("elts,e", po::value<uint32_t>(&number_of_elts)->default_value(number_of_elts),
                     "number of elements to generate");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
  po::notify(vm);

  ////////////////////////////////////////////////////////

  auto schema = read_xml_schema(xml_input);

  if (read_from_csv) {
    gerenate_from_csv(schema);
  } else {
    generate_dummy_data(schema, number_of_elts);
  }

  return 0;
}