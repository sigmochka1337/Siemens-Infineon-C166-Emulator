#include "helper.h"

namespace c166::test::or_word {

// C166S V1 SubSystem User's Manual v1.6, p. 6-69.
void TestGprToGpr(Checks& checks);
void TestImmediate3ToGpr(Checks& checks);
void TestImmediate16ToRegister(Checks& checks);
void TestIndirectToGpr(Checks& checks);
void TestIndirectPostIncrementToGpr(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::or_word

int main() {
  c166::test::Checks checks;
  c166::test::or_word::TestGprToGpr(checks);
  c166::test::or_word::TestImmediate3ToGpr(checks);
  c166::test::or_word::TestImmediate16ToRegister(checks);
  c166::test::or_word::TestIndirectToGpr(checks);
  c166::test::or_word::TestIndirectPostIncrementToGpr(checks);
  c166::test::or_word::TestDirectToRegister(checks);
  c166::test::or_word::TestRegisterToDirect(checks);
  return checks.Result();
}
