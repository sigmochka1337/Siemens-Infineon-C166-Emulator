#include "helper.h"

namespace c166::test::rets {

// C166S V1 SubSystem User's Manual v1.6, p. 6-80.
void TestReturnIpAndSegment(Checks& checks);

}  // namespace c166::test::rets

int main() {
  c166::test::Checks checks;
  c166::test::rets::TestReturnIpAndSegment(checks);
  return checks.Result();
}
