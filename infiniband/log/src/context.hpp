#pragma once

#include <dory/conn/context.hpp>
#include "log.hpp"

namespace dory{
struct ReplicationContext {
  ReplicationContext(ConnectionContext &cc, Log &log, ptrdiff_t log_offset)
      : cc{cc}, log{log}, log_offset{log_offset} {}
  ConnectionContext &cc;
  Log &log;
  ptrdiff_t log_offset;
};
}