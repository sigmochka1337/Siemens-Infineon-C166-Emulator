#include "executor/executor.h"

#include "exceptions.h"
#include "operands.h"
#include "types.h"

namespace c166 {
namespace {

class Helper {
 public:
  Helper(const Instruction& instruction) : instruction_(instruction) {}

  void ValidateOperandsSize(size_t size) {
    if (instruction_.operands.size() != size) {
      throw InvalidInstruction(instruction_);
    }
  }

  Operand* operator[](size_t index) { return instruction_.operands[index].get(); }

 private:
  const Instruction& instruction_;
};
}  // namespace

void Executor::Execute(const Instruction& instruction) {
  Helper helper(instruction);
  switch (instruction.mnemonic) {
    case Mnemonic::kMov: {
      helper.ValidateOperandsSize(2);

      if (move_.Mov(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAdd: {
      helper.ValidateOperandsSize(2);

      if (alu_.Add(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAddB: {
      helper.ValidateOperandsSize(2);

      if (alu_.Addb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAddC: {
      helper.ValidateOperandsSize(2);

      if (alu_.Addc(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAddCB: {
      helper.ValidateOperandsSize(2);

      if (alu_.Addcb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kSub: {
      helper.ValidateOperandsSize(2);

      if (alu_.Sub(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kSubB: {
      helper.ValidateOperandsSize(2);

      if (alu_.Subb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kSubC: {
      helper.ValidateOperandsSize(2);

      if (alu_.Subc(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kSubCB: {
      helper.ValidateOperandsSize(2);

      if (alu_.Subcb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kNeg: {
      helper.ValidateOperandsSize(1);

      if (alu_.Neg(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    default:
      throw UnknownInstruction(instruction);
  }

  auto pc = machine_.GetPC();
  pc.offset = (pc.offset + instruction.size) & 0xFFFF;
  machine_.SetPC(pc);
}
}  // namespace c166
