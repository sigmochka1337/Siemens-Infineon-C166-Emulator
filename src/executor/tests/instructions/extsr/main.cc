#include "helper.h"

namespace c166::test::extsr {

void TestImmediateSegment(Checks& checks);
void TestRegisterSegment(Checks& checks);

}  // namespace c166::test::extsr

int main() {
  c166::test::Checks checks;
  c166::test::extsr::TestImmediateSegment(checks);
  c166::test::extsr::TestRegisterSegment(checks);
  return checks.Result();
}
