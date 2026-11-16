#include "helper.h"

namespace c166::test::pcall {

// C166S V1 SubSystem User's Manual v1.6, pp. 6-71–6-72.
void TestRegisterAndAbsoluteTarget(Checks& checks);

}  // namespace c166::test::pcall

int main() {
  c166::test::Checks checks;
  c166::test::pcall::TestRegisterAndAbsoluteTarget(checks);
  return checks.Result();
}
