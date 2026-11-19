#include "helper.h"

namespace c166::test::calla {

// C166S V1 SubSystem User's Manual v1.6, p. 6-21.
void TestConditionalAbsolute(Checks& checks);

}  // namespace c166::test::calla

int main() {
  c166::test::Checks checks;
  c166::test::calla::TestConditionalAbsolute(checks);
  return checks.Result();
}
