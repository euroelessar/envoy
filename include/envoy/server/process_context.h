#pragma once

#include "envoy/common/pure.h"

namespace Envoy {

/**
 * Represents some other part of the process.
 */
class ProcessObject {
public:
  virtual ~ProcessObject() = default;
};

/**
 * Context passed to filters to access resources from non-Envoy parts of the
 * process.
 */
class ProcessContext {
public:
  virtual ~ProcessContext() = default;

  /**
   * @return the ProcessObject for this context.
   */
  virtual ProcessObject& get() const PURE;
};

} // namespace Envoy
