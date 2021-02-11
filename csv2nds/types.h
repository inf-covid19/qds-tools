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
  std::string name;
  std::string index;

  uint32_t offset{0};

  T temporary;

  std::vector<T> data;

  virtual inline uint32_t bin() const {
    return 0;
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
  enum EBinType {
    PDIGEST,
    GAUSSIAN
  };

  EBinType type{PDIGEST};

  inline uint32_t bin() const override {
    return type;
  }
};

struct TSpatial : public dimesion_t<coordinates_t> {
  std::string index_lat;
  std::string index_lon;

  uint32_t bins{1};

  inline uint32_t bin() const override {
    return bins;
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
  std::locale format;

  inline uint32_t bin() const override {
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

struct TCategorical : dimesion_t<uint16_t> {
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

  inline uint32_t bin() const override {
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

  inline bool invalid_data(const uint16_t &value) override {
    temporary = value;
    if (temporary < 0 || temporary >= bin()) {
      return true;
    } else {
      return false;
    }
  }
};

using TDimension = boost::variant<TSpatial, TCategorical, TTemporal, TPayload>;

struct TSchema {
  std::string input;
  std::string input_dir;

  std::string output;
  std::string output_dir;

  std::vector<TDimension> dimensions;
};
