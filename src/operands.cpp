#include "operands.h"

namespace c166 {
std::string IndirectMemoryOperand::ToString() const {
  switch (mode_) {
    case AddressMode::kPlain:
      return std::format("[{}]", RegisterIdToString(id_));
    case AddressMode::kPostIncrement:
      return std::format("[{}+]", RegisterIdToString(id_));
    case AddressMode::kPreDecrement:
      return std::format("[-{}]", RegisterIdToString(id_));
    case AddressMode::kDisplacement:
      return std::format("[{} + #0x{:06X}]", RegisterIdToString(id_), displacement_);
  }
  throw std::logic_error("Некорректный режим адресации");
}
}  // namespace c166
