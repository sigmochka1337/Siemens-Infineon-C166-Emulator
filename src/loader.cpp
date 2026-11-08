#include "loader.h"

#include <algorithm>
#include <cstdint>
#include <format>
#include <fstream>
#include <vector>

#include "exceptions.h"

namespace c166 {
namespace {

int HexDigit(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

bool ParseHex(std::string_view text, std::vector<Byte>& out) {
  out.clear();
  if (text.empty() || text.size() % 2 != 0) {
    return false;
  }
  out.reserve(text.size() / 2);
  for (std::size_t i = 0; i < text.size(); i += 2) {
    const int high = HexDigit(text[i]);
    const int low = HexDigit(text[i + 1]);
    if (high < 0 || low < 0) {
      return false;
    }
    out.push_back(static_cast<Byte>((high << 4) | low));
  }
  return true;
}

void MergeRanges(LoadResult& result) {
  if (result.ranges.empty()) {
    result.bytes = 0;
    return;
  }

  std::sort(result.ranges.begin(), result.ranges.end(),
            [](const LoadedRange& a, const LoadedRange& b) {
              return a.begin < b.begin;
            });

  std::vector<LoadedRange> merged;
  merged.reserve(result.ranges.size());
  for (const LoadedRange& range : result.ranges) {
    if (range.begin >= range.end) {
      continue;
    }
    if (!merged.empty() && range.begin <= merged.back().end) {
      merged.back().end = std::max(merged.back().end, range.end);
    } else {
      merged.push_back(range);
    }
  }

  result.ranges = std::move(merged);
  result.bytes = 0;
  for (const LoadedRange& range : result.ranges) {
    result.bytes += static_cast<std::size_t>(range.end - range.begin);
  }
}

bool WriteRange(IMachine& machine, PhysicalAddress address, const Byte* data,
                std::size_t size, std::string& error) {
  for (std::size_t i = 0; i < size; ++i) {
    const auto current = static_cast<std::uint64_t>(address) + i;
    if (current > kMaxPhysicalAddress) {
      error = "адрес записи выходит за 24-битное пространство C166";
      return false;
    }
    try {
      machine.Write8(static_cast<PhysicalAddress>(current), data[i]);
    } catch (const PhysicalAddressOutOfRange&) {
      error = std::format("адрес 0x{:06X} не помещается в память", current);
      return false;
    }
  }
  return true;
}

}  // namespace

const char* FileFormatName(FileFormat format) {
  switch (format) {
    case FileFormat::kUnknown:
      return "unknown";
    case FileFormat::kIntelHex:
      return "intel-hex";
    case FileFormat::kRawBinary:
      return "raw";
    case FileFormat::kOmf166:
      return "omf166";
  }
  return "unknown";
}

PhysicalAddress LoadResult::MinAddress() const {
  return ranges.empty() ? 0 : ranges.front().begin;
}

PhysicalAddress LoadResult::MaxAddress() const {
  return ranges.empty() ? 0 : ranges.back().end;
}

bool LoadResult::Contains(PhysicalAddress address) const {
  for (const LoadedRange& range : ranges) {
    if (address >= range.begin && address < range.end) {
      return true;
    }
  }
  return false;
}

PhysicalAddress LoadResult::NextMapped(PhysicalAddress address) const {
  for (const LoadedRange& range : ranges) {
    if (address < range.begin) {
      return range.begin;
    }
    if (address < range.end) {
      return address;
    }
  }
  return kNoAddress;
}

FileFormat DetectFormat(const std::vector<Byte>& data) {
  if (data.empty()) {
    return FileFormat::kUnknown;
  }

  std::size_t start = 0;
  if (data.size() >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) {
    start = 3;  // UTF-8 BOM перед текстовым HEX
  }
  while (start < data.size() &&
         (data[start] == ' ' || data[start] == '\t' || data[start] == '\r' ||
          data[start] == '\n')) {
    ++start;
  }
  if (start < data.size() && data[start] == ':') {
    return FileFormat::kIntelHex;
  }

  // Absolute OMF166 от L166 начинается с record type 0x70, а в первых
  // байтах лежит имя модуля/командная строка компилятора ("C166 ...").
  if (data.size() >= 8 && data[0] == 0x70) {
    const std::size_t limit = std::min<std::size_t>(data.size(), 64);
    for (std::size_t i = 0; i + 4 <= limit; ++i) {
      if (data[i] == 'C' && data[i + 1] == '1' && data[i + 2] == '6' &&
          data[i + 3] == '6') {
        return FileFormat::kOmf166;
      }
    }
  }

  return FileFormat::kRawBinary;
}

bool LoadIntelHex(std::string_view text, IMachine& machine, LoadResult& result,
                  std::string& error) {
  result = LoadResult{};
  result.format = FileFormat::kIntelHex;
  error.clear();

  std::uint32_t base = 0;
  bool saw_eof = false;
  std::size_t line_number = 0;
  std::size_t position = 0;

  while (position <= text.size()) {
    std::size_t end = text.find('\n', position);
    if (end == std::string_view::npos) {
      end = text.size();
    }

    std::string_view line = text.substr(position, end - position);
    if (!line.empty() && line.back() == '\r') {
      line.remove_suffix(1);
    }
    while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) {
      line.remove_prefix(1);
    }
    while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) {
      line.remove_suffix(1);
    }

