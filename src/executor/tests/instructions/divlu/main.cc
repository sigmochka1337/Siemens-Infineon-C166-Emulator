#include "helper.h"

namespace c166::test::divlu {

void TestWordGpr(Checks& checks);

}  // namespace c166::test::divlu

int main() {
  c166::test::Checks checks;
  c166::test::divlu::TestWordGpr(checks);
  return checks.Result();
}
