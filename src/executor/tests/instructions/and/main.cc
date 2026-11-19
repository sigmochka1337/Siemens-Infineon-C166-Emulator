#include "helper.h"

namespace c166::test::and_word {

// C166S V1 SubSystem User's Manual v1.6, p. 6-6.
void TestGprToGpr(Checks& checks);
void TestImmediate3ToGpr(Checks& checks);
void TestImmediate16ToRegister(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::and_word

int main() {
  c166::test::Checks checks;
  c166::test::and_word::TestGprToGpr(checks);
  c166::test::and_word::TestImmediate3ToGpr(checks);
  c166::test::and_word::TestImmediate16ToRegister(checks);
  c166::test::and_word::TestIndirectToGpr(checks);
  c166::test::and_word::TestIndirectPostIncrementToGpr(checks);
  c166::test::and_word::TestDirectToRegister(checks);
  c166::test::and_word::TestRegisterToDirect(checks);
  return checks.Result();
}
