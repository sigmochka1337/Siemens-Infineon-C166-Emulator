#include "helper.h"

namespace c166::test::bset {

// C166S V1 SubSystem User's Manual v1.6, p. 6-19.
void TestDirectBit(Checks& checks);

}  // namespace c166::test::bset

int main() {
  c166::test::Checks checks;
  c166::test::bset::TestDirectBit(checks);
  return checks.Result();
}
