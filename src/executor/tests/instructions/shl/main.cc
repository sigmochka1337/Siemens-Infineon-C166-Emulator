#include "helper.h"

namespace c166::test::shl {

// C166S V1 SubSystem User's Manual v1.6, p. 6-86.
void TestImmediateCount(Checks& checks);
void TestRegisterCount(Checks& checks);

}  // namespace c166::test::shl

int main() {
  c166::test::Checks checks;
  c166::test::shl::TestImmediateCount(checks);
  c166::test::shl::TestRegisterCount(checks);
  return checks.Result();
}
