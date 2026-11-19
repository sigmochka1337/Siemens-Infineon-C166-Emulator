#include "helper.h"

namespace c166::test::jb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-49.
void TestBitAndRelativeOffset(Checks& checks);

}  // namespace c166::test::jb

int main() {
  c166::test::Checks checks;
  c166::test::jb::TestBitAndRelativeOffset(checks);
  return checks.Result();
}
