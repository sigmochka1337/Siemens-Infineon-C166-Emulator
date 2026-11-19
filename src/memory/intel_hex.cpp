#include <algorithm>
#include <charconv>
#include <cstdint>
#include <format>
#include <fstream>
#include <spanstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "memory/machine.h"

namespace c166 {
namespace {

struct DataBlock {
  PhysicalAddress address;
  std::vector<Byte> bytes;
};

[[noreturn]] void InvalidRecord(std::size_t line, std::string_view message) {
  throw std::invalid_argument(std::format("Intel HEX line {}: {}", line, message));
}

std::vector<Byte> DecodeRecord(std::string_view line, std::size_t line_number) {
  constexpr std::size_t kRecordOverhead = 5;
  constexpr std::size_t kMaxRecordBytes = kRecordOverhead + 0xFF;
  if (!line.starts_with(':') || line.size() < 1 + 2 * kRecordOverhead ||
      line.size() > 1 + 2 * kMaxRecordBytes || line.size() % 2 == 0) {
    InvalidRecord(line_number, "invalid record layout");
  }
  line.remove_prefix(1);

  std::vector<Byte> bytes;
  bytes.reserve(line.size() / 2);
  unsigned checksum = 0;
  for (std::size_t offset = 0; offset < line.size(); offset += 2) {
    const char* start = line.data() + offset;
    unsigned value = 0;
    const auto [end, error] = std::from_chars(start, start + 2, value, 16);
    if (error != std::errc{} || end != start + 2) {
      InvalidRecord(line_number, "invalid hexadecimal digit");
    }
    bytes.push_back(static_cast<Byte>(value));
    checksum += value;
  }
  if (bytes.size() != kRecordOverhead + bytes.front()) {
    InvalidRecord(line_number, "byte count does not match record length");
  }
  if (checksum % 256 != 0) {
    InvalidRecord(line_number, "invalid checksum");
  }
  return bytes;
}

void SaveBlock(std::vector<DataBlock>& blocks, std::uint64_t address, std::span<const Byte> bytes,
               std::size_t line_number) {
  if (bytes.empty()) return;
  if (address + bytes.size() > std::uint64_t{kMaxPhysicalAddress} + 1) {
    throw std::out_of_range(
        std::format("Intel HEX line {}: data exceeds physical memory", line_number));
  }
  blocks.push_back({static_cast<PhysicalAddress>(address), {bytes.begin(), bytes.end()}});
}

void LoadHexImage(Machine& machine, std::istream& input) {
  std::vector<DataBlock> blocks;
  PhysicalAddress base = 0;
  bool segmented = false;
  bool eof_record = false;
  std::size_t line_number = 0;
  for (std::string text; std::getline(input, text);) {
    ++line_number;
    std::string_view line(text);
    if (line_number == 1 && line.starts_with("\xEF\xBB\xBF")) line.remove_prefix(3);
    const auto first = line.find_first_not_of(" \t\r");
    if (first == std::string_view::npos) continue;
    line = line.substr(first, line.find_last_not_of(" \t\r") - first + 1);
    if (eof_record) InvalidRecord(line_number, "record after EOF");

    const auto record = DecodeRecord(line, line_number);
    const Word offset = static_cast<Word>((record[1] << 8) | record[2]);
    const Byte type = record[3];
    const auto data = std::span<const Byte>(record).subspan(4, record.front());
    switch (type) {
      case 0x00: {
        // A segmented load wraps the 16-bit offset within the same segment.
        const auto count =
            segmented ? std::min(data.size(), std::size_t{0x10000} - offset) : data.size();
        SaveBlock(blocks, std::uint64_t{base} + offset, data.first(count), line_number);
        SaveBlock(blocks, base, data.subspan(count), line_number);
        break;
      }
      case 0x01:
        if (!data.empty()) InvalidRecord(line_number, "EOF record must have no data");
        eof_record = true;
        break;
      case 0x02:
      case 0x04:
        if (offset != 0 || data.size() != 2) InvalidRecord(line_number, "invalid address record");
        segmented = type == 0x02;
        base = ((PhysicalAddress{data[0]} << 8) | data[1]) << (segmented ? 4 : 16);
        break;
      case 0x03:
      case 0x05:
        // Start address metadata does not affect the memory image.
        if (offset != 0 || data.size() != 4) InvalidRecord(line_number, "invalid start record");
        break;
      default:
        InvalidRecord(line_number, std::format("unsupported record type 0x{:02X}", type));
    }
  }
  if (input.bad() || !input.eof()) throw std::runtime_error("could not read Intel HEX data");
  if (!eof_record) InvalidRecord(line_number + 1, "missing EOF record");

  for (const auto& block : blocks) machine.LoadBinary(block.address, block.bytes);
}

}  // namespace

void Machine::LoadIntelHex(std::string_view text) {
  std::ispanstream input(text);
  LoadHexImage(*this, input);
}

void Machine::LoadIntelHexFile(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) throw std::runtime_error("could not open Intel HEX file: " + path.string());
  LoadHexImage(*this, input);
}

}  // namespace c166
