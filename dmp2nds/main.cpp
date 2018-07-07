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

      node.put("index", d->name);
      node.put("bin", d->bin());
      node.put("offset", d->offset);

      schema_node.add_child("spatial", node);

    } else if (auto d = std::get_if<TCategorical>(&variant)) {
      pt::ptree node;

      node.put("index", d->name);
      node.put("bin", d->bin());
      node.put("offset", d->offset);

      schema_node.add_child("categorical", node);

    } else if (auto d = std::get_if<TTemporal>(&variant)) {
      pt::ptree node;

      node.put("index", d->name);
      node.put("bin", d->bin());
      node.put("offset", d->offset);

      schema_node.add_child("temporal", node);

    } else if (auto d = std::get_if<TPayload>(&variant)) {
      pt::ptree node;

      node.put("index", d->name);
      node.put("bin", d->bin());
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

inline int linearScale(double min, double max, double a, double b, double x) {
  return std::floor((((b - a) * (x - min)) / (max - min)) + a);
  //return value >= b ? (b - 1) : (value < a ? a : value);
}

void read_dmp_file(TSchema &schema, BinaryHeader &bin_header, const std::string &file) {
  namespace bt = boost::posix_time;
  const bt::ptime timet_start(boost::gregorian::date(1970, 1, 1));

  std::vector<uint8_t> dmp_record;
  uint32_t dmp_size = 0;

  // reserve memory
  for (auto &variant : schema.dimensions) {
    if (auto d = std::get_if<TSpatial>(&variant)) {
      dmp_size += d->size * 2;
    } else if (auto d = std::get_if<TCategorical>(&variant)) {
      dmp_size += d->size;
    } else if (auto d = std::get_if<TTemporal>(&variant)) {
      dmp_size += d->size;
    } else if (auto d = std::get_if<TPayload>(&variant)) {
      dmp_size += d->size;
    }
  }

  // allocate record size
  dmp_record.resize(dmp_size);

  std::ifstream infile(file, std::ios::binary);

  // skip lines
  infile.unsetf(std::ios_base::skipws);

  for (int i = 0; i < schema.lines_to_skip; ++i) {
    infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  while (!infile.eof()) {
    try {
      // read dmp record
      infile.read((char *) &dmp_record[0], dmp_size);

      bool invalid = false;
      for (auto &variant : schema.dimensions) {
        // formated templates
        coordinates_t formated_spatial;
        uint16_t formated_categorical{0};
        uint32_t formated_temporal{0};
        float formated_payload{0.f};

        if (auto d = std::get_if<TSpatial>(&variant)) {
          // lat
          std::memcpy(&formated_spatial.lat, &dmp_record[d->index_lat], d->size);

          // lon
          std::memcpy(&formated_spatial.lon, &dmp_record[d->index_lon], d->size);

          if (invalid = d->invalid_data(formated_spatial)) {
            break;
          }

        } else if (auto d = std::get_if<TCategorical>(&variant)) {
          std::memcpy(&formated_categorical, &dmp_record[d->index], d->size);

          switch (d->bin_type) {
            case TCategorical::RANGE: {
              auto it = std::lower_bound(d->range.begin(), d->range.end(), formated_categorical);
              formated_categorical = it - d->range.begin();
            }
              break;
            case TCategorical::SEQUENTIAl: {
              uint32_t diff = d->sequential.second - d->sequential.first + 1;
              float rand = formated_categorical;

              formated_categorical = linearScale(d->sequential.first, d->sequential.second + 1, 0, diff, rand);
            }
              break;
          }

          if (invalid = d->invalid_data(formated_categorical)) {
            break;
          }

        } else if (auto d = std::get_if<TTemporal>(&variant)) {

          /* time_t time(record.time);
          struct tm *ptm = std::gmtime(&time);

          generator.addInt("tseries", util::mkgmtime(ptm)); */

          uint64_t raw = 0;
          std::memcpy(&raw, &dmp_record[d->index], d->size);

          bt::ptime pt;

          std::istringstream is(std::to_string(raw));
          is.imbue(d->format);
          is >> pt;

          bt::time_duration diff = pt - timet_start;

          formated_temporal = diff.total_seconds();

          if (invalid = d->invalid_data(formated_temporal)) {
            break;
          }

        } else if (auto d = std::get_if<TPayload>(&variant)) {
          std::memcpy(&formated_payload, &dmp_record[d->index], d->size);

          if (invalid = d->invalid_data(formated_payload)) {
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

  /*size_t num_lines = 0;

  // open file
  std::string line;
  std::ifstream infile(file);

  // count number of lines
  while (std::getline(infile, line)) {
    ++num_lines;
  }

  // reset file
  infile.clear();
  infile.seekg(0, std::ios::beg);

  std::cout << "log: parsing [" << file << "]" << std::endl;

  // source: https://bravenewmethod.com/2016/09/17/quick-and-robust-c-csv-reader-with-boost/
  // used to split the file in lines
  const boost::regex linesregx("\\r\\n|\\n\\r|\\n|\\r");

  // used to split each line to tokens, assuming ',' as column separator
  const boost::regex fieldsregx(sep + "(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))");

  namespace bt = boost::posix_time;
  const bt::ptime timet_start(boost::gregorian::date(1970, 1, 1));

  // read header
  std::getline(infile, line);

  // split line to tokens
  boost::sregex_token_iterator header_ti(line.begin(), line.end(), fieldsregx, -1);
  boost::sregex_token_iterator header_end;

  std::vector<std::string> header(header_ti, header_end);

  std::transform(header.begin(), header.end(), header.begin(), [](const std::string &lhs) {
    return boost::algorithm::to_lower_copy(lhs);
  });

  std::unordered_map<std::string, uint32_t> index_map;
  for (auto &variant : schema.dimensions) {

    if (auto d = std::get_if<TSpatial>(&variant)) {
      auto it_lat = std::find(header.begin(), header.end(), d->index_lat);
      auto it_lon = std::find(header.begin(), header.end(), d->index_lon);

      if (it_lat == header.end() || it_lon == header.end()) {
        std::cerr << "error: invalid spatial column [" << file << "]" << std::endl;
        return;
      }

      index_map[(*it_lat)] = it_lat - header.begin();
      index_map[(*it_lon)] = it_lon - header.begin();

    } else if (auto d = std::get_if<TCategorical>(&variant)) {
      auto it = std::find(header.begin(), header.end(), d->index);

      if (it == header.end()) {
        std::cerr << "error: invalid categorical column [" << file << "]" << std::endl;
        return;
      }

      index_map[(*it)] = it - header.begin();

    } else if (auto d = std::get_if<TTemporal>(&variant)) {
      auto it = std::find(header.begin(), header.end(), d->index);

      if (it == header.end()) {
        std::cerr << "error: invalid temporal column [" << file << "]" << std::endl;
        return;
      }

      index_map[(*it)] = it - header.begin();

    } else if (auto d = std::get_if<TPayload>(&variant)) {
      auto it = std::find(header.begin(), header.end(), d->index);

      if (it == header.end()) {
        std::cerr << "error: invalid payload column [" << file << "]" << std::endl;
        std::cerr << d->index << std::endl;

        for (auto &ptr : header) {
          std::cerr << "[" << ptr << "]" << std::endl;
        }
        return;
      }

      index_map[(*it)] = it - header.begin();
    }
  }

  // reserve memory
  for (auto &variant : schema.dimensions) {
    if (auto d = std::get_if<TSpatial>(&variant)) {
      d->data.reserve(d->data.size() + num_lines);
    } else if (auto d = std::get_if<TCategorical>(&variant)) {
      d->data.reserve(d->data.size() + num_lines);
    } else if (auto d = std::get_if<TTemporal>(&variant)) {
      d->data.reserve(d->data.size() + num_lines);
    } else if (auto d = std::get_if<TPayload>(&variant)) {
      d->data.reserve(d->data.size() + num_lines);
    }
  }

  // formated templates
  coordinates_t formated_spatial;
  uint16_t formated_categorical;
  uint32_t formated_temporal;
  float formated_payload;

  while (!infile.eof()) {
    std::getline(infile, line);

    if (line.empty()) {
      continue;
    }

    try {
      // split line to tokens
      boost::sregex_token_iterator ti(line.begin(), line.end(), fieldsregx, -1);
      boost::sregex_token_iterator ti_end;

      std::vector<std::string> unformatted_data(ti, ti_end);

      // ignore invalid lines
      if (unformatted_data.size() < schema.dimensions.size()) {
        continue;
      }

      std::transform(unformatted_data.begin(),
                     unformatted_data.end(),
                     unformatted_data.begin(),
                     [](const std::string &lhs) {
                       return boost::algorithm::to_lower_copy(lhs);
                     });

      bool invalid = false;
      for (auto &variant : schema.dimensions) {
        if (auto d = std::get_if<TSpatial>(&variant)) {
          if (unformatted_data[index_map[d->index_lat]].empty()) {
            invalid = true;
            break;
          } else if (unformatted_data[index_map[d->index_lon]].empty()) {
            invalid = true;
            break;
          }

          // lat
          formated_spatial.lat = std::stof(unformatted_data[index_map[d->index_lat]]);

          // lon
          formated_spatial.lon = std::stof(unformatted_data[index_map[d->index_lon]]);

          if (invalid = d->invalid_data(formated_spatial)) {
            break;
          }

        } else if (auto d = std::get_if<TCategorical>(&variant)) {
          if (unformatted_data[index_map[d->index]].empty()) {
            invalid = true;
            break;
          }

          switch (d->bin_type) {
            case TCategorical::RANGE: {
              auto it =
                  std::lower_bound(d->range.begin(), d->range.end(), std::stof(unformatted_data[index_map[d->index]]));
              formated_categorical = it - d->range.begin();
            }
              break;

            case TCategorical::BINARY: {
              formated_categorical = std::stoi(unformatted_data[index_map[d->index]]);
            }
              break;

            case TCategorical::SEQUENTIAl: {
              uint32_t diff = d->sequential.second - d->sequential.first + 1;
              float rand = std::stof(unformatted_data[index_map[d->index]]);

              formated_categorical = linearScale(d->sequential.first, d->sequential.second + 1, 0, diff, rand);
            }
              break;
          }

          if (invalid = d->invalid_data(formated_categorical)) {
            break;
          }

        } else if (auto d = std::get_if<TTemporal>(&variant)) {
          if (unformatted_data[index_map[d->index]].empty()) {
            invalid = true;
            break;
          }

          bt::ptime pt;

          std::istringstream is(unformatted_data[index_map[d->index]]);
          is.imbue(d->format);
          is >> pt;

          bt::time_duration diff = pt - timet_start;

          formated_temporal = diff.total_seconds();

          if (invalid = d->invalid_data(formated_temporal)) {
            break;
          }

        } else if (auto d = std::get_if<TPayload>(&variant)) {
          if (unformatted_data[index_map[d->index]].empty()) {
            invalid = true;
            break;
          }

          // payload
          formated_payload = std::stof(unformatted_data[index_map[d->index]]);

          if (invalid = d->invalid_data(formated_payload)) {
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
  }*/

  // close input file
  infile.close();
}

// transform csv to binary
void generate_from_dmp(TSchema &schema) {
  BinaryHeader bin_header;

  // sum to record size
  bin_header.bytes = 0;
  bin_header.records = 0;

  // list all files in folder
  if (boost::filesystem::is_directory(schema.input_dir)) {
    std::cout << "log: " << schema.input_dir << " is a directory containing:" << std::endl;

    for (auto &entry : boost::make_iterator_range(boost::filesystem::directory_iterator(schema.input_dir), {}))
      read_dmp_file(schema, bin_header, entry.path().string());

  } else if (boost::filesystem::is_regular_file(schema.input)) {
    read_dmp_file(schema, bin_header, schema.input);
  }

  ////////////////////////////////////////////////////////
  std::cout << "log: writing binary file" << std::endl;

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

  std::cout << "log: elts_n: " << bin_header.records << std::endl;
  std::cout << "log: elts_size: " << bin_header.bytes << std::endl;

  // writer formatted values
  for (auto &variant : schema.dimensions) {
    if (auto d = std::get_if<TSpatial>(&variant)) {
      binary.write((char *) &d->data[0], d->data.size() * d->bytes());
    } else if (auto d = std::get_if<TCategorical>(&variant)) {
      binary.write((char *) &d->data[0], d->data.size() * d->bytes());
    } else if (auto d = std::get_if<TTemporal>(&variant)) {
      binary.write((char *) &d->data[0], d->data.size() * d->bytes());
    } else if (auto d = std::get_if<TPayload>(&variant)) {
      binary.write((char *) &d->data[0], d->data.size() * d->bytes());
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
  schema.input_dir = config.get("input-dir", "");

  schema.output = config.get("output", "output");
  schema.output_dir = config.get("output-dir", "./");

  tree.get("<xmlattr>.ver", "0.0");
  auto &cschema = tree.get_child("config.schema");

  uint32_t offset = 0;

  for (auto &d : cschema) {
    if (d.first == "categorical") {
      // initialize categorical dimension
      auto dimension = TCategorical();

      // offset
      dimension.offset = offset;
      offset += dimension.bytes();

      // read attributes
      dimension.index = d.second.get("<xmlattr>.offset", 0);
      dimension.size = d.second.get("<xmlattr>.bytes", 0);
      dimension.name = d.second.get("<xmlattr>.name", "");

      if (dimension.name.empty()) {
        std::cerr << "error: invalid csv schema" << std::endl;
        continue;
      }

      auto type = d.second.get("<xmlattr>.type", "discrete");
      if (type == "range") {
        dimension.bin_type = TCategorical::RANGE;
      } else if (type == "binary") {
        dimension.bin_type = TCategorical::BINARY;
      } else if (type == "sequential") {
        dimension.bin_type = TCategorical::SEQUENTIAl;
      }

      switch (dimension.bin_type) {
        case TCategorical::RANGE: {
          for (auto &attr : d.second.get_child("attributes", pt::ptree())) {
            auto min = attr.second.get("min", 0);
            auto max = attr.second.get("max", 0);
            dimension.range.emplace_back(max);
          }
        }
          break;

        case TCategorical::SEQUENTIAl: {
          auto min = d.second.get("attributes.min", 0);
          auto max = d.second.get("attributes.max", 0);
          dimension.sequential = std::make_pair(min, max);
        }
          break;
      }

      schema.dimensions.emplace_back(dimension);

    } else if (d.first == "temporal") {
      // initialize temporal dimension
      auto dimension = TTemporal();

      // offset
      dimension.offset = offset;
      offset += dimension.bytes();

      // read attributes
      dimension.index = d.second.get("<xmlattr>.offset", 0);
      dimension.size = d.second.get("<xmlattr>.bytes", 0);
      dimension.name = d.second.get("<xmlattr>.name", "");

      if (dimension.name.empty()) {
        std::cerr << "error: invalid csv schema" << std::endl;
        continue;
      }

      dimension.interval = d.second.get("attributes.interval", 3600);

      std::string format_str = d.second.get("attributes.format", "%d/%m/%Y-%H:%M");

      namespace bt = boost::posix_time;
      dimension.format = std::locale(std::locale::classic(), new bt::time_input_facet(format_str.c_str()));

      schema.dimensions.emplace_back(dimension);

    } else if (d.first == "spatial") {
      // initialize temporal dimension
      auto dimension = TSpatial();

      // offset
      dimension.offset = offset;
      offset += dimension.bytes();

      // read attributes
      dimension.index_lat = d.second.get("<xmlattr>.offset-lat", 0);
      dimension.index_lon = d.second.get("<xmlattr>.offset-lon", 0);
      dimension.size = d.second.get("<xmlattr>.bytes", 0);
      dimension.name = d.second.get("<xmlattr>.name", "");

      if (dimension.name.empty()) {
        std::cerr << "error: invalid csv schema" << std::endl;
        continue;
      }

      dimension.bins = d.second.get("attributes.bin", 0);

      schema.dimensions.emplace_back(dimension);

    } else if (d.first == "payload") {
      // initialize temporal dimension
      auto dimension = TPayload();

      // offset
      dimension.offset = offset;
      offset += dimension.bytes();

      // read attributes
      dimension.index = d.second.get("<xmlattr>.offset", 0);
      dimension.size = d.second.get("<xmlattr>.bytes", 0);
      dimension.name = d.second.get("<xmlattr>.name", "");

      if (dimension.name.empty()) {
        std::cerr << "error: invalid csv schema" << std::endl;
        continue;
      }

      dimension.type = (TPayload::EBinType) d.second.get("attributes.type", 0);

      schema.dimensions.emplace_back(dimension);
    }
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

  generate_from_dmp(schema);

  return 0;
}