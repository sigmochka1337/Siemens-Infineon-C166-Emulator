#include "helper.h"

namespace c166::test::mov {

void TestRegisterSource(Checks& checks);
void TestImmediate4ToGpr(Checks& checks);
void TestImmediate16ToRegister(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestIndirectWithDisplacementToGpr(Checks& checks);
void TestGprToIndirect(Checks& checks);
void TestGprToIndirectPreDecrement(Checks& checks);
void TestGprToIndirectWithDisplacement(Checks& checks);
void TestIndirectToIndirect(Checks& checks);
void TestIndirectToIndirectPostIncrement(Checks& checks);
void TestIndirectPostIncrementToIndirect(Checks& checks);
void TestDirectToIndirect(Checks& checks);
void TestIndirectToDirect(Checks& checks);
void TestRegisterToDirect(Checks& checks);
void TestDirectToRegister(Checks& checks);

}  // namespace c166::test::mov

int main() {
  c166::test::Checks checks;
  c166::test::mov::TestRegisterSource(checks);
  c166::test::mov::TestImmediate4ToGpr(checks);
  c166::test::mov::TestImmediate16ToRegister(checks);
  c166::test::mov::TestIndirectToGpr(checks);
  c166::test::mov::TestIndirectPostIncrementToGpr(checks);
  c166::test::mov::TestIndirectWithDisplacementToGpr(checks);
  c166::test::mov::TestGprToIndirect(checks);
  c166::test::mov::TestGprToIndirectPreDecrement(checks);
  c166::test::mov::TestGprToIndirectWithDisplacement(checks);
  c166::test::mov::TestIndirectToIndirect(checks);
  c166::test::mov::TestIndirectToIndirectPostIncrement(checks);
  c166::test::mov::TestIndirectPostIncrementToIndirect(checks);
  c166::test::mov::TestDirectToIndirect(checks);
  c166::test::mov::TestIndirectToDirect(checks);
  c166::test::mov::TestRegisterToDirect(checks);
  c166::test::mov::TestDirectToRegister(checks);
  return checks.Result();
}
