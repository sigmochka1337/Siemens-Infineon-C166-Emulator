#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

#include "executor/executor.h"

namespace {
using namespace c166;

void Check(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

template <typename T>
Result<T> UnusedAccess() {
  return Result<T>::Failure(
      {FaultCode::kUnmapped, std::nullopt, std::nullopt, "Unused fake access"});
}

// This fake isolates instruction semantics from the real memory implementation.
class FakeMachine final : public IMachine {
 public:
  std::array<Word, 256> registers{};
  CodeAddress pc{};
  ExtensionState extension{};
  size_t mutations = 0;
  std::optional<Diagnostic> psw_read_error;

  Result<Byte> Read8(PhysicalAddress, AccessKind) override { return UnusedAccess<Byte>(); }
  Result<Word> Read16(PhysicalAddress, AccessKind) override { return UnusedAccess<Word>(); }
  Result<void> Write8(PhysicalAddress, Byte, AccessKind) override { return UnusedAccess<void>(); }
  Result<void> Write16(PhysicalAddress, Word, AccessKind) override { return UnusedAccess<void>(); }
  Result<Byte> Peek8(PhysicalAddress) const override { return UnusedAccess<Byte>(); }
  Result<Word> ReadRegister(RegisterId id) override {
    if (psw_read_error.has_value() && id == RegisterId::kPSW) {
      return Result<Word>::Failure(*psw_read_error);
    }
    return Result<Word>::Success(registers[static_cast<Byte>(id)]);
  }
  Result<void> WriteRegister(RegisterId id, Word value) override {
    registers[static_cast<Byte>(id)] = value;
    ++mutations;
    return Result<void>::Success();
  }
  CodeAddress GetPC() const override { return pc; }
  Result<void> SetPC(CodeAddress value) override {
    pc = value;
    ++mutations;
    return Result<void>::Success();
  }
  ExtensionState GetExtension() const override { return extension; }
  void SetExtension(ExtensionState value) override {
    extension = value;
    ++mutations;
  }
  void Reset() override {
    registers.fill(0);
    pc = {};
    extension = {};
    mutations = 0;
    psw_read_error.reset();
  }
  CpuSnapshot Snapshot() const override { return {}; }
  Result<void> LoadBinary(PhysicalAddress, std::span<const Byte>) override {
    return UnusedAccess<void>();
  }
  Result<PhysicalAddress> ResolveDataAddress(LogicalAddress) const override {
    return UnusedAccess<PhysicalAddress>();
  }
  Result<PhysicalAddress> ResolveShortRegister(Byte, Width) const override {
    return UnusedAccess<PhysicalAddress>();
  }
  Result<BitLocation> ResolveBitAddress(BitAddress) const override {
    return UnusedAccess<BitLocation>();
  }

  Word& Reg(RegisterId id) { return registers[static_cast<Byte>(id)]; }
};

Instruction Nop(CodeAddress pc = {}) {
  Instruction instruction;
  instruction.opcode = Opcode::kNop;
  instruction.pc = pc;
  instruction.length = 2;
  return instruction;
}

Instruction MoveImmediate(Word value, CodeAddress pc = {}) {
  Instruction instruction = Nop(pc);
  instruction.opcode = Opcode::kMov;
  instruction.length = 4;
  instruction.operand_count = 2;
  instruction.operands[0] = {RegisterOperand{RegisterId::kR0}, Width::kWord};
  instruction.operands[1] = {ImmediateOperand{value}, Width::kWord};
  return instruction;
}

Instruction RegisterBinary(Opcode opcode, CodeAddress pc = {}) {
  Instruction instruction = Nop(pc);
  instruction.opcode = opcode;
  instruction.operand_count = 2;
  instruction.operands[0] = {RegisterOperand{RegisterId::kR0}, Width::kWord};
  instruction.operands[1] = {RegisterOperand{RegisterId::kR1}, Width::kWord};
  return instruction;
}

Instruction Jump(Condition condition, int8_t words, CodeAddress pc = {}) {
  Instruction instruction = Nop(pc);
  instruction.opcode = Opcode::kJmpR;
  instruction.condition = condition;
  instruction.operand_count = 1;
  instruction.operands[0] = {RelativeOperand{words}, Width::kWord};
  return instruction;
}

void TestMove(const Executor& executor, FakeMachine& machine) {
  machine.Reset();
  machine.Reg(RegisterId::kPSW) = 0xA040 | psw::kC | psw::kV | psw::kZ;
  Check(executor.Execute(MoveImmediate(0x8000), machine).status == StepStatus::kCompleted,
        "MOV status");
  Check(machine.Reg(RegisterId::kR0) == 0x8000, "MOV result");
  Check(machine.Reg(RegisterId::kPSW) == (0xA040 | psw::kC | psw::kV | psw::kE | psw::kN),
        "MOV changes E/Z/N and preserves C/V and other bits");
  Check(machine.pc == CodeAddress{0, 4}, "MOV advances by four bytes");
  Check(executor.Execute(MoveImmediate(0, machine.pc), machine).status == StepStatus::kCompleted,
        "MOV zero status");
  Check(machine.Reg(RegisterId::kPSW) == (0xA040 | psw::kC | psw::kV | psw::kZ), "MOV zero flags");
  machine.Reg(RegisterId::kR1) = 0xFFFF;
  Check(executor.Execute(RegisterBinary(Opcode::kMov, machine.pc), machine).status ==
            StepStatus::kCompleted,
        "Register MOV status");
  Check(machine.Reg(RegisterId::kR0) == 0xFFFF && machine.Reg(RegisterId::kR1) == 0xFFFF,
        "Register MOV copies without changing source");
  Check((machine.Reg(RegisterId::kPSW) & psw::kConditionMask) == (psw::kC | psw::kV | psw::kN),
        "MOV negative non-sentinel flags");
}

void TestAdd(const Executor& executor, FakeMachine& machine) {
  struct Case {
    Word left;
    Word right;
    Word expected;
    Word flags;
  };
  const std::array<Case, 6> cases{{
      {0xFFFF, 1, 0, psw::kZ | psw::kC},
      {0x7FFF, 1, 0x8000, psw::kN | psw::kV},
      {0x8000, 0x8000, 0, psw::kE | psw::kZ | psw::kV | psw::kC},
      {0x8000, 0, 0x8000, psw::kN},
      {1, 0x8000, 0x8001, psw::kE | psw::kN},
      {0xFFFE, 1, 0xFFFF, psw::kN},
  }};
  for (const auto& item : cases) {
    machine.Reset();
    machine.Reg(RegisterId::kR0) = item.left;
    machine.Reg(RegisterId::kR1) = item.right;
    machine.Reg(RegisterId::kPSW) = 0xA040 | psw::kConditionMask;
    Check(executor.Execute(RegisterBinary(Opcode::kAdd), machine).status == StepStatus::kCompleted,
          "ADD status");
    Check(machine.Reg(RegisterId::kR0) == item.expected, "ADD arithmetic result");
    Check(machine.Reg(RegisterId::kPSW) == (0xA040 | item.flags), "ADD flag boundary case");
    Check(machine.pc == CodeAddress{0, 2}, "ADD advances once");
  }
}

void TestConditions(const Executor& executor, FakeMachine& machine) {
  // Truth vectors are ordered by the encoded condition nibble 0..15.
  struct Case {
    Word flags;
    std::array<bool, 16> taken;
  };
  const std::array<Case, 5> cases{{
      {0,
       {true, true, false, true, false, true, false, true, false, true, true, false, false, true,
        true, false}},
      {psw::kConditionMask,
       {true, false, true, false, true, false, true, false, true, false, false, true, false, true,
        false, true}},
      {psw::kN,
       {true, true, false, true, false, true, true, false, false, true, false, true, true, false,
        true, false}},
      {psw::kV,
       {true, true, false, true, true, false, false, true, false, true, false, true, true, false,
        true, false}},
      {psw::kZ,
       {true, false, true, false, false, true, false, true, false, true, false, true, false, true,
        false, true}},
  }};
  for (const auto& item : cases) {
    for (Byte condition = 0; condition < 16; ++condition) {
      machine.Reset();
      machine.pc = {7, 0x100};
      machine.Reg(RegisterId::kPSW) = item.flags;
      const auto result =
          executor.Execute(Jump(static_cast<Condition>(condition), 2, machine.pc), machine);
      Check(result.status == StepStatus::kCompleted, "JMPR condition status");
      Check(machine.pc == CodeAddress{7, static_cast<Word>(item.taken[condition] ? 0x106 : 0x102)},
            "JMPR encoded condition");
      Check(result.flow == (item.taken[condition] ? FlowKind::kBranch : FlowKind::kSequential),
            "JMPR reports actual control flow");
      Check(machine.Reg(RegisterId::kPSW) == item.flags, "JMPR preserves PSW");
    }
  }
}

void TestRelativeAndBoundaries(const Executor& executor, FakeMachine& machine) {
  struct Case {
    Word start;
    int8_t displacement;
    Word target;
  };
  const std::array<Case, 4> cases{
      {{0x100, 127, 0x200}, {0x200, -128, 0x102}, {0x100, -1, 0x100}, {0xFFFE, -1, 0xFFFE}}};
  for (const auto& item : cases) {
    machine.Reset();
    machine.pc = {0x42, item.start};
    const auto result =
        executor.Execute(Jump(Condition::kUnconditional, item.displacement, machine.pc), machine);
    Check(result.status == StepStatus::kCompleted, "Relative displacement status");
    Check(machine.pc == CodeAddress{0x42, item.target}, "Relative signed word displacement");
  }
  const std::array<Instruction, 4> failures{{
      Nop({2, 0xFFFE}),
      MoveImmediate(1, {2, 0xFFFC}),
      MoveImmediate(1, {2, 0xFFFE}),
      Jump(Condition::kUnconditional, -128, {2, 0}),
  }};
  for (const auto& instruction : failures) {
    machine.Reset();
    machine.pc = instruction.pc;
    const auto result = executor.Execute(instruction, machine);
    Check(result.status == StepStatus::kFaulted && result.diagnostic.has_value() &&
              result.diagnostic->code == FaultCode::kSegmentBoundary,
          "Segment diagnostic");
    Check(machine.mutations == 0 && machine.pc == instruction.pc,
          "Boundary failure before mutation");
  }
}

void TestUnsupported(const Executor& executor, FakeMachine& machine) {
  std::array<Instruction, 8> instructions{};
  instructions[0] = RegisterBinary(Opcode::kSub);
  instructions[1] = RegisterBinary(Opcode::kMov);
  instructions[1].operands[0].width = Width::kByte;
  instructions[2] = RegisterBinary(Opcode::kMov);
  instructions[2].operands[0].value = RegisterOperand{RegisterId::kPSW};
  instructions[3] = MoveImmediate(1);
  instructions[3].opcode = Opcode::kAdd;
  instructions[4] = Nop();
  instructions[4].operand_count = 255;
  instructions[5] = Nop();
  instructions[5].length = 6;
  instructions[6] = Nop();
  instructions[6].pc.offset = 2;
  instructions[7] = Jump(static_cast<Condition>(0x10), 1);
  for (const auto& instruction : instructions) {
    machine.Reset();
    machine.extension = {ExtensionMode::kPage, 0x123, 2, true};
    const auto result = executor.Execute(instruction, machine);
    Check(result.status == StepStatus::kFaulted && result.diagnostic.has_value(),
          "Rejected form diagnostic");
    Check(machine.mutations == 0, "Unsupported or malformed input must not mutate machine");
    Check(machine.extension.remaining == 2, "Rejected form must not consume EXT range");
  }
}

void TestExtensionAndReadFault(const Executor& executor, FakeMachine& machine) {
  machine.Reset();
  machine.extension = {ExtensionMode::kPage, 0x123, 2, true};
  machine.Reg(RegisterId::kPSW) = 0xA05F;
  Check(executor.Execute(Nop(), machine).status == StepStatus::kCompleted, "NOP status");
  Check(machine.extension.remaining == 1 && machine.extension.extended_sfr &&
            machine.extension.value == 0x123,
        "One executed instruction consumes one EXT slot");
  Check(machine.Reg(RegisterId::kPSW) == 0xA05F, "NOP preserves PSW");
  Check(executor.Execute(Jump(Condition::kUnconditional, -1, machine.pc), machine).status ==
            StepStatus::kCompleted,
        "Branch in EXT status");
  Check(machine.extension.remaining == 0 && machine.extension.mode == ExtensionMode::kNone &&
            !machine.extension.extended_sfr,
        "EXT clears only after its final executed instruction");

  machine.Reset();
  machine.psw_read_error = Diagnostic{FaultCode::kUnmapped, 0xFF10, std::nullopt, "Read fault"};
  const auto result = executor.Execute(MoveImmediate(1), machine);
  Check(result.status == StepStatus::kFaulted && result.diagnostic->code == FaultCode::kUnmapped,
        "Read fault propagation");
  Check(machine.mutations == 0, "Failed operand acquisition must not retire the instruction");

  machine.psw_read_error =
      Diagnostic{FaultCode::kMisaligned, 0xFF11, TrapKind::kIllegalWordAccess, "Word access trap"};
  const auto trap = executor.Execute(MoveImmediate(1), machine);
  Check(trap.status == StepStatus::kTrap && trap.diagnostic.has_value() &&
            trap.diagnostic->trap == TrapKind::kIllegalWordAccess &&
            trap.diagnostic->address == 0xFF11,
        "Architectural trap subtype and address must survive bus propagation");
  Check(machine.mutations == 0, "Operand trap must not retire the instruction");
}
}  // namespace

int main() {
  try {
    const Executor executor;
    FakeMachine machine;
    TestMove(executor, machine);
    TestAdd(executor, machine);
    TestConditions(executor, machine);
    TestRelativeAndBoundaries(executor, machine);
    TestUnsupported(executor, machine);
    TestExtensionAndReadFault(executor, machine);
    std::cout << "executor tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
