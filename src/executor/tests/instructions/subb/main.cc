#include "helper.h"

namespace c166::test::subb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-93.
void TestByteGprToByteGpr(Checks& checks);
void TestImmediate3ToByteGpr(Checks& checks);
void TestIndirectToByteGpr(Checks& checks);
void TestIndirectPostIncrementToByteGpr(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::subb

int main() {
  c166::test::Checks checks;
  c166::test::subb::TestByteGprToByteGpr(checks);
  c166::test::subb::TestImmediate3ToByteGpr(checks);
  c166::test::subb::TestIndirectToByteGpr(checks);
  c166::test::subb::TestIndirectPostIncrementToByteGpr(checks);
  c166::test::subb::TestImmediate8ToRegister(checks);
  c166::test::subb::TestDirectToRegister(checks);
  c166::test::subb::TestRegisterToDirect(checks);
  return checks.Result();
}
