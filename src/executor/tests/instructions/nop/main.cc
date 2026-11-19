#include "helper.h"

namespace c166::test::nop {

void TestNoOperands(Checks& checks);

}  // namespace c166::test::nop

int main() {
  c166::test::Checks checks;
  c166::test::nop::TestNoOperands(checks);
  return checks.Result();
}
