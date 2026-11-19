#include "helper.h"

namespace c166::test::jmpa {

// C166S V1 SubSystem User's Manual v1.6, p. 6-52.
void TestConditionalAbsolute(Checks& checks);

}  // namespace c166::test::jmpa

int main() {
  c166::test::Checks checks;
  c166::test::jmpa::TestConditionalAbsolute(checks);
  return checks.Result();
}
