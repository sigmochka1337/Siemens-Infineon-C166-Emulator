#include "helper.h"

namespace c166::test::movbs {

// C166S V1 SubSystem User's Manual v1.6, p. 6-62.
void TestByteGprToWordGpr(Checks& checks);
void TestRegisterToDirect(Checks& checks);
void TestDirectToRegister(Checks& checks);

}  // namespace c166::test::movbs

int main() {
  c166::test::Checks checks;
  c166::test::movbs::TestByteGprToWordGpr(checks);
  c166::test::movbs::TestRegisterToDirect(checks);
  c166::test::movbs::TestDirectToRegister(checks);
  return checks.Result();
}
