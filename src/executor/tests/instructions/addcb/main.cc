#include "helper.h"

namespace c166::test::addcb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-5.
void TestByteGprToByteGpr(Checks& checks);
void TestImmediate3ToByteGpr(Checks& checks);
void TestIndirectToByteGpr(Checks& checks);
void TestIndirectPostIncrementToByteGpr(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::addcb

int main() {
  c166::test::Checks checks;
  c166::test::addcb::TestByteGprToByteGpr(checks);
  c166::test::addcb::TestImmediate3ToByteGpr(checks);
  c166::test::addcb::TestIndirectToByteGpr(checks);
  c166::test::addcb::TestIndirectPostIncrementToByteGpr(checks);
  c166::test::addcb::TestImmediate8ToRegister(checks);
  c166::test::addcb::TestDirectToRegister(checks);
  c166::test::addcb::TestRegisterToDirect(checks);
  return checks.Result();
}
