#include "helper.h"

namespace c166::test::movbz {

// C166S V1 SubSystem User's Manual v1.6, p. 6-63.
void TestByteGprToWordGpr(Checks& checks);
void TestRegisterToDirect(Checks& checks);
void TestDirectToRegister(Checks& checks);

}  // namespace c166::test::movbz

int main() {
  c166::test::Checks checks;
  c166::test::movbz::TestByteGprToWordGpr(checks);
  c166::test::movbz::TestRegisterToDirect(checks);
  c166::test::movbz::TestDirectToRegister(checks);
  return checks.Result();
}
