#include "helper.h"

namespace c166::test::cmpb {

void TestGprToGpr(Checks& checks);
void TestImmediate3ToGpr(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestDirectToRegister(Checks& checks);

}  // namespace c166::test::cmpb

int main() {
  c166::test::Checks checks;
  c166::test::cmpb::TestGprToGpr(checks);
  c166::test::cmpb::TestImmediate3ToGpr(checks);
  c166::test::cmpb::TestImmediate8ToRegister(checks);
  c166::test::cmpb::TestIndirectToGpr(checks);
  c166::test::cmpb::TestIndirectPostIncrementToGpr(checks);
  c166::test::cmpb::TestDirectToRegister(checks);
  return checks.Result();
}
