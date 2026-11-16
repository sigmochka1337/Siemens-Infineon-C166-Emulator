#include "helper.h"

namespace c166::test::jmps {

// C166S V1 SubSystem User's Manual v1.6, p. 6-55.
void TestSegmentedAbsolute(Checks& checks);

}  // namespace c166::test::jmps

int main() {
  c166::test::Checks checks;
  c166::test::jmps::TestSegmentedAbsolute(checks);
  return checks.Result();
}
