#pragma once
#include <exception>
#include <format>

#include "types.h"
namespace c166 {
class PhysicalAddressOutOfRange : public std::exception {
 public:
  explicit PhysicalAddressOutOfRange(PhysicalAddress address)
      : address_(address),
        msg_(std::format("Адрес `{:#08X}` не доступен. Доступные адреса: 0x000000 - 0xFFFFFF",
                         address_)) {}

  const char* what() const noexcept override { return msg_.c_str(); }

  PhysicalAddress GetAddress() const { return address_; }

 private:
  PhysicalAddress address_;
  std::string msg_;
};

class LogicalAddressOutOfRange : public std::exception {
 public:
  explicit LogicalAddressOutOfRange(LogicalAddress address)
      : address_(address),
        msg_(std::format("Адрес `{:#06X}` не доступен. Доступные адреса: 0x0000 - 0xFFFF",
                         address_)) {}

  const char* what() const noexcept override { return msg_.c_str(); }

  LogicalAddress GetAddress() const { return address_; }

 private:
  LogicalAddress address_;
  std::string msg_;
};

class AddressIsReadOnly : public std::exception {
 public:
  explicit AddressIsReadOnly(LogicalAddress address)
      : address_(address),
        msg_(std::format("Адрес `{:#06X}` доступен только для чтения", address_)) {}

  const char* what() const noexcept override { return msg_.c_str(); }

  LogicalAddress GetAddress() const { return address_; }

 private:
  LogicalAddress address_;
  std::string msg_;
};

class UnreadableInstruction : public std::exception {
 public:
  explicit UnreadableInstruction(CodeAddress address, std::array<Byte, 4> bytes)
      : address_(address),
        bytes_(bytes),
        msg_(std::format("Не удалось определить инструкцию по адресу {:#08X}. Контекст: {:#04X} "
                         "{:#04X} {:#04X} {:#04X}",
                         address.ToPhysicalAddress(), bytes[0], bytes[1], bytes[2], bytes[3])) {}

  const char* what() const noexcept override { return msg_.c_str(); }

  CodeAddress GetAddress() const { return address_; }

 private:
  CodeAddress address_;
  std::array<Byte, 4> bytes_;
  std::string msg_;
};

class UnknownInstruction : public std::exception {
 public:
  explicit UnknownInstruction(const Instruction& instruction)
      : instruction_(instruction),
        msg_(std::format("Неизвестная инструкция: {}", instruction.ToString())) {}

  const char* what() const noexcept override { return msg_.c_str(); }

  const Instruction& GetInstruction() const { return instruction_; }

 private:
  const Instruction& instruction_;
  std::string msg_;
};

class InvalidInstruction : public std::exception {
 public:
  explicit InvalidInstruction(const Instruction& instruction)
      : instruction_(instruction),
        msg_(std::format("Инструкция не может быть исполнена: {}", instruction.ToString())) {}

  const char* what() const noexcept override { return msg_.c_str(); }

  const Instruction& GetInstruction() const { return instruction_; }

 private:
  const Instruction& instruction_;
  std::string msg_;
};

class InvalidBreakpointAddress : public std::exception {
 public:
  explicit InvalidBreakpointAddress(PhysicalAddress address)
      : address_(address),
        msg_(std::format("Невозможно установить breakpoint по адресу `{:#08X}`", address_)) {}

  const char* what() const noexcept override { return msg_.c_str(); }

  PhysicalAddress GetAddress() const { return address_; }

 private:
  PhysicalAddress address_;
  std::string msg_;
};

class BreakpointDoesNotExist : public std::exception {
 public:
  explicit BreakpointDoesNotExist(PhysicalAddress address)
      : address_(address),
        msg_(std::format("Не существует breakpoint по адресу `{:#08X}`", address_)) {}

  const char* what() const noexcept override { return msg_.c_str(); }

  PhysicalAddress GetAddress() const { return address_; }

 private:
  PhysicalAddress address_;
  std::string msg_;
};
}  // namespace c166
