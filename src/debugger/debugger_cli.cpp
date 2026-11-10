#include "debugger/debugger_cli.h"

#include <exception>
#include <format>
#include <iostream>
#include <sstream>
namespace c166 {
DebuggerCli::DebuggerCli(IDebugger& debugger, std::istream& in, std::ostream& out)
    : debugger_(debugger), in_(in), out_(out) {}
void DebuggerCli::Loop() {
  std::string line;
  while (true) {
    out_ << "(c166) " << std::flush;
    if (!std::getline(in_, line)) {
      break;
    }
    if (!Handle(line)) {
      break;
    }
  }
}
bool DebuggerCli::Handle(const std::string& line) {
  std::istringstream input(line);
  std::string command;
  input >> command;
  if (command.empty()) {
    return true;
  }
  try {
    if (command == "step" || command == "s") {
      debugger_.Step();
      out_ << "ok\n";
    } else if (command == "run" || command == "r") {
      debugger_.Run();
      out_ << "stopped\n";
    } else if (command == "break" || command == "b") {
      PhysicalAddress address = 0;
      if (input >> std::hex >> address) {
        debugger_.AddBreakpoint(address);
        out_ << "breakpoint added\n";
      } else {
        out_ << "usage: break <hex address>\n";
      }
    } else if (command == "delete" || command == "d") {
      PhysicalAddress address = 0;
      if (input >> std::hex >> address) {
        debugger_.RemoveBreakpoint(address);
        out_ << "breakpoint removed\n";
      } else {
        out_ << "usage: delete <hex address>\n";
      }
    } else if (command == "quit" || command == "q") {
      return false;
    } else if (command == "show" || command == "i") {
      out_ << debugger_.NextInstructionToString() << "\n";
    } else if (command == "trace" || command == "t") {
      PrintState();
    } else if (command == "xp") {
      PhysicalAddress address = 0;
      if (input >> std::hex >> address) {
        Word value = debugger_.PhysicalRead16(address);
        out_ << std::format("phys[0x{:06X}] = 0x{:04X}\n", address, value);
      } else {
        out_ << "usage: xp <hex address>\n";
      }
    } else if (command == "xl") {
      LogicalAddress address = 0;
      if (input >> std::hex >> address) {
        Word value = debugger_.LogicalRead16(address);
        out_ << std::format("log[0x{:04X}] = 0x{:04X}\n", address, value);
      } else {
        out_ << "usage: xl <hex address>\n";
      }
    } else if (command == "wp") {
      PhysicalAddress address = 0;
      Word value = 0;
      if (input >> std::hex >> address >> value) {
        debugger_.PhysicalWrite16(address, value);
        out_ << "written\n";
      } else {
        out_ << "usage: wp <hex address> <hex value>\n";
      }
    } else if (command == "wl") {
      LogicalAddress address = 0;
      Word value = 0;
      if (input >> std::hex >> address >> value) {
        debugger_.LogicalWrite16(address, value);
        out_ << "written\n";
      } else {
        out_ << "usage: wl <hex address> <hex value>\n";
      }
    } else {
      out_ << "commands: step (s), run (r), break (b) <addr>, delete (d) <addr>, show (i), trace "
              "(t),\n"
              "          xp <addr>, xl <addr>, wp <addr> <val>, wl <addr> <val>, quit (q)\n";
    }
  } catch (const std::exception& e) {
    out_ << "error: " << e.what() << "\n";
  }
  return true;
}
void DebuggerCli::PrintState() const {
  Snapshot snapshot = debugger_.Trace();
  out_ << std::format("PSW = 0x{:04X}\n", snapshot.psw);
  out_ << std::format("N={} C={} V={} Z={} E={}\n", (snapshot.psw & psw::kN) != 0 ? 1 : 0,
                      (snapshot.psw & psw::kC) != 0 ? 1 : 0, (snapshot.psw & psw::kV) != 0 ? 1 : 0,
                      (snapshot.psw & psw::kZ) != 0 ? 1 : 0, (snapshot.psw & psw::kE) != 0 ? 1 : 0);
  Word sp = 0;
  Word stkun = 0;
  for (const auto& [id, value] : snapshot.registers) {
    out_ << std::format("{} = 0x{:04X}\n", RegisterIdToString(id), value);
    if (id == RegisterId::kSP) {
      sp = value;
    }
    if (id == RegisterId::kSTKUN) {
      stkun = value;
    }
  }
  int stack_words = stkun >= sp ? (stkun - sp) / 2 : 0;
  out_ << std::format("stack size: {} words\n", stack_words);
}
}  // namespace c166