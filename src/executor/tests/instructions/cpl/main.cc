#include "helper.h"

namespace c166::test::cpl {

// C166S V1 SubSystem User's Manual v1.6, p. 6-31.
void TestWordGpr(Checks& checks);

}  // namespace c166::test::cpl

int main() {
  c166::test::Checks checks;
  c166::test::cpl::TestWordGpr(checks);
  return checks.Result();
}
