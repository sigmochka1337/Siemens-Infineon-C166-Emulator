#include "helper.h"

namespace c166::test::ashr {

// C166S V1 SubSystem User's Manual v1.6, p. 6-8.
void TestImmediateCount(Checks& checks);
void TestRegisterCount(Checks& checks);

}  // namespace c166::test::ashr

int main() {
  c166::test::Checks checks;
  c166::test::ashr::TestImmediateCount(checks);
  c166::test::ashr::TestRegisterCount(checks);
  return checks.Result();
}
