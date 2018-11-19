#pragma once

#include "meter.h"
#include <atomic>

namespace spectator {

class MaxGauge : public Meter {
 public:
  explicit MaxGauge(IdPtr id) noexcept;
  IdPtr MeterId() const noexcept override;
  std::vector<Measurement> Measure() const noexcept override;
  MeterType GetType() const noexcept override { return MeterType::MaxGauge; };

  void Update(double value) noexcept;
  double Get() const noexcept;

 private:
  IdPtr id_;
  mutable std::atomic<double> value_;
};

}  // namespace spectator
