#pragma once
#include "types.h"

namespace c166 {

// mov r0 r1
// r1 - RegisterOperand
class RegisterOperand : public Operand {
 public:
  explicit RegisterOperand(RegisterId id) : id_(id) {}
  RegisterId Id() const { return id_; }

  std::string ToString() const override { return RegisterIdToString(id_); }

 private:
  RegisterId id_;
};

// mov r0 #0x1234
// #0x1234 - ImmediateOperand
class ImmediateOperand : public Operand {
 public:
  explicit ImmediateOperand(Word value) : value_(value) {}
  Word Value() const { return value_; }

  std::string ToString() const override { return std::format("#0x{:04X}", value_); }

 private:
  Word value_;
};

// mov r0 0x1234
// 0x1234 - DirectMemoryOperand
class DirectMemoryOperand : public Operand {
 public:
  explicit DirectMemoryOperand(LogicalAddress address) : address_(address) {}
  LogicalAddress Address() const { return address_; }

  std::string ToString() const override { return std::format("0x{:04X}", address_); }

 private:
  LogicalAddress address_;
};

// mov r0 [r1]
// [r1] - IndirectMemoryOperand
//
// mov r0 [r1+]
// [r1+] - IndirectMemoryOperand
//
// mov r0 [r1+]
// [-r1] - IndirectMemoryOperand
//
// mov r0 [r1+]
// [r1 + #0x1234] - IndirectMemoryOperand
// Only r0 - r15
class IndirectMemoryOperand : public Operand {
 public:
  enum class AddressMode {
    kPlain,          // [r1]
    kPostIncrement,  // [r1+]
    kPreDecrement,   // [-r1]
    kDisplacement,   // [r1 + #0x1234]
  };

  static std::unique_ptr<IndirectMemoryOperand> CreatePlain(RegisterId id) {
    return std::unique_ptr<IndirectMemoryOperand>(
        new IndirectMemoryOperand(id, 0, AddressMode::kPlain));
  }

  static std::unique_ptr<IndirectMemoryOperand> CreatePostIncrement(RegisterId id) {
    return std::unique_ptr<IndirectMemoryOperand>(
        new IndirectMemoryOperand(id, 0, AddressMode::kPostIncrement));
  }

  static std::unique_ptr<IndirectMemoryOperand> CreatePreDecrement(RegisterId id) {
    return std::unique_ptr<IndirectMemoryOperand>(
        new IndirectMemoryOperand(id, 0, AddressMode::kPreDecrement));
  }

  static std::unique_ptr<IndirectMemoryOperand> CreateDisplacement(RegisterId id,
                                                                   Word displacement) {
    return std::unique_ptr<IndirectMemoryOperand>(
        new IndirectMemoryOperand(id, displacement, AddressMode::kDisplacement));
  }

  std::string ToString() const override;

  RegisterId Id() const { return id_; }
  Word Displacement() const { return displacement_; }
  AddressMode Mode() const { return mode_; }

 private:
  IndirectMemoryOperand(RegisterId id, Word displacement, AddressMode mode)
      : id_(id), displacement_(displacement), mode_(mode) {}
  RegisterId id_;
  Word displacement_;
  AddressMode mode_;
};

// caddr — 16-битный адрес внутри текущего 64-КБ сегмента.
class CodeAddressOperand : public Operand {
 public:
  explicit CodeAddressOperand(Word offset) : offset_(offset) {}
  Word Offset() const { return offset_; }

  std::string ToString() const override { return std::format("0x{:04X}", offset_); }

 private:
  Word offset_;
};

// seg,caddr — адрес в другом сегменте (CALLS/JMPS).
class SegmentedAddressOperand : public Operand {
 public:
  SegmentedAddressOperand(Byte segment, Word offset) : segment_(segment), offset_(offset) {}
  Byte Segment() const { return segment_; }
  Word Offset() const { return offset_; }

  std::string ToString() const override {
    return std::format("0x{:02X}:0x{:04X}", segment_, offset_);
  }

 private:
  Byte segment_;
  Word offset_;
};

// bitaddr — слово в bit-addressable памяти и номер бита в нём.
class BitOperand : public Operand {
 public:
  BitOperand(Word word_address, Byte bit) : word_address_(word_address), bit_(bit) {}
  Word WordAddress() const { return word_address_; }
  Byte Bit() const { return bit_; }

  std::string ToString() const override { return std::format("0x{:04X}.{}", word_address_, bit_); }

 private:
  Word word_address_;
  Byte bit_;
};

// rel — знаковое смещение в словах (C166 считает rel в словах).
class RelativeOperand : public Operand {
 public:
  explicit RelativeOperand(int words) : words_(words) {}
  int Words() const { return words_; }

  std::string ToString() const override { return std::format("{:+d}", words_); }

 private:
  int words_;
};
}  // namespace c166
