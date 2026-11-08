#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <string>

#include "fetcher.h"
#include "flat_machine.h"
#include "loader.h"

namespace {

using namespace c166;

constexpr PhysicalAddress kUnsetAddress = kNoAddress;

void PrintUsage(const char* program) {
  std::cout << std::format(
      "c166 — дизассемблер/выборка инструкций C166 classic\n"
      "\n"
      "Использование:\n"
      "  {} -f ФАЙЛ [--base АДРЕС] [--start АДРЕС] [--count N]\n"
      "\n"
      "  -f, --file   Intel HEX (.H86/.hex) или сырой бинарник\n"
      "  --base       базовый адрес для сырого бинарника (по умолчанию 0)\n"
      "  --start      физический адрес первой инструкции (CSP:IP)\n"
      "  --count      сколько инструкций разобрать (0 — все)\n"
      "\n"
      "Примеры:\n"
      "  {} -f Hello.H86 --start 0xC00000\n"
      "  {} -f sample.bin --base 0x100\n",
      program, program, program);
}

bool ParseAddress(const char* text, PhysicalAddress& value) {
  if (text == nullptr || *text == '\0') {
    return false;
  }
  char* end = nullptr;
  const unsigned long parsed = std::strtoul(text, &end, 0);
  if (end == text || *end != '\0' || parsed > kMaxPhysicalAddress) {
    return false;
  }
  value = static_cast<PhysicalAddress>(parsed);
  return true;
}

bool ParseCount(const char* text, std::size_t& value) {
  if (text == nullptr || *text == '\0') {
    return false;
  }
  char* end = nullptr;
  const unsigned long parsed = std::strtoul(text, &end, 0);
  if (end == text || *end != '\0') {
    return false;
  }
  value = static_cast<std::size_t>(parsed);
  return true;
}

CodeAddress FromPhysical(PhysicalAddress address) {
  return CodeAddress{static_cast<Byte>((address >> 16) & 0xFF),
                     static_cast<Word>(address & 0xFFFF)};
}

std::string FormatAddress(PhysicalAddress address) {
  return std::format("0x{:06X}", address);
}

}  // namespace

int main(int argc, char** argv) {
  std::string path;
  PhysicalAddress base = 0;
  PhysicalAddress start = kUnsetAddress;
  std::size_t max_count = 0;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      PrintUsage(argv[0]);
      return 0;
    }
    if (arg == "-f" || arg == "--file") {
      if (i + 1 >= argc) {
        std::cerr << "error: после -f нужен путь к файлу\n";
        return 2;
      }
      path = argv[++i];
      continue;
    }
    if (arg == "--base") {
      if (i + 1 >= argc || !ParseAddress(argv[++i], base)) {
        std::cerr << "error: некорректный --base\n";
        return 2;
      }
      continue;
    }
    if (arg == "--start") {
      if (i + 1 >= argc || !ParseAddress(argv[++i], start)) {
        std::cerr << "error: некорректный --start\n";
        return 2;
      }
      continue;
    }
    if (arg == "--count") {
      if (i + 1 >= argc || !ParseCount(argv[++i], max_count)) {
        std::cerr << "error: некорректный --count\n";
        return 2;
      }
      continue;
    }
    std::cerr << std::format("error: неизвестный аргумент '{}'\n", arg);
    return 2;
  }

  if (path.empty()) {
    PrintUsage(argv[0]);
    return 2;
  }

  FlatMachine machine;
  LoadResult loaded;
  std::string error;
  if (!LoadFile(path, base, machine, loaded, error)) {
    std::cerr << std::format("error: {}\n", error);
    return 2;
  }

  std::cout << std::format("file:   {}\n", path);
  std::cout << std::format("format: {}\n", FileFormatName(loaded.format));
  std::cout << std::format("bytes:  {}\n", loaded.bytes);
  for (const LoadedRange& range : loaded.ranges) {
    std::cout << std::format("range:  {}..{}\n", FormatAddress(range.begin),
                             FormatAddress(range.end - 1));
  }
  if (loaded.hasEntry) {
    std::cout << std::format("entry:  {}\n", FormatAddress(loaded.entry));
  }

  PhysicalAddress address = start;
  if (address == kUnsetAddress) {
    address = loaded.hasEntry ? loaded.entry : loaded.MinAddress();
  }

  Fetcher fetcher(machine);
  std::size_t decoded = 0;
  while (address != kNoAddress) {
    address = loaded.NextMapped(address);
    if (address == kNoAddress) {
      break;
    }
    if (max_count != 0 && decoded >= max_count) {
      break;
    }

    try {
      const Instruction instruction = fetcher.Fetch(FromPhysical(address));
      std::cout << std::format("{}  {}\n", FormatAddress(address),
                               instruction.ToString());
      if (instruction.size == 0) {
        std::cerr << "error: декодер вернул инструкцию нулевой длины\n";
        return 1;
      }
      const std::uint64_t next =
          static_cast<std::uint64_t>(address) + instruction.size;
      if (next > kMaxPhysicalAddress) {
        break;
      }
      address = static_cast<PhysicalAddress>(next);
      ++decoded;
    } catch (const std::exception& exception) {
      std::cerr << std::format("{}: error: {}\n", FormatAddress(address),
                               exception.what());
      return 1;
    }
  }

  std::cout << std::format("decoded {} instruction(s)\n", decoded);
  return 0;
}
