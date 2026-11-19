#include "helper.h"

namespace c166::test::subcb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-95.
void TestByteGprToByteGpr(Checks& checks);
void TestImmediate3ToByteGpr(Checks& checks);
void TestIndirectToByteGpr(Checks& checks);
void TestIndirectPostIncrementToByteGpr(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::subcb

int main() {
  c166::test::Checks checks;
  c166::test::subcb::TestByteGprToByteGpr(checks);
  c166::test::subcb::TestImmediate3ToByteGpr(checks);
  c166::test::subcb::TestIndirectToByteGpr(checks);
  c166::test::subcb::TestIndirectPostIncrementToByteGpr(checks);
  c166::test::subcb::TestImmediate8ToRegister(checks);
  c166::test::subcb::TestDirectToRegister(checks);
  c166::test::subcb::TestRegisterToDirect(checks);
  return checks.Result();
}
