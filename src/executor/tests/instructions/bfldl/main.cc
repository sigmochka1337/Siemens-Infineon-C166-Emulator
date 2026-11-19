#include "helper.h"

namespace c166::test::bfldl {

void TestBitWordMaskData(Checks& checks);

}  // namespace c166::test::bfldl

int main() {
  c166::test::Checks checks;
  c166::test::bfldl::TestBitWordMaskData(checks);
  return checks.Result();
}
