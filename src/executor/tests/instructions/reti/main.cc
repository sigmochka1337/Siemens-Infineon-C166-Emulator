#include "helper.h"

namespace c166::test::reti {

// C166S V1 SubSystem User's Manual v1.6, p. 6-78.
void TestInterruptFrame(Checks& checks);

}  // namespace c166::test::reti

int main() {
  c166::test::Checks checks;
  c166::test::reti::TestInterruptFrame(checks);
  return checks.Result();
}
