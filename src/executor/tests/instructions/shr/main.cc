#include "helper.h"

namespace c166::test::shr {

// C166S V1 SubSystem User's Manual v1.6, p. 6-88.
void TestImmediateCount(Checks& checks);
void TestRegisterCount(Checks& checks);

}  // namespace c166::test::shr

int main() {
  c166::test::Checks checks;
  c166::test::shr::TestImmediateCount(checks);
  c166::test::shr::TestRegisterCount(checks);
  return checks.Result();
}
