#include "helper.h"

namespace c166::test::sub {

// C166S V1 SubSystem User's Manual v1.6, p. 6-92.
void TestGprToGpr(Checks& checks);
void TestImmediate3ToGpr(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestImmediate16ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::sub

int main() {
  c166::test::Checks checks;
  c166::test::sub::TestGprToGpr(checks);
  c166::test::sub::TestImmediate3ToGpr(checks);
  c166::test::sub::TestIndirectToGpr(checks);
  c166::test::sub::TestIndirectPostIncrementToGpr(checks);
  c166::test::sub::TestImmediate16ToRegister(checks);
  c166::test::sub::TestDirectToRegister(checks);
  c166::test::sub::TestRegisterToDirect(checks);
  return checks.Result();
}
