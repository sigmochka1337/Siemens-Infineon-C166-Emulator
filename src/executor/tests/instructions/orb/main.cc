#include "helper.h"

namespace c166::test::orb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-70.
void TestByteGprToByteGpr(Checks& checks);
void TestImmediate3ToByteGpr(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestIndirectToByteGpr(Checks& checks);
void TestIndirectPostIncrementToByteGpr(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::orb

int main() {
  c166::test::Checks checks;
  c166::test::orb::TestByteGprToByteGpr(checks);
  c166::test::orb::TestImmediate3ToByteGpr(checks);
  c166::test::orb::TestImmediate8ToRegister(checks);
  c166::test::orb::TestIndirectToByteGpr(checks);
  c166::test::orb::TestIndirectPostIncrementToByteGpr(checks);
  c166::test::orb::TestDirectToRegister(checks);
  c166::test::orb::TestRegisterToDirect(checks);
  return checks.Result();
}
