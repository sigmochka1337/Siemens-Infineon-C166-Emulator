#pragma once

#include <ostream>

#include "src/includes/debugger.h"

namespace c166 {

void ReportSnapshot(std::ostream& output, const DebugSnapshot& snapshot);

}  // namespace c166
