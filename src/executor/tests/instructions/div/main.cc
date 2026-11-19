#include "helper.h"

namespace c166::test::div {

void TestWordGpr(Checks& checks);

}  // namespace c166::test::div

int main() {
  c166::test::Checks checks;
  c166::test::div::TestWordGpr(checks);
  return checks.Result();
}
