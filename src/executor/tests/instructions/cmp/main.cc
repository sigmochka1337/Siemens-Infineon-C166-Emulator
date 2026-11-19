#include "helper.h"

namespace c166::test::cmp {

void TestGprToGpr(Checks& checks);
void TestImmediate3ToGpr(Checks& checks);
void TestImmediate16ToRegister(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestDirectToRegister(Checks& checks);

}  // namespace c166::test::cmp

int main() {
  c166::test::Checks checks;
  c166::test::cmp::TestGprToGpr(checks);
  c166::test::cmp::TestImmediate3ToGpr(checks);
  c166::test::cmp::TestImmediate16ToRegister(checks);
  c166::test::cmp::TestIndirectToGpr(checks);
  c166::test::cmp::TestIndirectPostIncrementToGpr(checks);
  c166::test::cmp::TestDirectToRegister(checks);
  return checks.Result();
}
