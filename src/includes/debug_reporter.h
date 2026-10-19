#pragma once
#include "debug_snapshot.h"
#include <ostream>

void report_state(std::ostream& out, const CpuSnapshot& s);
void report_pc(std::ostream& out, const CpuSnapshot& s);
void report_regs(std::ostream& out, const CpuSnapshot& s);
void report_psw(std::ostream& out, const CpuSnapshot& s);