    ++line_number;
    if (line.empty()) {
      if (end == text.size()) {
        break;
      }
      position = end + 1;
      continue;
    }

    if (line.front() != ':') {
      error = std::format("строка {}: запись Intel HEX должна начинаться с ':'",
                          line_number);
      return false;
    }

    std::vector<Byte> record;
    if (!ParseHex(line.substr(1), record)) {
      error = std::format("строка {}: некорректные hex-цифры", line_number);
      return false;
    }
    if (record.size() < 5) {
      error = std::format("строка {}: слишком короткая запись", line_number);
      return false;
    }

    const Byte length = record[0];
    if (record.size() != static_cast<std::size_t>(length) + 5) {
      error = std::format("строка {}: длина записи не совпадает с полем length",
                          line_number);
      return false;
    }

    Byte checksum = 0;
    for (Byte value : record) {
      checksum = static_cast<Byte>(checksum + value);
    }
    if (checksum != 0) {
      error = std::format("строка {}: неверная контрольная сумма", line_number);
      return false;
    }

    const Word offset = static_cast<Word>((record[1] << 8) | record[2]);
    const Byte type = record[3];
    const Byte* payload = record.data() + 4;

    switch (type) {
      case 0x00: {
        const std::uint64_t absolute =
            static_cast<std::uint64_t>(base) + offset;
        if (absolute + length > static_cast<std::uint64_t>(kMaxPhysicalAddress) + 1) {
          error = std::format("строка {}: адрес выходит за 24-битное пространство",
                              line_number);
          return false;
        }
        if (!WriteRange(machine, static_cast<PhysicalAddress>(absolute), payload,
                        length, error)) {
          error = std::format("строка {}: {}", line_number, error);
          return false;
        }
        if (length != 0) {
          result.ranges.push_back(
              LoadedRange{static_cast<PhysicalAddress>(absolute),
                          static_cast<PhysicalAddress>(absolute + length)});
        }
        break;
      }

      case 0x01:
        saw_eof = true;
        break;

      case 0x02:
        if (length != 2) {
          error = std::format("строка {}: запись 02 должна иметь длину 2",
                              line_number);
          return false;
        }
        base = (static_cast<std::uint32_t>(payload[0] << 8) | payload[1]) << 4;
        break;

      case 0x03:
        if (length != 4) {
          error = std::format("строка {}: запись 03 должна иметь длину 4",
                              line_number);
          return false;
        }
        result.entry =
            (((static_cast<PhysicalAddress>(payload[0]) << 8) | payload[1]) << 4) +
            ((static_cast<PhysicalAddress>(payload[2]) << 8) | payload[3]);
        result.hasEntry = true;
        break;

      case 0x04:
        if (length != 2) {
          error = std::format("строка {}: запись 04 должна иметь длину 2",
                              line_number);
          return false;
        }
        base = (static_cast<std::uint32_t>(payload[0] << 8) | payload[1]) << 16;
        break;

      case 0x05:
        if (length != 4) {
          error = std::format("строка {}: запись 05 должна иметь длину 4",
                              line_number);
          return false;
        }
        result.entry = (static_cast<PhysicalAddress>(payload[0]) << 24) |
                       (static_cast<PhysicalAddress>(payload[1]) << 16) |
                       (static_cast<PhysicalAddress>(payload[2]) << 8) | payload[3];
        result.hasEntry = true;
        break;

      default:
        error = std::format("строка {}: неподдерживаемый тип записи 0x{:02X}",
                            line_number, type);
        return false;
    }

