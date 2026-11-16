#include "helper.h"

namespace c166::test::pop {

// C166S V1 SubSystem User's Manual v1.6, p. 6-73.
void TestStackToRegister(Checks& checks);

}  // namespace c166::test::pop

int main() {
  c166::test::Checks checks;
  c166::test::pop::TestStackToRegister(checks);
  return checks.Result();
}
