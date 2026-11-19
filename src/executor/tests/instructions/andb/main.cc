#include "helper.h"

namespace c166::test::andb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-7.
void TestByteGprToByteGpr(Checks& checks);
void TestImmediate3ToByteGpr(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestIndirectToByteGpr(Checks& checks);
void TestIndirectPostIncrementToByteGpr(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::andb

int main() {
  c166::test::Checks checks;
  c166::test::andb::TestByteGprToByteGpr(checks);
  c166::test::andb::TestImmediate3ToByteGpr(checks);
  c166::test::andb::TestImmediate8ToRegister(checks);
  c166::test::andb::TestIndirectToByteGpr(checks);
  c166::test::andb::TestIndirectPostIncrementToByteGpr(checks);
  c166::test::andb::TestDirectToRegister(checks);
  c166::test::andb::TestRegisterToDirect(checks);
  return checks.Result();
}
