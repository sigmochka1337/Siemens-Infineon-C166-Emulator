#include "helper.h"

namespace c166::test::diswdt {

void TestProtectedEncoding(Checks& checks);

}  // namespace c166::test::diswdt

int main() {
  c166::test::Checks checks;
  c166::test::diswdt::TestProtectedEncoding(checks);
  return checks.Result();
}
