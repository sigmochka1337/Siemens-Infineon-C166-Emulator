#pragma once
#include <span>

#include "interfaces/decoder.h"
#include "interfaces/executor.h"
#include "interfaces/fetcher.h"
#include "interfaces/memory.h"
#include "interfaces/raw_memory.h"

namespace c166 {
class ICPUCore {
 public:
  virtual ExecutionResult Tick() = 0;
  virtual void Reset() = 0;
  virtual void SetEntryPoint(CodeAddress address) = 0;
  virtual CodeAddress GetEntryPoint() const = 0;
  virtual bool LoadBinary(PhysicalAddress load_address, std::span<const Byte> bytes) = 0;
  virtual CPUSnapshot Snapshot() const = 0;

  virtual ~ICPUCore() = default;
};
}  // namespace c166
