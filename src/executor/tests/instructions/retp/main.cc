#include "helper.h"

namespace c166::test::retp {

// C166S V1 SubSystem User's Manual v1.6, p. 6-79.
void TestReturnIpAndRegister(Checks& checks);

}  // namespace c166::test::retp

int main() {
  c166::test::Checks checks;
  c166::test::retp::TestReturnIpAndRegister(checks);
  return checks.Result();
}
