#include "helper.h"

namespace c166::test::trap {

void TestTrapNumber(Checks& checks);

}  // namespace c166::test::trap

int main() {
  c166::test::Checks checks;
  c166::test::trap::TestTrapNumber(checks);
  return checks.Result();
}
