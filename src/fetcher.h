#pragma once

#include "interfaces/fetcher.h"
#include "interfaces/memory.h"

namespace c166 {

// Выборка и декодирование одной инструкции C166 classic.
//
// Fetcher читает байты по физическому адресу CSP:IP и отдаёт Instruction.
// Он не меняет состояние машины: PC, регистры и память остаются прежними.
class Fetcher final : public IFetcher {
 public:
  explicit Fetcher(const IMachine& machine) : machine_(machine) {}

  Instruction Fetch(CodeAddress pc) override;

 private:
  const IMachine& machine_;
};

}  // namespace c166
