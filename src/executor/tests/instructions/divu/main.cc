#include "helper.h"

namespace c166::test::divu {

void TestWordGpr(Checks& checks);

}  // namespace c166::test::divu

int main() {
  c166::test::Checks checks;
  c166::test::divu::TestWordGpr(checks);
  return checks.Result();
}
