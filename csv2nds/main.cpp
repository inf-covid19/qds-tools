#include "stdafx.h"
#include "types.h"
#include "date_util.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

void generate_xml(TSchema &schema, uint32_t bytes) {
  // generate xml info

  pt::ptree root;
  pt::ptree config_node;

  config_node.put("name", schema.output);
  config_node.put("bytes", bytes);
  config_node.put("file", schema.output + ".nds");

  pt::ptree schema_node;

  for (auto &variant : schema.dimensions) {
    if (auto d = std::get_if<TSpatial>(&variant)) {
      pt::ptree node;

      node.put("index", d->binary_index);
      node.put("bin", d->bins());
      node.put("offset", d->offset);

      schema_node.add_child("spatial", node);

    } else if (auto d = std::get_if<TCategorical>(&variant)) {
      pt::ptree node;

      node.put("index", d->binary_index);
      node.put("bin", d->bins());
      node.put("offset", d->offset);

      schema_node.add_child("categorical", node);

    } else if (auto d = std::get_if<TTemporal>(&variant)) {
      pt::ptree node;

      node.put("index", d->binary_index);
      node.put("bin", d->bins());
      node.put("offset", d->offset);

      schema_node.add_child("temporal", node);

    } else if (auto d = std::get_if<TPayload>(&variant)) {
      pt::ptree node;

      node.put("index", d->binary_index);
      node.put("bin", d->bins());
      node.put("offset", d->offset);

      schema_node.add_child("payload", node);
    }
  }

  config_node.add_child("schema", schema_node);

  root.add_child("config", config_node);

  std::ofstream output_xml(schema.output_dir + "/" + schema.output + ".xml", std::ios::out);

  pt::xml_writer_settings<boost::property_tree::ptree::key_type> settings('\t', 1);
  pt::write_xml(output_xml, root, settings);

  output_xml.close();
}

inline int linearScale(float min, float max, float a, float b, float x) {
  return std::floor((((b - a) * (x - min)) / (max - min)) + a);
  //return value >= b ? (b - 1) : (value < a ? a : value);
}

