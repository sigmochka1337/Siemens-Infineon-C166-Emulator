#include "helper.h"

namespace c166::test::movb {

// C166S V1 SubSystem User's Manual v1.6, pp. 6-60 and 6-61.
void TestImmediate4ToByteGpr(Checks& checks);
void TestByteGprToByteGpr(Checks& checks);
void TestIndirectWithDisplacementToByteGpr(Checks& checks);
void TestIndirectPostIncrementToByteGpr(Checks& checks);
void TestIndirectToByteGpr(Checks& checks);
void TestByteGprToIndirectPreDecrement(Checks& checks);
void TestByteGprToIndirectWithDisplacement(Checks& checks);
void TestByteGprToIndirect(Checks& checks);
void TestIndirectToIndirectPostIncrement(Checks& checks);
void TestIndirectPostIncrementToIndirect(Checks& checks);
void TestIndirectToIndirect(Checks& checks);
void TestDirectToIndirect(Checks& checks);
void TestIndirectToDirect(Checks& checks);
void TestRegisterToDirect(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);

}  // namespace c166::test::movb

int main() {
  c166::test::Checks checks;
  c166::test::movb::TestImmediate4ToByteGpr(checks);
  c166::test::movb::TestByteGprToByteGpr(checks);
  c166::test::movb::TestIndirectWithDisplacementToByteGpr(checks);
  c166::test::movb::TestIndirectPostIncrementToByteGpr(checks);
  c166::test::movb::TestIndirectToByteGpr(checks);
  c166::test::movb::TestByteGprToIndirectPreDecrement(checks);
  c166::test::movb::TestByteGprToIndirectWithDisplacement(checks);
  c166::test::movb::TestByteGprToIndirect(checks);
  c166::test::movb::TestIndirectToIndirectPostIncrement(checks);
  c166::test::movb::TestIndirectPostIncrementToIndirect(checks);
  c166::test::movb::TestIndirectToIndirect(checks);
  c166::test::movb::TestDirectToIndirect(checks);
  c166::test::movb::TestIndirectToDirect(checks);
  c166::test::movb::TestRegisterToDirect(checks);
  c166::test::movb::TestImmediate8ToRegister(checks);
  c166::test::movb::TestDirectToRegister(checks);
  return checks.Result();
}
