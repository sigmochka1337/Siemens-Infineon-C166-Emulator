#pragma once

#include "src/includes/cpu_state.h"

namespace c166 {

// Executor делегирует сюда арифметику и обновление PSW.
class IAlu {
 public:
  virtual ~IAlu() = default;
  virtual std::uint16_t Add(std::uint16_t left, std::uint16_t right,
                            Psw& psw) = 0;
  virtual std::uint8_t AddByte(std::uint8_t left, std::uint8_t right,
                               Psw& psw) = 0;
  virtual std::uint16_t AddWithCarry(std::uint16_t left, std::uint16_t right,
                                     Psw& psw) = 0;
  virtual std::uint8_t AddByteWithCarry(std::uint8_t left, std::uint8_t right,
                                        Psw& psw) = 0;
  virtual std::uint16_t Sub(std::uint16_t left, std::uint16_t right,
                            Psw& psw) = 0;
  virtual std::uint8_t SubByte(std::uint8_t left, std::uint8_t right,
                               Psw& psw) = 0;
  virtual std::uint16_t SubWithCarry(std::uint16_t left, std::uint16_t right,
                                     Psw& psw) = 0;
  virtual std::uint8_t SubByteWithCarry(std::uint8_t left, std::uint8_t right,
                                        Psw& psw) = 0;
  virtual void Compare(std::uint16_t left, std::uint16_t right, Psw& psw) = 0;
  virtual void CompareByte(std::uint8_t left, std::uint8_t right, Psw& psw) = 0;
  virtual std::uint16_t Negate(std::uint16_t value, Psw& psw) = 0;
  virtual std::uint8_t NegateByte(std::uint8_t value, Psw& psw) = 0;
  virtual std::uint32_t Multiply(std::uint16_t left, std::uint16_t right,
                                 Psw& psw) = 0;
  virtual std::uint32_t MultiplyUnsigned(std::uint16_t left,
                                         std::uint16_t right, Psw& psw) = 0;
  virtual std::uint32_t Divide(std::uint32_t dividend, std::uint16_t divisor,
                               Psw& psw) = 0;
  virtual std::uint32_t DivideUnsigned(std::uint32_t dividend,
                                       std::uint16_t divisor, Psw& psw) = 0;
  virtual std::uint32_t DivideLong(std::uint32_t dividend,
                                   std::uint16_t divisor, Psw& psw) = 0;
  virtual std::uint32_t DivideLongUnsigned(std::uint32_t dividend,
                                           std::uint16_t divisor, Psw& psw) = 0;
};

}  // namespace c166
