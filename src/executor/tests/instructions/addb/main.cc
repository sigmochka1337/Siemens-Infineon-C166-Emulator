#include "helper.h"

namespace c166::test::addb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-3.
void TestByteGprToByteGpr(Checks& checks);
void TestImmediate3ToByteGpr(Checks& checks);
void TestIndirectToByteGpr(Checks& checks);
void TestIndirectPostIncrementToByteGpr(Checks& checks);
void TestImmediate8ToRegister(Checks& checks);
void TestDirectToRegister(Checks& checks);
void TestRegisterToDirect(Checks& checks);

}  // namespace c166::test::addb

int main() {
  c166::test::Checks checks;
  c166::test::addb::TestByteGprToByteGpr(checks);
  c166::test::addb::TestImmediate3ToByteGpr(checks);
  c166::test::addb::TestIndirectToByteGpr(checks);
  c166::test::addb::TestIndirectPostIncrementToByteGpr(checks);
  c166::test::addb::TestImmediate8ToRegister(checks);
  c166::test::addb::TestDirectToRegister(checks);
  c166::test::addb::TestRegisterToDirect(checks);
  return checks.Result();
}
