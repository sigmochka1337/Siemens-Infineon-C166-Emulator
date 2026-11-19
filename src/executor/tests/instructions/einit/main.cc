#include "helper.h"

namespace c166::test::einit {

void TestProtectedEncoding(Checks& checks);

}  // namespace c166::test::einit

int main() {
  c166::test::Checks checks;
  c166::test::einit::TestProtectedEncoding(checks);
  return checks.Result();
}
