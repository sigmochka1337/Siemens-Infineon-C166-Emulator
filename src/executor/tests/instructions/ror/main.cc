#include "helper.h"

namespace c166::test::ror {

// C166S V1 SubSystem User's Manual v1.6, p. 6-83.
void TestImmediateCount(Checks& checks);
void TestRegisterCount(Checks& checks);

}  // namespace c166::test::ror

int main() {
  c166::test::Checks checks;
  c166::test::ror::TestImmediateCount(checks);
  c166::test::ror::TestRegisterCount(checks);
  return checks.Result();
}
