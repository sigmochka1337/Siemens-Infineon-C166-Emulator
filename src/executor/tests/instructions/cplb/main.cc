#include "helper.h"

namespace c166::test::cplb {

// C166S V1 SubSystem User's Manual v1.6, p. 6-32.
void TestByteGpr(Checks& checks);

}  // namespace c166::test::cplb

int main() {
  c166::test::Checks checks;
  c166::test::cplb::TestByteGpr(checks);
  return checks.Result();
}