// transform csv to binary
void gerenate_from_csv(TSchema &schema) {
  BinaryHeader bin_header;

  // sum to record size
  bin_header.bytes = 0;
  bin_header.records = 0;

  // source: https://bravenewmethod.com/2016/09/17/quick-and-robust-c-csv-reader-with-boost/

  // used to split the file in lines
  const boost::regex linesregx("\\r\\n|\\n\\r|\\n|\\r");

  // used to split each line to tokens, assuming ',' as column separator
  const boost::regex fieldsregx(",(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))");

  namespace bt = boost::posix_time;
  const bt::ptime timet_start(boost::gregorian::date(1970, 1, 1));

  std::string line;
  std::ifstream infile(schema.input);

  // skip csv lines
  for (auto i = 0; i < schema.lines_to_skip; ++i) {
    std::getline(infile, line);
  }

  while (!infile.eof()) {
    std::getline(infile, line);

    try {
      // split line to tokens
      boost::sregex_token_iterator ti(line.begin(), line.end(), fieldsregx, -1);
      boost::sregex_token_iterator ti_end;

      std::vector<std::string> unformatted_data(ti, ti_end);

      // ignore invalid lines
      if (unformatted_data.size() < schema.dimensions.size()) {
        continue;
      }

      bool invalid = false;
      for (auto &variant : schema.dimensions) {
        if (auto d = std::get_if<TSpatial>(&variant)) {
          coordinates_t formated_value;

          // lat
          formated_value.lat = std::stof(unformatted_data[d->csv_index_lat]);

          // lon -> lat column + 1
          formated_value.lon = std::stof(unformatted_data[d->csv_index_lon]);

          if (invalid = d->invalid_data(formated_value)) {
            break;
          }

        } else if (auto d = std::get_if<TCategorical>(&variant)) {
          uint8_t formated_value;

          switch (d->bin_type) {
            case TCategorical::DISCRETE: {
              auto it = std::find(d->discrete.begin(), d->discrete.end(), unformatted_data[d->csv_index]);
              formated_value = it - d->discrete.begin();
            }
              break;
            case TCategorical::RANGE: {
              auto it =
                  std::lower_bound(d->range.begin(), d->range.end(), std::stof(unformatted_data[d->csv_index]));
              formated_value = it - d->range.begin();
            }
              break;

            case TCategorical::BINARY: {
              formated_value = std::stoi(unformatted_data[d->csv_index]);
            }
              break;

            case TCategorical::SEQUENTIAl: {
              uint32_t diff = d->sequential.second - d->sequential.first + 1;
              float rand = std::stof(unformatted_data[d->csv_index]);

              formated_value = linearScale(d->sequential.first, d->sequential.second + 1, 0, diff, rand);
            }
              break;
          }

          if (invalid = d->invalid_data(formated_value)) {
            break;
          }

        } else if (auto d = std::get_if<TTemporal>(&variant)) {
          uint32_t formated_value;

          bt::ptime pt;
          std::locale format(std::locale::classic(), new bt::time_input_facet(d->format.c_str()));

          std::istringstream is(unformatted_data[d->csv_index]);
          is.imbue(format);
          is >> pt;

          bt::time_duration diff = pt - timet_start;

          formated_value = diff.total_seconds();

          if (invalid = d->invalid_data(formated_value)) {
            break;
          }

        } else if (auto d = std::get_if<TPayload>(&variant)) {
          float formated_value;

          // payload
          formated_value = std::stof(unformatted_data[d->csv_index]);

          if (invalid = d->invalid_data(formated_value)) {
            break;
          }
        }
      }

      // invalid unformatted data
      if (invalid) {
        continue;
      }

      // emplace_back valid data
      for (auto &variant : schema.dimensions) {
        if (auto d = std::get_if<TSpatial>(&variant)) {
          d->data.emplace_back(d->temporary);
        } else if (auto d = std::get_if<TCategorical>(&variant)) {
          d->data.emplace_back(d->temporary);
        } else if (auto d = std::get_if<TTemporal>(&variant)) {
          d->data.emplace_back(d->temporary);
        } else if (auto d = std::get_if<TPayload>(&variant)) {
          d->data.emplace_back(d->temporary);
        }
      }

      // update number of records
      ++bin_header.records;

    } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }

  // close input file
  infile.close();

  ////////////////////////////////////////////////////////
  // binary output
  std::ofstream binary(schema.output_dir + "/" + schema.output + ".nds", std::ios::out | std::ios::binary);

  for (auto &variant : schema.dimensions) {
    if (auto d = std::get_if<TSpatial>(&variant)) {
      bin_header.bytes += d->bytes();
    } else if (auto d = std::get_if<TCategorical>(&variant)) {
      bin_header.bytes += d->bytes();
    } else if (auto d = std::get_if<TTemporal>(&variant)) {
      bin_header.bytes += d->bytes();
    } else if (auto d = std::get_if<TPayload>(&variant)) {
      bin_header.bytes += d->bytes();
    }
  }

  // write binary header
  binary.write((char *) &bin_header, sizeof(BinaryHeader));

  std::cout << "elts_n: " << bin_header.records << std::endl;
  std::cout << "elts_size: " << bin_header.bytes << std::endl;

  // writer formatted values
  for (auto &variant : schema.dimensions) {
    if (auto d = std::get_if<TSpatial>(&variant)) {
      binary.write((char *)&d->data[0], d->data.size() * d->bytes());
    } else if (auto d = std::get_if<TCategorical>(&variant)) {
      binary.write((char *)&d->data[0], d->data.size() * d->bytes());
    } else if (auto d = std::get_if<TTemporal>(&variant)) {
      binary.write((char *)&d->data[0], d->data.size() * d->bytes());
    } else if (auto d = std::get_if<TPayload>(&variant)) {
      binary.write((char *)&d->data[0], d->data.size() * d->bytes());
    }
  }

  binary.close();

  generate_xml(schema, bin_header.bytes);
}

