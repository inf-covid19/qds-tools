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

template<typename T>
struct dimesion_t {
  uint32_t offset{0};
  uint32_t csv_index{0};
  uint32_t binary_index{0};

  T temporary;

  std::vector<T> data;

  virtual inline uint32_t bins() const {
    return 1;
  };

  virtual inline bool invalid_data(const T &value) {
    temporary = value;
    return false;
  };

  inline uint32_t bytes() {
    return sizeof(T);
  };
};

struct TPayload : public dimesion_t<float> {
};

struct TSpatial : public dimesion_t<coordinates_t> {
  union {
    uint32_t csv_index{0};
    struct {
      uint32_t csv_index_lat : 16;
      uint32_t csv_index_lon : 16;
    };
  };

  uint32_t bin{1};

  inline uint32_t bins() const override {
    return bin;
  }

  inline bool invalid_data(const coordinates_t &value) override {
    temporary = value;
    if (temporary.lat < -90.f || temporary.lat > 90.f) {
      return true;
    } else if (temporary.lon < -180.f || temporary.lat > 180.f) {
      return true;
    } else {
      return false;
    }
  }
};

struct TTemporal : public dimesion_t<uint32_t> {
  uint32_t interval;
  std::string format;

  inline uint32_t bins() const override {
    return interval;
  }

  inline bool invalid_data(const uint32_t &value) override {
    temporary = value;
    if (temporary < 0 || temporary > 2147472000) {
      return true;
    } else {
      return false;
    }
  }
};

struct TCategorical : dimesion_t<uint8_t> {
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

  inline uint32_t bins() const override {
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

  inline bool invalid_data(const uint8_t &value) override {
    temporary = value;
    if (temporary < 0 || temporary >= bins()) {
      return true;
    } else {
      return false;
    }
  }
};

using TDimension = std::variant<TSpatial, TCategorical, TTemporal, TPayload>;

struct TSchema {
  uint32_t lines_to_skip{0};

  std::string input;
  std::string output;
  std::string output_dir;

  std::vector<TDimension> dimensions;
};
