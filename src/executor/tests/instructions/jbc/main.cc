#include "helper.h"

namespace c166::test::jbc {

// C166S V1 SubSystem User's Manual v1.6, p. 6-50.
void TestBitAndRelativeOffset(Checks& checks);

}  // namespace c166::test::jbc

int main() {
  c166::test::Checks checks;
  c166::test::jbc::TestBitAndRelativeOffset(checks);
  return checks.Result();
}
