//
// Created by cicerolp on 1/8/18.
//

#pragma once

namespace string_util {

std::vector<std::string> split(const std::string &input, const std::regex &regex) {
  static const std::sregex_token_iterator last;
  return std::vector<std::string> {std::sregex_token_iterator{input.begin(), input.end(), regex, -1}, last};
}

std::vector<std::string> split(const std::string &input, const std::string &str_regex) {
  static const std::sregex_token_iterator last;
  std::regex regex(str_regex);
  return std::vector<std::string> {std::sregex_token_iterator{input.begin(), input.end(), regex, -1}, last};
}

std::string next_token(std::vector<std::string>::const_iterator &it) {
  return *(++it);
}
} // namespace string_util

struct BinaryHeader {
  uint32_t bytes;
  uint32_t records;
};

struct coordinates_t {
  float lat, lon;
};

struct dimesion_t {
  enum EType {
    CATEGORICAL,
    TEMPORAL,
    SPATIAL
  };

  EType type;
  uint32_t column_index;

  template<typename T>
  static inline T *get(dimesion_t *ptr) {
    return (T *) ptr;
  }

  virtual uint32_t bytes() const {
    return 0;
  }
};

struct TSpatial : public dimesion_t {
  TSpatial() {
    type = EType::SPATIAL;
  }

  uint32_t bytes() const override {
    return sizeof(coordinates_t);
  }
};

struct TTemporal : public dimesion_t {
  TTemporal() {
    type = EType::TEMPORAL;
  }

  uint32_t interval;
  std::string format;

  uint32_t bytes() const override {
    return sizeof(uint32_t);
  }
};

struct TCategorical : dimesion_t {
  TCategorical() {
    type = EType::CATEGORICAL;
  }

  enum EBinType {
    DISCRETE,
    RANGE,
    BINARY,
    SEQUENTIAl
  };

  EBinType bin_type;

  // min -> [nds index]
  std::vector<uint32_t> range;

  // key -> [nds index]
  std::vector<std::string> discrete;

  // [lower_bound, upper_bound]
  std::pair<uint32_t, uint32_t> sequential;

  inline uint32_t get_n_bins() const {
    switch (bin_type) {
      case TCategorical::DISCRETE: {
        return discrete.size();
      }
        break;
      case TCategorical::RANGE: {
        return range.size();
      }
        break;
      case TCategorical::BINARY: {
        return 2;
      }
        break;
      case TCategorical::SEQUENTIAl: {
        return sequential.second - sequential.first + 1;
      }
        break;
    }
  }

  uint32_t bytes() const override {
    return sizeof(uint8_t);
  }
};

struct TSchema {
  bool header;

  std::string input;
  std::string output;

  template<typename T>
  inline T *get(uint32_t key) {
    auto pair = dimension_map.find(key);

    if (pair == dimension_map.end()) {
      dimension_map[key] = std::make_unique<T>();
      pair = dimension_map.find(key);
    }

    return (T *) pair->second.get();
  }

  std::map<uint32_t, std::unique_ptr<dimesion_t>> dimension_map;
};
