#include "stdafx.h"

#include "date_util.h"
#include "string_util.h"

struct Translate {
  Translate() = default;

  Translate(std::string _name, std::vector<std::string> _key, std::vector<uint32_t> _value)
      : name(_name), key(_key), value(_value) {};

  uint32_t find(const std::string &str) const {
    auto iter = std::find(key.begin(), key.end(), str);
    return value[uint32_t(iter - key.begin())];
  }

  std::string name;
  std::vector<std::string> key;
  std::vector<uint32_t> value;
};

struct Nanocube {
  std::string instance;
  std::string tseries;
  std::string spatial;
  uint32_t from_bin, to_bin, bucket_size;
  std::unordered_map<std::string, Translate> schemata;
};

inline void nanocubes_log_tseries(const Nanocube &cube,
                                  std::string &query,
                                  const std::string &from,
                                  const std::string &to) {
  // [dimension_name].interval.([lower_bound]:[upper_bound])

  uint32_t to_days;
  uint32_t from_days = (cube.bucket_size * std::stoi(from)) + cube.from_bin;

  if (to == "10000000000") to_days = cube.to_bin;
  else to_days = (cube.bucket_size * (std::stoi(to) + std::stoi(from))) + cube.from_bin;

  query += "/const=" + cube.tseries;

  query += ".interval.(" + std::to_string(from_days) + ":" + std::to_string(to_days) + ")";
}

inline void nanocubes_log_fields(const Nanocube &cube, std::string &query, const std::string &str) {
  // [dimension_name].values.([value_0]:[value_1]:...:[value_N])

  auto fields = string_util::split(str, ";");
  for (auto &clause : fields) {
    auto literals = string_util::split(clause, std::regex("=|\\|"));

    query += "/const=" + cube.schemata.at(literals[0]).name + ".values.(";

    std::vector<int> values;
    for (size_t i = 1; i < literals.size(); i++) {
      query += std::to_string(cube.schemata.at(literals[0]).find(literals[i])) + ":";
    }
    query = query.substr(0, query.size() - 1);
    query += ")";
  }
}

