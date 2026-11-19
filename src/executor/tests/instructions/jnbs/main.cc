#include "helper.h"

namespace c166::test::jnbs {

// C166S V1 SubSystem User's Manual v1.6, p. 6-57.
void TestBitAndRelativeOffset(Checks& checks);

}  // namespace c166::test::jnbs

int main() {
  c166::test::Checks checks;
  c166::test::jnbs::TestBitAndRelativeOffset(checks);
  return checks.Result();
}
