#include "debugger/debugger_cli.h"

#include <exception>
#include <format>
#include <fstream>
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
    if (!line.empty() && line != "history" && line != "h") {
      history_.push_back(line);
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
        if (trace_instructions_) {
          out_ << debugger_.NextInstructionToString() << "\n";
        }
        debugger_.Step();
      }
      out_ << "ok\n";
    } else if (command == "history" || command == "h") {
      for (size_t i = 0; i < history_.size(); i++) {
        out_ << std::format("{:3}: {}\n", i, history_[i]);
      }
    } else if (command == "si") {
      debugger_.StepIn();
      out_ << "ok\n";
    } else if (command == "so") {
      debugger_.StepOut();
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
      PrintState(out_);
    } else if (command == "dump") {
      std::string filename;
      if (input >> filename) {
        std::ofstream file(filename);
        if (file.is_open()) {
          PrintState(file);
          out_ << "trace dumped to " << filename << "\n";
        } else {
          out_ << "error: could not open file\n";
        }
      } else {
        out_ << "usage: dump <filename>\n";
      }
    } else if (command == "set") {
      std::string reg_str;
      Word val;
      if ((input >> reg_str >> std::hex >> val)) {
        RegisterId id = RegisterId::kR0;
        bool found = false;
        for (int i = 0; i <= static_cast<int>(RegisterId::kTFR); ++i) {
          if (RegisterIdToString(static_cast<RegisterId>(i)) == reg_str) {
            id = static_cast<RegisterId>(i);
            found = true;
            break;
          }
        }
        if (found) {
          debugger_.SetRegister(id, val);
          out_ << "register set\n";
        } else {
          out_ << "error: unknown register\n";
        }
      } else {
        out_ << "usage: set <reg_name> <hex_value>\n";
      }
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
      out_ << "commands:\n"
              "step (s) [count]      - execute instructions\n"
              "si / so               - step in / step out\n"
              "run (r)               - run until breakpoint\n"
              "break (b) [addr]      - add/list breakpoints\n"
              "delete (d) <index>    - remove breakpoint\n"
              "show (i)              - show next instruction\n"
              "trace (t)             - dump registers to console\n"
              "dump <file>           - save registers to file\n"
              "set <reg> <val>       - modify register\n"
              "xp <addr>             - read physical memory\n"
              "xl <addr>             - read logical memory\n"
              "wp <addr> <val>       - write physical memory\n"
              "wl <addr> <val>       - write logical memory\n"
              "history (h)           - show command history\n"
              "quit (q)              - exit debugger\n";
    }
  } catch (const std::exception& e) {
    out_ << "error: " << e.what() << "\n";
  }
  return true;
}
void DebuggerCli::PrintState(std::ostream& out) const {
  Snapshot snapshot = debugger_.Trace();
  out << std::format("PC  = 0x{:06X}\n", snapshot.pc.ToPhysicalAddress());
  out << std::format("PSW = 0x{:04X}\n", snapshot.psw);
  out << std::format("N={} C={} V={} Z={} E={}\n", (snapshot.psw & psw::kN) != 0 ? 1 : 0,
                     (snapshot.psw & psw::kC) != 0 ? 1 : 0, (snapshot.psw & psw::kV) != 0 ? 1 : 0,
                     (snapshot.psw & psw::kZ) != 0 ? 1 : 0, (snapshot.psw & psw::kE) != 0 ? 1 : 0);
  Word sp = 0;
  Word stkun = 0;
  for (const auto& [id, value] : snapshot.registers) {
    out << std::format("{} = 0x{:04X}\n", RegisterIdToString(id), value);
    if (id == RegisterId::kSP) {
      sp = value;
    }
    if (id == RegisterId::kSTKUN) {
      stkun = value;
    }
  }
  int stack_words = stkun >= sp ? (stkun - sp) / 2 : 0;
  out << std::format("stack size: {} words\n", stack_words);
}
}  // namespace c166