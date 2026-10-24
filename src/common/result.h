#pragma once

#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "common/types.h"

namespace c166 {
enum class FaultCode : uint8_t {
  kUnmapped,
  kOutOfRange,
  kReadOnly,
  kMisaligned,
  kNotExecutable,
  kDeviceError,
  kInvalidEncoding,
  kUnsupportedInstruction,
  kTruncatedInstruction,
  kSegmentBoundary,
  kInvalidArgument,
  kTrap,
  kStopped,
};
enum class TrapKind : uint8_t {
  kIllegalInstruction,
  kIllegalOperand,
  kIllegalWordAccess,
  kIllegalInstructionAddress,
  kStackOverflow,
  kStackUnderflow,
  kSoftware,
};
struct Diagnostic {
  FaultCode code = FaultCode::kInvalidArgument;
  std::optional<PhysicalAddress> address;
  std::optional<TrapKind> trap;
  std::string message;
};

inline bool IsTrap(const Diagnostic& diagnostic) {
  return diagnostic.code == FaultCode::kTrap || diagnostic.trap.has_value();
}

// Exactly one of value or error exists. Value()/Error() require checking Ok().
template <typename T>
class Result {
 public:
  static Result Success(T value) { return Result(std::move(value)); }
  static Result Failure(Diagnostic error) { return Result(std::move(error)); }
  bool Ok() const { return std::holds_alternative<T>(data_); }
  const T& Value() const { return std::get<T>(data_); }
  T& Value() { return std::get<T>(data_); }
  const Diagnostic& Error() const { return std::get<Diagnostic>(data_); }

 private:
  explicit Result(T value) : data_(std::move(value)) {}
  explicit Result(Diagnostic error) : data_(std::move(error)) {}
  std::variant<T, Diagnostic> data_;
};
template <>
class Result<void> {
 public:
  static Result Success() { return Result(std::nullopt); }
  static Result Failure(Diagnostic error) { return Result(std::move(error)); }
  bool Ok() const { return !error_.has_value(); }
  const Diagnostic& Error() const { return error_.value(); }

 private:
  explicit Result(std::optional<Diagnostic> error) : error_(std::move(error)) {}
  std::optional<Diagnostic> error_;
};
}  // namespace c166
