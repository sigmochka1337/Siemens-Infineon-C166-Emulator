#pragma once
#include "interfaces/raw_memory.h"

namespace c166 {
class IMemoryBus {
 public:
  virtual WriteResult Write8(PhysicalAddress address, Byte value) = 0;
  virtual WriteResult Write16(PhysicalAddress address, Word value) = 0;
  virtual WriteResult Write32(PhysicalAddress address, Dword value) = 0;

  virtual ReadResult<Byte> Read8(PhysicalAddress address) const = 0;
  virtual ReadResult<Word> Read16(PhysicalAddress address) const = 0;
  virtual ReadResult<Dword> Read32(PhysicalAddress address) const = 0;

  virtual ~IMemoryBus() = default;
};

class IRegisterBus {
 public:
  virtual Word GetR(uint8_t index) const = 0;
  virtual void SetR(uint8_t index, Word value) = 0;

  virtual Word GetSP() const = 0;
  virtual void SetSP(Word value) = 0;

  virtual Byte GetCSP() const = 0;
  virtual void SetCSP(Byte value) = 0;

  virtual Word GetIP() const = 0;
  virtual void SetIP(Word value) = 0;

  virtual Word GetDPP(uint8_t index) const = 0;
  virtual void SetDPP(uint8_t index, Word value) = 0;

  virtual Word GetCP() const = 0;
  virtual void SetCP(Word value) = 0;

  virtual Word GetPSW() const = 0;
  virtual void SetPSW(Word value) = 0;

  virtual Word GetMDH() const = 0;
  virtual void SetMDH(Word value) = 0;

  virtual Word GetMDL() const = 0;
  virtual void SetMDL(Word value) = 0;

  virtual ~IRegisterBus() = default;
};

class IConstantBus {
 public:
  virtual OperandAddress Store8(Byte value) = 0;
  virtual OperandAddress Store16(Word value) = 0;
  virtual OperandAddress Store32(Dword value) = 0;

  virtual ReadResult<Byte> Read8(OperandAddress address) const = 0;
  virtual ReadResult<Word> Read16(OperandAddress address) const = 0;
  virtual ReadResult<Dword> Read32(OperandAddress address) const = 0;

  virtual ~IConstantBus() = default;
};
}  // namespace c166