void nanocubes_log(const std::vector<std::string> &files) {
  Nanocube flights, brightkite, gowalla, twitter;

  // flights
  flights.instance = "on_time_performance";
  flights.spatial = "origin_airport";
  flights.tseries = "crs_dep_time";
  flights.from_bin = date_util::mkgmtime(1987, 1, 1);
  flights.to_bin = date_util::mkgmtime(2009, 1, 1);
  flights.bucket_size = 14400;
  flights.schemata.insert({"ontime", Translate("on_time",
                                               {
                                                   "61 _min_early", "31_60_min_early", "16_30_min_early",
                                                   "6_15_min_early", "5_min_earlylate", "6_15_min_late",
                                                   "16_30_min_late", "31_60_min_late", "61 _min_late"
                                               },
                                               {
                                                   0, 1, 2, 3, 4, 5, 6, 7, 8
                                               }
  )});

  flights.schemata.insert({"carrier", Translate("unique_carrier",
                                                {
                                                    "Pacific_Southwest", "TWA", "United", "Southwest", "Eastern",
                                                    "America_West", "Northwest", "Pan_Am", "Piedmont", "Continental",
                                                    "Delta", "American", "US_Air", "Alaska", "Midway", "Aloha",
                                                    "American_Eagle", "Skywest", "Expressjet", "ATA",
                                                    "Altantic_Southest", "AirTran", "JetBlue", "Independence",
                                                    "Hawaiian", "Comair", "Frontier", "Mesa", "Pinnacle"
                                                },
                                                {
                                                    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
                                                    19, 20, 21, 22, 23, 24, 25, 26, 27, 28
                                                }
  )});

  // brightkite
  brightkite.instance = "brightkite";
  brightkite.spatial = "coord";
  brightkite.tseries = "time";
  brightkite.from_bin = date_util::mkgmtime(2008, 1, 1);
  brightkite.to_bin = date_util::mkgmtime(2010, 12, 1);
  brightkite.bucket_size = 604800;
  brightkite.schemata.insert({"day_of_week", Translate("day_of_week",
                                                       {
                                                           "Mon", "Tue", "Wed", "Thr", "Fri", "Sat", "Sun"
                                                       },
                                                       {
                                                           0, 1, 2, 3, 4, 5, 6
                                                       }
  )});

  brightkite.schemata.insert({"hour_of_day", Translate("hour_of_day",
                                                       {
                                                           "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
                                                           "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                                                           "20", "21", "22", "23"
                                                       },
                                                       {
                                                           0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                                                           18, 19, 20, 21, 22, 23
                                                       }
  )});

  // gowalla
  gowalla.instance = "gowalla";
  gowalla.spatial = "coord";
  gowalla.tseries = "time";
  gowalla.from_bin = date_util::mkgmtime(2008, 1, 1);
  gowalla.to_bin = date_util::mkgmtime(2010, 12, 1);
  gowalla.bucket_size = 604800;
  gowalla.schemata.insert({"dayofweek", Translate("day_of_week",
                                                  {
                                                      "Mon", "Tue", "Wed", "Thr", "Fri", "Sat", "Sun"
                                                  },
                                                  {
                                                      0, 1, 2, 3, 4, 5, 6
                                                  }
  )});

  gowalla.schemata.insert({"hour", Translate("hour_of_day",
                                             {
                                                 "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
                                                 "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                                                 "20", "21", "22", "23"
                                             },
                                             {
                                                 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                                                 18, 19, 20, 21, 22, 23
                                             }
  )});

  // twitter
  twitter.instance = "twitter-small";
  twitter.spatial = "coord";
  twitter.tseries = "time";
  twitter.from_bin = date_util::mkgmtime(2011, 11, 1);
  twitter.to_bin = date_util::mkgmtime(2012, 11, 1);
  twitter.bucket_size = 14400;
  twitter.schemata.insert({"device", Translate("device",
                                               {
                                                   "none", "iphone", "android", "ipad", "windows"
                                               },
                                               {
                                                   0, 1, 2, 3, 4
                                               }
  )});

  std::unordered_map<std::string, Nanocube>
      nanocubes{{"13", flights}, {"14", brightkite}, {"12", gowalla}, {"15", twitter}};

  for (auto &file : files) {
    std::ifstream infile(file);

    while (!infile.eof()) {

      std::string line;
      std::getline(infile, line);

      std::string query = "/api/query";

      try {
        if (line.empty()) continue;

        auto record = string_util::split(line, ",");

        if (record.size() != 3) continue;

        auto url = string_util::split(record[0], "/");

        if (url.size() < 12) continue;

        query += "/dataset=" + nanocubes[url[4]].instance;
        query += "/aggr=count";

        if (url[5] == "tile") {
          // [dimension_name].tile.([x]:[y]:[z]:[resolution])

          query += "/const=" + nanocubes[url[4]].spatial;

          query += ".tile";

          // url[6] -> zoom
          // url[7] -> resolution
          // url[8] -> x
          // url[9] -> y

          query += ".(" + url[8] + ":" + url[9] + ":" + url[6] + ":" + url[7] + ")";

          nanocubes_log_tseries(nanocubes[url[4]], query, url[10], url[11]);

          if (url.size() > 12) nanocubes_log_fields(nanocubes[url[4]], query, url[12]);

        } else if (url[5] == "query") {
          // [dimension_name].region.([x0]:[y0]:[x1]:[y1]:[z])

          query += "/const=" + nanocubes[url[4]].spatial;

          query += ".region";

          // url[7] -> zoom
          // url[8] -> x0
          // url[9] -> y0
          // url[10] -> x1
          // url[11] -> y1

          // zoom
          if (url[7] == "undefined") {
            throw std::invalid_argument("malformed url");
          }

          query += ".(" + url[8] + ":" + url[9] + ":" + url[10] + ":" + url[11] + ":" + url[7] + ")";

          if (url.size() == 18) {
            nanocubes_log_fields(nanocubes[url[4]], query, url[13]);
            nanocubes_log_tseries(nanocubes[url[4]], query, url[15], url[16]);
          } else if (url.size() > 13) {
            if (url[12] == "where") nanocubes_log_fields(nanocubes[url[4]], query, url[13]);
            else if (url[12] == "tseries") nanocubes_log_tseries(nanocubes[url[4]], query, url[13], url[14]);
          }
        }
      } catch (const std::invalid_argument &e) {
        std::cerr << "error: invalid nanocubes query [" << e.what() << "]" << std::endl;
      }

      std::cout << query << std::endl;
    }
    infile.close();
  }
}

int main(int argc, char *argv[]) {
  std::vector<std::string> files;

  if (argc < 2) {
    std::cerr << "error: invalid argument" << std::endl;
    exit(-1);
  } else {
    for (int i = 1; i < argc; i++) {

      std::string arg = argv[i];

      files.emplace_back(argv[i]);
    }
  }

  nanocubes_log(files);

  return 0;
}

