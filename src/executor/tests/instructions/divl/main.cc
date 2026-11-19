#include "helper.h"

namespace c166::test::divl {

void TestWordGpr(Checks& checks);

}  // namespace c166::test::divl

int main() {
  c166::test::Checks checks;
  c166::test::divl::TestWordGpr(checks);
  return checks.Result();
}
