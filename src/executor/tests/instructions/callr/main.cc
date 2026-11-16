#include "helper.h"

namespace c166::test::callr {

// C166S V1 SubSystem User's Manual v1.6, p. 6-23.
void TestRelativeTarget(Checks& checks);

}  // namespace c166::test::callr

int main() {
  c166::test::Checks checks;
  c166::test::callr::TestRelativeTarget(checks);
  return checks.Result();
}
