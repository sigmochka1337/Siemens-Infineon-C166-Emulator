#pragma once

#include "interfaces/fetcher.h"

namespace c166 {
class Fetcher final : public IFetcher {
 public:
  Result<RawInstruction> Fetch(CodeAddress pc, IMemoryBus& memory,
                               const IDecoder& decoder) const override;
};
}  // namespace c166
