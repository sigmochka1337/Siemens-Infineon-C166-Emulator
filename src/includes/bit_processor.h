#pragma once

#include <cstdint>

#include "src/includes/cpu_state.h"

namespace c166 {

class IBitProcessor {
 public:
  virtual ~IBitProcessor() = default;
  virtual bool Read(std::uint32_t address) const = 0;
  virtual void Write(std::uint32_t address, bool value) = 0;
  virtual std::uint16_t And(std::uint16_t left, std::uint16_t right,
                            Psw& psw) = 0;
  virtual std::uint8_t AndByte(std::uint8_t left, std::uint8_t right,
                               Psw& psw) = 0;
  virtual std::uint16_t Or(std::uint16_t left, std::uint16_t right,
                           Psw& psw) = 0;
  virtual std::uint8_t OrByte(std::uint8_t left, std::uint8_t right,
                              Psw& psw) = 0;
  virtual std::uint16_t Xor(std::uint16_t left, std::uint16_t right,
                            Psw& psw) = 0;
  virtual std::uint8_t XorByte(std::uint8_t left, std::uint8_t right,
                               Psw& psw) = 0;
  virtual std::uint16_t Complement(std::uint16_t value, Psw& psw) = 0;
  virtual std::uint8_t ComplementByte(std::uint8_t value, Psw& psw) = 0;
  virtual std::uint16_t ShiftLeft(std::uint16_t value, std::uint8_t count,
                                  Psw& psw) = 0;
  virtual std::uint16_t ShiftRight(std::uint16_t value, std::uint8_t count,
                                   Psw& psw) = 0;
  virtual std::uint16_t ArithmeticShiftRight(std::uint16_t value,
                                             std::uint8_t count, Psw& psw) = 0;
  virtual std::uint16_t RotateLeft(std::uint16_t value, std::uint8_t count,
                                   Psw& psw) = 0;
  virtual std::uint16_t RotateRight(std::uint16_t value, std::uint8_t count,
                                    Psw& psw) = 0;
};

}  // namespace c166
