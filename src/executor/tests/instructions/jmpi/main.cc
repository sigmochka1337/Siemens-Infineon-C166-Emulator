#include "helper.h"

namespace c166::test::jmpi {

// C166S V1 SubSystem User's Manual v1.6, p. 6-53.
void TestConditionalIndirect(Checks& checks);

}  // namespace c166::test::jmpi

int main() {
  c166::test::Checks checks;
  c166::test::jmpi::TestConditionalIndirect(checks);
  return checks.Result();
}
