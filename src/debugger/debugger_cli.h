#pragma once
#include <iosfwd>
#include <string>
#include <vector>

#include "interfaces/debugger.h"
namespace c166 {
class DebuggerCli {
 public:
  DebuggerCli(IDebugger& debugger, std::istream& in, std::ostream& out);
  void Loop();

 private:
  bool Handle(const std::string& line);
  IDebugger& debugger_;
  std::istream& in_;
  std::ostream& out_;
  void PrintState(std::ostream& out) const;
  std::vector<std::string> history_;
  bool trace_instructions_ = false;
};
}  // namespace c166