// read xml into boost property tree
TSchema read_xml_schema(const std::string &xml_input) {
  TSchema schema;

  pt::ptree tree;
  pt::read_xml(xml_input, tree);

  // config
  auto &config = tree.get_child("config");
  schema.input = config.get("input", "");
  schema.lines_to_skip = config.get("input.<xmlattr>.lines-to-skip", 0);

  schema.output = config.get("output", "output");
  schema.output_dir = config.get("output-dir", "./");

  tree.get("<xmlattr>.ver", "0.0");
  auto &cschema = tree.get_child("config.schema");

  uint32_t offset = 0;
  uint32_t binary_index = 0;

  for (auto &d : cschema) {
    if (d.first == "categorical") {
      // initialize categorical dimension
      auto dimension = TCategorical();

      // offset
      dimension.offset = offset;
      offset += dimension.bytes();

      // binary index
      dimension.binary_index = binary_index;\

      // read attributes
      dimension.csv_index = d.second.get("<xmlattr>.index", 0);

      auto type = d.second.get("<xmlattr>.type", "discrete");
      if (type == "discrete") {
        dimension.bin_type = TCategorical::DISCRETE;
      } else if (type == "range") {
        dimension.bin_type = TCategorical::RANGE;
      } else if (type == "binary") {
        dimension.bin_type = TCategorical::BINARY;
      } else if (type == "sequential") {
        dimension.bin_type = TCategorical::SEQUENTIAl;
      }

      for (auto &bins : d.second.get_child("bins", pt::ptree())) {
        // store bin info
        if (bins.first == "bin") {

          switch (dimension.bin_type) {
            case TCategorical::DISCRETE: {
              dimension.discrete.emplace_back(bins.second.get("key", ""));
            }
              break;
            case TCategorical::RANGE: {
              dimension.range.emplace_back(bins.second.get("max", 0));
            }
              break;
            case TCategorical::BINARY: {
              // nothing
            }
              break;

            case TCategorical::SEQUENTIAl: {
              dimension.sequential = std::make_pair(bins.second.get("min", 0), bins.second.get("max", 0));
            }
              break;
          }
        }
      }

      schema.dimensions.emplace_back(dimension);

    } else if (d.first == "temporal") {
      // initialize temporal dimension
      auto dimension = TTemporal();

      // offset
      dimension.offset = offset;
      offset += dimension.bytes();

      // binary index
      dimension.binary_index = binary_index;

      // read attributes
      dimension.csv_index = d.second.get("<xmlattr>.index", 0);

      for (auto &bins : d.second.get_child("bins", pt::ptree())) {
        // store bin info
        if (bins.first == "bin") {
          dimension.interval = std::stoi(bins.second.get("interval", ""));

          dimension.format = bins.second.get("format", "");
        }
      }

      schema.dimensions.emplace_back(dimension);

    } else if (d.first == "spatial") {
      // initialize temporal dimension
      auto dimension = TSpatial();

      // offset
      dimension.offset = offset;
      offset += dimension.bytes();

      // binary index
      dimension.binary_index = binary_index;

      // read attributes
      dimension.csv_index_lat = d.second.get("<xmlattr>.index-lat", 0);
      dimension.csv_index_lon = d.second.get("<xmlattr>.index-lon", 0);

      for (auto &bins : d.second.get_child("bins", pt::ptree())) {
        // store bin info
        if (bins.first == "bin") {
          dimension.bin = bins.second.get("bin", 1);
        }
      }

      schema.dimensions.emplace_back(dimension);

    } else if (d.first == "payload") {
      // initialize temporal dimension
      auto dimension = TPayload();

      // offset
      dimension.offset = offset;
      offset += dimension.bytes();

      // binary index
      dimension.binary_index = binary_index;

      // read attributes
      dimension.csv_index = d.second.get("<xmlattr>.index", 0);

      for (auto &bins : d.second.get_child("bins", pt::ptree())) {
        // store bin info
        if (bins.first == "bin") {
          // nothing
        }
      }

      schema.dimensions.emplace_back(dimension);
    }

    ++binary_index;
  }

  return schema;
}

int main(int argc, char *argv[]) {
  std::string xml_input = "schema.xml";

  // declare the supported options
  po::options_description desc("Command Line Arguments");

  desc.add_options()("input,i", po::value<std::string>(&xml_input)->default_value(xml_input),
                     "xml input file");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
  po::notify(vm);

  ////////////////////////////////////////////////////////

  auto schema = read_xml_schema(xml_input);

  gerenate_from_csv(schema);

  return 0;
}