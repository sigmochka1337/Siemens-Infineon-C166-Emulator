#include "helper.h"

namespace c166::test::mul {

void TestWordGprs(Checks& checks);

}  // namespace c166::test::mul

int main() {
  c166::test::Checks checks;
  c166::test::mul::TestWordGprs(checks);
  return checks.Result();
}
