#include "helper.h"

namespace c166::test::extp {

void TestImmediatePage(Checks& checks);
void TestRegisterPage(Checks& checks);

}  // namespace c166::test::extp

int main() {
  c166::test::Checks checks;
  c166::test::extp::TestImmediatePage(checks);
  c166::test::extp::TestRegisterPage(checks);
  return checks.Result();
}
