#include "helper.h"

namespace c166::test::push {

// C166S V1 SubSystem User's Manual v1.6, p. 6-75.
void TestRegisterToStack(Checks& checks);

}  // namespace c166::test::push

int main() {
  c166::test::Checks checks;
  c166::test::push::TestRegisterToStack(checks);
  return checks.Result();
}
