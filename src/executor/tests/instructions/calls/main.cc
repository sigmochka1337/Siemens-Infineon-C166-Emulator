#include "helper.h"

namespace c166::test::calls {

// C166S V1 SubSystem User's Manual v1.6, p. 6-24.
void TestSegmentedAbsolute(Checks& checks);

}  // namespace c166::test::calls

int main() {
  c166::test::Checks checks;
  c166::test::calls::TestSegmentedAbsolute(checks);
  return checks.Result();
}
