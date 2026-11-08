#include "flat_machine.h"

#include <algorithm>
#include <stdexcept>

#include "exceptions.h"

namespace c166 {
namespace {
constexpr std::size_t kRegisterCount =
    static_cast<std::size_t>(RegisterId::kSTKUN) + 1;
}  // namespace

FlatMachine::FlatMachine(std::size_t memory_size)
    : memory_(memory_size, 0), registers_(kRegisterCount, 0) {
  if (memory_size == 0 || memory_size > kMaxPhysicalAddress + 1) {
    throw std::invalid_argument("некорректный размер памяти FlatMachine");
  }
}

Byte FlatMachine::Read8(PhysicalAddress address) const {
  if (address >= memory_.size()) {
    throw PhysicalAddressOutOfRange(address);
  }
  return memory_[address];
}

void FlatMachine::Write8(PhysicalAddress address, Byte value) {
  if (address >= memory_.size()) {
    throw PhysicalAddressOutOfRange(address);
  }
  memory_[address] = value;
}

Word FlatMachine::Read16(PhysicalAddress address) const {
  const Word low = Read8(address);
  const Word high = Read8(address + 1);
  return static_cast<Word>(low | (high << 8));
}

void FlatMachine::Write16(PhysicalAddress address, Word value) {
  Write8(address, static_cast<Byte>(value & 0xFF));
  Write8(address + 1, static_cast<Byte>(value >> 8));
}

Word FlatMachine::ReadRegister(RegisterId id) const {
  const auto index = static_cast<std::size_t>(id);
  if (index >= registers_.size()) {
    throw std::out_of_range("некорректный RegisterId");
  }
  return registers_[index];
}

void FlatMachine::WriteRegister(RegisterId id, Word value) {
  const auto index = static_cast<std::size_t>(id);
  if (index >= registers_.size()) {
    throw std::out_of_range("некорректный RegisterId");
  }
  registers_[index] = value;
}

CodeAddress FlatMachine::GetPC() const { return pc_; }

void FlatMachine::SetPC(CodeAddress pc) { pc_ = pc; }

void FlatMachine::Reset() {
  for (auto& value : registers_) {
    value = 0;
  }
  pc_ = CodeAddress{};
}

PhysicalAddress FlatMachine::Resolve(LogicalAddress address) { return address; }

void FlatMachine::ClearMemory() { std::fill(memory_.begin(), memory_.end(), 0); }

}  // namespace c166
