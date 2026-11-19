#include "helper.h"

namespace c166::test::extpr {

void TestImmediatePage(Checks& checks);
void TestRegisterPage(Checks& checks);

}  // namespace c166::test::extpr

int main() {
  c166::test::Checks checks;
  c166::test::extpr::TestImmediatePage(checks);
  c166::test::extpr::TestRegisterPage(checks);
  return checks.Result();
}
