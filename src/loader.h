#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "interfaces/memory.h"
#include "types.h"

namespace c166 {

enum class FileFormat : std::uint8_t {
  kUnknown,
  kIntelHex,
  kRawBinary,
  kOmf166,
};

const char* FileFormatName(FileFormat format);

struct LoadedRange {
  PhysicalAddress begin = 0;
  PhysicalAddress end = 0;  // не включительно
};

struct LoadResult {
  FileFormat format = FileFormat::kUnknown;
  std::vector<LoadedRange> ranges;
  std::size_t bytes = 0;
  PhysicalAddress entry = 0;
  bool hasEntry = false;

  PhysicalAddress MinAddress() const;
  PhysicalAddress MaxAddress() const;  // не включительно
  bool Contains(PhysicalAddress address) const;
  PhysicalAddress NextMapped(PhysicalAddress address) const;
};

inline constexpr PhysicalAddress kNoAddress = 0xFFFFFFFFu;

// Определяет формат по содержимому, а не по расширению: Keil может отдать
// файл без расширения (например, absolute OMF166 `Hello`).
FileFormat DetectFormat(const std::vector<Byte>& data);

// Intel HEX-86/HEX-386: записи 00/01/02/03/04/05.
bool LoadIntelHex(std::string_view text, IMachine& machine, LoadResult& result,
                  std::string& error);

// Сырой бинарник: все байты подряд начиная с base.
bool LoadBinary(const std::vector<Byte>& data, PhysicalAddress base, IMachine& machine,
                LoadResult& result, std::string& error);

// Читает файл, определяет формат и загружает его в machine.
bool LoadFile(const std::string& path, PhysicalAddress raw_base, IMachine& machine,
              LoadResult& result, std::string& error);

}  // namespace c166
