#include "helper.h"

namespace c166::test::negb {

void TestByteGpr(Checks& checks);

}  // namespace c166::test::negb

int main() {
  c166::test::Checks checks;
  c166::test::negb::TestByteGpr(checks);
  return checks.Result();
}
