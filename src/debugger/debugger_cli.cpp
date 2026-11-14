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
      int count = 1;
      input >> count;
      if (count <= 0) {
        count = 1;
      }
      for (int i = 0; i < count; i++) {
        debugger_.Step();
      }
      out_ << "ok\n";
    } else if (command == "run" || command == "r") {
      debugger_.Run();
      out_ << "stopped\n";
    } else if (command == "break" || command == "b") {
      std::string arg;
      if (!(input >> arg)) {
        auto bps = debugger_.GetBreakpoints();
        if (bps.empty()) {
          out_ << "no breakpoints\n";
        } else {
          for (size_t i = 0; i < bps.size(); i++) {
            out_ << std::format("{}: 0x{:06X}\n", i, bps[i]);
          }
        }
      } else {
        try {
          PhysicalAddress address = std::stoul(arg, nullptr, 16);
          debugger_.AddBreakpoint(address);
          out_ << "breakpoint added\n";
        } catch (const std::exception&) {
          out_ << "usage: break <hex address>\n";
        }
      }
    } else if (command == "delete" || command == "d") {
      size_t index = 0;
      if (input >> index) {
        auto bps = debugger_.GetBreakpoints();
        if (index < bps.size()) {
          debugger_.RemoveBreakpoint(bps[index]);
          out_ << "breakpoint removed\n";
        } else {
          out_ << "error: index out of range\n";
        }
      } else {
        out_ << "usage: delete <index>\n";
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
      out_ << "commands: step (s), run (r), break (b) <addr>, delete (d) [addr], show (i), trace "
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