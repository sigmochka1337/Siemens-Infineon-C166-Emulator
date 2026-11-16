#include "helper.h"

namespace c166::test::jmpr {

// C166S V1 SubSystem User's Manual v1.6, p. 6-54.
void TestConditionalRelative(Checks& checks);

}  // namespace c166::test::jmpr

int main() {
  c166::test::Checks checks;
  c166::test::jmpr::TestConditionalRelative(checks);
  return checks.Result();
}
