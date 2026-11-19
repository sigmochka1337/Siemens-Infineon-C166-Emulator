#include "helper.h"

namespace c166::test::xorb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-99.
void TestByteGprToByteGpr(Checks& checks);
void TestImmediate3ToByteGpr(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestIndirectToByteGpr(Checks& checks);
void TestIndirectPostIncrementToByteGpr(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::xorb

int main() {
  c166::test::Checks checks;
  c166::test::xorb::TestByteGprToByteGpr(checks);
  c166::test::xorb::TestImmediate3ToByteGpr(checks);
  c166::test::xorb::TestImmediate8ToRegister(checks);
  c166::test::xorb::TestIndirectToByteGpr(checks);
  c166::test::xorb::TestIndirectPostIncrementToByteGpr(checks);
  c166::test::xorb::TestDirectToRegister(checks);
  c166::test::xorb::TestRegisterToDirect(checks);
  return checks.Result();
}
