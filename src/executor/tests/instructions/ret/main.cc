#include "helper.h"

namespace c166::test::ret {

// C166S V1 SubSystem User's Manual v1.6, p. 6-77.
void TestReturnIp(Checks& checks);

}  // namespace c166::test::ret

int main() {
  c166::test::Checks checks;
  c166::test::ret::TestReturnIp(checks);
  return checks.Result();
}
