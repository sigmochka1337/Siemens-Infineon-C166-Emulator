#include "debugger/debugger_cli.h"

#include <exception>
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
    } else if (command == "quiet" || command == "q") {
      return false;
    } else {
      out_ << "commands: step (s), run (r), break (b) <addr>, dedlete (d) <adder>, quiet (q)\n";
    }
  } catch (const std::exception& e) {
    out_ << "error: " << e.what() << "\n";
  }
  return true;
}
}  // namespace c166