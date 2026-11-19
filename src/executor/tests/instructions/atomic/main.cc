#include "helper.h"

namespace c166::test::atomic {

void TestInstructionRange(Checks& checks);

}  // namespace c166::test::atomic

int main() {
  c166::test::Checks checks;
  c166::test::atomic::TestInstructionRange(checks);
  return checks.Result();
}
