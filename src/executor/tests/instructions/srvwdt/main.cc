#include "helper.h"

namespace c166::test::srvwdt {

void TestProtectedEncoding(Checks& checks);

}  // namespace c166::test::srvwdt

int main() {
  c166::test::Checks checks;
  c166::test::srvwdt::TestProtectedEncoding(checks);
  return checks.Result();
}
