#include "helper.h"

namespace c166::test::jnb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-56.
void TestBitAndRelativeOffset(Checks& checks);

}  // namespace c166::test::jnb

int main() {
  c166::test::Checks checks;
  c166::test::jnb::TestBitAndRelativeOffset(checks);
  return checks.Result();
}
