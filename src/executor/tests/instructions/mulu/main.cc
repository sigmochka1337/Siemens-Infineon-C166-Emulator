#include "helper.h"

namespace c166::test::mulu {

void TestWordGprs(Checks& checks);

}  // namespace c166::test::mulu

int main() {
  c166::test::Checks checks;
  c166::test::mulu::TestWordGprs(checks);
  return checks.Result();
}
