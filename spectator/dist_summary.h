#pragma once
#include "meter.h"
#include <atomic>

namespace spectator {

class DistributionSummary : public Meter {
 public:
  explicit DistributionSummary(IdPtr id) noexcept;
  IdPtr MeterId() const noexcept override;
  std::vector<Measurement> Measure() const noexcept override;
  MeterType GetType() const noexcept override {
    return MeterType::DistributionSummary;
  }

  void Record(int64_t amount) noexcept;
  int64_t Count() const noexcept;
  int64_t TotalAmount() const noexcept;

 private:
  IdPtr id_;
  mutable std::atomic<int64_t> count_;
  mutable std::atomic<int64_t> total_;
  mutable std::atomic<double> totalSq_;
  mutable std::atomic<int64_t> max_;
};

}  // namespace spectator
