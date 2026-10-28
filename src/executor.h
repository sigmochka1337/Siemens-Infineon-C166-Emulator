#include "interfaces/executor.h"
#include "interfaces/memory.h"

namespace c166 {
class Executor : public IExecutor {
 public:
  Executor(IMachine& machine) : machine_(machine) {}

  void Execute(const Instruction& instruction) override;

 private:
  IMachine& machine_;
};
}  // namespace c166
