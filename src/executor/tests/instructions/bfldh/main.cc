#include "helper.h"

namespace c166::test::bfldh {

void TestBitWordMaskData(Checks& checks);

}  // namespace c166::test::bfldh

int main() {
  c166::test::Checks checks;
  c166::test::bfldh::TestBitWordMaskData(checks);
  return checks.Result();
}
