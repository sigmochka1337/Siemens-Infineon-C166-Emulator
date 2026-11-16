#include "helper.h"

namespace c166::test::add {

// C166S V1 SubSystem User's Manual v1.6, p. 6-2.
void TestGprToGpr(Checks& checks);
void TestImmediate3ToGpr(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestImmediate16ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::add

int main() {
  c166::test::Checks checks;
  c166::test::add::TestGprToGpr(checks);
  c166::test::add::TestImmediate3ToGpr(checks);
  c166::test::add::TestIndirectToGpr(checks);
  c166::test::add::TestIndirectPostIncrementToGpr(checks);
  c166::test::add::TestImmediate16ToRegister(checks);
  c166::test::add::TestDirectToRegister(checks);
  c166::test::add::TestRegisterToDirect(checks);
  return checks.Result();
}
