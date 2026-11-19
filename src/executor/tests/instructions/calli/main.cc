#include "helper.h"

namespace c166::test::calli {

// C166S V1 SubSystem User's Manual v1.6, p. 6-22.
void TestConditionalIndirect(Checks& checks);

}  // namespace c166::test::calli

int main() {
  c166::test::Checks checks;
  c166::test::calli::TestConditionalIndirect(checks);
  return checks.Result();
}
