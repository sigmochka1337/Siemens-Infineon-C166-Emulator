#include "helper.h"

namespace c166::test::subc {

// C166S V1 SubSystem User's Manual v1.6, p. 6-94.
void TestGprToGpr(Checks& checks);
void TestImmediate3ToGpr(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestImmediate16ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::subc

int main() {
  c166::test::Checks checks;
  c166::test::subc::TestGprToGpr(checks);
  c166::test::subc::TestImmediate3ToGpr(checks);
  c166::test::subc::TestIndirectToGpr(checks);
  c166::test::subc::TestIndirectPostIncrementToGpr(checks);
  c166::test::subc::TestImmediate16ToRegister(checks);
  c166::test::subc::TestDirectToRegister(checks);
  c166::test::subc::TestRegisterToDirect(checks);
  return checks.Result();
}
