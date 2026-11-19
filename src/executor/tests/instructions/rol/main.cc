#include "helper.h"

namespace c166::test::rol {

// C166S V1 SubSystem User's Manual v1.6, p. 6-81.
void TestImmediateCount(Checks& checks);
void TestRegisterCount(Checks& checks);

}  // namespace c166::test::rol

int main() {
  c166::test::Checks checks;
  c166::test::rol::TestImmediateCount(checks);
  c166::test::rol::TestRegisterCount(checks);
  return checks.Result();
}
