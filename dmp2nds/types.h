//
// Created by cicerolp on 1/8/18.
//

#pragma once

namespace string_util {

std::vector<std::string> split(const std::string &input, const std::regex &regex) {
  static const std::sregex_token_iterator last;
  return std::vector<std::string>{std::sregex_token_iterator{input.begin(), input.end(), regex, -1}, last};
}

std::vector<std::string> split(const std::string &input, const std::string &str_regex) {
  static const std::sregex_token_iterator last;
  std::regex regex(str_regex);
  return std::vector<std::string>{std::sregex_token_iterator{input.begin(), input.end(), regex, -1}, last};
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
  float lat{0.f}, lon{0.f};
};

template<typename T>
struct dimesion_t {
  std::string name;
  uint32_t index;
  uint32_t size;

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
  uint32_t index_lat;
  uint32_t index_lon;

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
  bool unix_time{false};
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

    return 0;
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

using TDimension = std::variant<TSpatial, TCategorical, TTemporal, TPayload>;

struct TSchema {
  std::string input;
  std::string input_dir;

  std::string output;
  std::string output_dir;

  uint32_t lines_to_skip{32};

  std::vector<TDimension> dimensions;
};
