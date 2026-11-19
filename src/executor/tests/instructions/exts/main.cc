#include "helper.h"

namespace c166::test::exts {

void TestImmediateSegment(Checks& checks);
void TestRegisterSegment(Checks& checks);

}  // namespace c166::test::exts

int main() {
  c166::test::Checks checks;
  c166::test::exts::TestImmediateSegment(checks);
  c166::test::exts::TestRegisterSegment(checks);
  return checks.Result();
}