    if (saw_eof) {
      break;
    }
    if (end == text.size()) {
      break;
    }
    position = end + 1;
  }

  if (!saw_eof) {
    error = "в Intel HEX нет записи EOF (:00000001FF)";
    return false;
  }
  if (result.ranges.empty()) {
    error = "в Intel HEX нет ни одного байта данных";
    return false;
  }

  MergeRanges(result);
  return true;
}

bool LoadBinary(const std::vector<Byte>& data, PhysicalAddress base, IMachine& machine,
                LoadResult& result, std::string& error) {
  result = LoadResult{};
  result.format = FileFormat::kRawBinary;
  error.clear();

  if (data.empty()) {
    error = "пустой бинарный файл";
    return false;
  }
  if (base > kMaxPhysicalAddress) {
    error = "базовый адрес выходит за 24-битное пространство C166";
    return false;
  }
  const std::uint64_t end = static_cast<std::uint64_t>(base) + data.size();
  if (end > static_cast<std::uint64_t>(kMaxPhysicalAddress) + 1) {
    error = "бинарник не помещается в 24-битное пространство C166";
    return false;
  }

  if (!WriteRange(machine, base, data.data(), data.size(), error)) {
    return false;
  }

  result.ranges.push_back(
      LoadedRange{base, static_cast<PhysicalAddress>(end)});
  result.bytes = data.size();
  return true;
}

bool LoadFile(const std::string& path, PhysicalAddress raw_base, IMachine& machine,
              LoadResult& result, std::string& error) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    error = std::format("не удалось открыть файл '{}'", path);
    return false;
  }

  std::vector<Byte> data;
  char byte = 0;
  while (file.get(byte)) {
    data.push_back(static_cast<Byte>(static_cast<unsigned char>(byte)));
  }
  if (!file.eof()) {
    error = std::format("ошибка чтения файла '{}'", path);
    return false;
  }

  const FileFormat format = DetectFormat(data);
  switch (format) {
    case FileFormat::kIntelHex: {
      const std::string_view text(reinterpret_cast<const char*>(data.data()),
                                  data.size());
      return LoadIntelHex(text, machine, result, error);
    }
    case FileFormat::kRawBinary:
      return LoadBinary(data, raw_base, machine, result, error);
    case FileFormat::kOmf166:
      error = std::format(
          "'{}' — absolute OMF166 (Keil), а не raw binary. Сконвертируй его в "
          "Intel HEX: скопируй в файл с расширением .abs, затем выполни "
          "'OH166 <файл>.abs H167' и загрузи получившийся .H86.",
          path);
      return false;
    case FileFormat::kUnknown:
      error = std::format("'{}' пустой или неизвестный формат", path);
      return false;
  }

  error = "неизвестный формат входного файла";
  return false;
}

}  // namespace c166
