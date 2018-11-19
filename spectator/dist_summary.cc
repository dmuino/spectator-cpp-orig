#include "dist_summary.h"
#include "atomicnumber.h"

namespace spectator {

DistributionSummary::DistributionSummary(IdPtr id) noexcept
    : id_(std::move(id)), count_(0), total_(0), totalSq_(0.0), max_(0) {}

IdPtr DistributionSummary::MeterId() const noexcept { return id_; }

std::vector<Measurement> DistributionSummary::Measure() const noexcept {
  std::vector<Measurement> results;
  auto cnt = count_.exchange(0, std::memory_order_relaxed);
  if (cnt == 0) {
    return results;
  }

  auto total = total_.exchange(0, std::memory_order_relaxed);
  auto t_sq = totalSq_.exchange(0.0, std::memory_order_relaxed);
  auto mx = max_.exchange(0, std::memory_order_relaxed);
  results.reserve(4);
  results.push_back({id_->WithStat("count"), static_cast<double>(cnt)});
  results.push_back({id_->WithStat("totalAmount"), static_cast<double>(total)});
  results.push_back({id_->WithStat("totalOfSquares"), t_sq});
  results.push_back({id_->WithStat("max"), static_cast<double>(mx)});
  return results;
}

void DistributionSummary::Record(int64_t amount) noexcept {
  if (amount >= 0) {
    count_.fetch_add(1, std::memory_order_relaxed);
    total_.fetch_add(amount, std::memory_order_relaxed);
    add_double(&totalSq_, static_cast<double>(amount) * amount);
    update_max(&max_, amount);
  }
}

int64_t DistributionSummary::Count() const noexcept {
  return count_.load(std::memory_order_relaxed);
}

int64_t DistributionSummary::TotalAmount() const noexcept {
  return total_.load(std::memory_order_relaxed);
}

}  // namespace spectator
