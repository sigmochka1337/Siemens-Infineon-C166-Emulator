#pragma once
#include <array>

#include "types.h"

namespace c166 {
enum class TrapClass { kSoftware, kA, kB };

struct InstructionSequence {
  Mnemonic prefix = Mnemonic::kNop;
  Word address = 0;
  unsigned remaining = 0;

  bool ExtendedRegisters() const {
    return prefix == Mnemonic::kExtr || prefix == Mnemonic::kExtpr || prefix == Mnemonic::kExtsr;
  }
};

// State shared by successive instructions, cleared along with the CPU on reset.
struct CpuState {
  InstructionSequence sequence;
  std::array<std::optional<Word>, 4> previous_pages{};
  std::vector<TrapClass> trap_frames;
};
}  // namespace c166
