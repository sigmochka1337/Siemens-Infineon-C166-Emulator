#include "helper.h"

namespace c166::test::extr {

void TestInstructionRange(Checks& checks);

}  // namespace c166::test::extr

int main() {
  c166::test::Checks checks;
  c166::test::extr::TestInstructionRange(checks);
  return checks.Result();
}
