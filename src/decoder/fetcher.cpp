#include "decoder/fetcher.h"

namespace c166 {
Result<RawInstruction> Fetcher::Fetch(CodeAddress pc, IMemoryBus& memory,
                                      const IDecoder& decoder) const {
  const auto physical = ToPhysicalAddress(pc);
  if ((pc.offset & 1u) != 0) {
    return Result<RawInstruction>::Failure({FaultCode::kTrap, physical,
                                            TrapKind::kIllegalInstructionAddress,
                                            "instruction address must be word aligned"});
  }
  const auto first = memory.Read16(physical, AccessKind::kCode);
  if (!first.Ok()) {
    return Result<RawInstruction>::Failure(first.Error());
  }
  RawInstruction raw;
  raw.pc = pc;
  raw.bytes[0] = static_cast<Byte>(first.Value() & 0xFF);
  raw.bytes[1] = static_cast<Byte>(first.Value() >> 8);
  const auto length = decoder.InstructionLength({raw.bytes[0], raw.bytes[1]});
  if (!length.Ok()) {
    auto error = length.Error();
    error.address = physical;
    return Result<RawInstruction>::Failure(std::move(error));
  }
  if (length.Value() != 2 && length.Value() != 4) {
    return Result<RawInstruction>::Failure({FaultCode::kInvalidArgument, physical, std::nullopt,
                                            "decoder returned an invalid C166 instruction length"});
  }
  raw.size = length.Value();
  if (raw.size == 4) {
    if (pc.offset > 0xFFFC) {
      return Result<RawInstruction>::Failure(
          {FaultCode::kSegmentBoundary, physical, std::nullopt,
           "sequential fetch across a code segment is unsupported"});
    }
    const auto second = memory.Read16(physical + 2, AccessKind::kCode);
    if (!second.Ok()) {
      return Result<RawInstruction>::Failure(second.Error());
    }
    raw.bytes[2] = static_cast<Byte>(second.Value() & 0xFF);
    raw.bytes[3] = static_cast<Byte>(second.Value() >> 8);
  }
  return Result<RawInstruction>::Success(raw);
}
}  // namespace c166
