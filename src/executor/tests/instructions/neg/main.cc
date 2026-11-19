#include "helper.h"

namespace c166::test::neg {

// C166S V1 SubSystem User's Manual v1.6, p. 6-66.
void TestWordGpr(Checks& checks);

}  // namespace c166::test::neg

int main() {
  c166::test::Checks checks;
  c166::test::neg::TestWordGpr(checks);
  return checks.Result();
}
