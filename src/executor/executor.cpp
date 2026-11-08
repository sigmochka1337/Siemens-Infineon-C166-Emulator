#include "executor/executor.h"

#include "exceptions.h"
#include "operands.h"
#include "types.h"

namespace c166 {
namespace {

class Helper {
 public:
  Helper(const Instruction& instruction, IMachine& machine)
      : instruction_(instruction), machine_(machine) {}

  void ValidateOperandsSize(size_t size) {
    if (instruction_.operands.size() != size) {
      throw InvalidInstruction(instruction_);
    }
  }

  Operand* operator[](size_t index) { return instruction_.operands[index].get(); }

  void AdvancePC() {
    CodeAddress pc = machine_.GetPC();
    pc.offset = static_cast<Word>(pc.offset + instruction_.size);
    machine_.SetPC(pc);
  }

 private:
  const Instruction& instruction_;
  IMachine& machine_;
};
}  // namespace

void Executor::Execute(const Instruction& instruction) {
  Helper helper(instruction, machine_);
  helper.AdvancePC();

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

    case Mnemonic::kPush: {
      helper.ValidateOperandsSize(1);

      if (stack_.Push(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kPop: {
      helper.ValidateOperandsSize(1);

      if (stack_.Pop(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCallA: {
      helper.ValidateOperandsSize(2);

      if (stack_.Calla(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCallI: {
      helper.ValidateOperandsSize(2);

      if (stack_.Calli(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCallR: {
      helper.ValidateOperandsSize(1);

      if (stack_.Callr(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCallS: {
      helper.ValidateOperandsSize(2);

      if (stack_.Calls(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kPCall: {
      helper.ValidateOperandsSize(2);

      if (stack_.Pcall(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJmpA: {
      helper.ValidateOperandsSize(2);

      if (move_.Jmpa(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJmpI: {
      helper.ValidateOperandsSize(2);

      if (move_.Jmpi(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJmpR: {
      helper.ValidateOperandsSize(2);

      if (move_.Jmpr(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJmpS: {
      helper.ValidateOperandsSize(2);

      if (move_.Jmps(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kRet: {
      helper.ValidateOperandsSize(0);

      if (stack_.Ret()) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kRetS: {
      helper.ValidateOperandsSize(0);

      if (stack_.Rets()) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kRetP: {
      helper.ValidateOperandsSize(1);

      if (stack_.Retp(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kReti: {
      helper.ValidateOperandsSize(0);

      if (stack_.Reti()) break;
      throw InvalidInstruction(instruction);
    }

    default:
      throw UnknownInstruction(instruction);
  }
}
}  // namespace c166
