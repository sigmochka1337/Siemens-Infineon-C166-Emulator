#include "helper.h"

namespace c166::test::addc {

// C166S V1 SubSystem User's Manual v1.6, p. 6-4.
void TestGprToGpr(Checks& checks);
void TestImmediate3ToGpr(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestImmediate16ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::addc

int main() {
  c166::test::Checks checks;
  c166::test::addc::TestGprToGpr(checks);
  c166::test::addc::TestImmediate3ToGpr(checks);
  c166::test::addc::TestIndirectToGpr(checks);
  c166::test::addc::TestIndirectPostIncrementToGpr(checks);
  c166::test::addc::TestImmediate16ToRegister(checks);
  c166::test::addc::TestDirectToRegister(checks);
  c166::test::addc::TestRegisterToDirect(checks);
  return checks.Result();
}
