#pragma once

#include "config.h"
#include "counter.h"
#include "dist_summary.h"
#include "gauge.h"
#include "publisher.h"
#include "timer.h"
#include <mutex>
#include <ska/flat_hash_map.hpp>

namespace spectator {
class Registry {
 public:
  using clock = std::chrono::steady_clock;

  explicit Registry(Config config) noexcept;
  const Config& GetConfig() const noexcept;

  IdPtr CreateId(std::string name, Tags tags) noexcept;

  std::shared_ptr<DefaultCounter> Counter(IdPtr id) noexcept;
  std::shared_ptr<DefaultCounter> Counter(std::string name) noexcept;

  std::shared_ptr<DefaultDistributionSummary> DistributionSummary(
      IdPtr id) noexcept;
  std::shared_ptr<DefaultDistributionSummary> DistributionSummary(
      std::string name) noexcept;

  std::shared_ptr<DefaultGauge> Gauge(IdPtr id) noexcept;
  std::shared_ptr<DefaultGauge> Gauge(std::string name) noexcept;

  std::shared_ptr<DefaultTimer> Timer(IdPtr id) noexcept;
  std::shared_ptr<DefaultTimer> Timer(std::string name) noexcept;

  std::vector<std::shared_ptr<Meter>> Meters() const noexcept;
  std::vector<Measurement> Measurements() const noexcept;

  void Start() noexcept;
  void Stop() noexcept;

 private:
  Config config_;
  mutable std::mutex meters_mutex{};
  ska::flat_hash_map<IdPtr, std::shared_ptr<Meter>> meters_;

  std::shared_ptr<Meter> insert_if_needed(
      std::shared_ptr<Meter> meter) noexcept;
  void log_type_error(const Id& id, MeterType prev_type,
                      MeterType attempted_type) const noexcept;

  template <typename M, typename... Args>
  std::shared_ptr<M> create_and_register_as_needed(IdPtr id,
                                                   Args&&... args) noexcept {
    std::shared_ptr<M> new_meter_ptr{
        std::make_shared<M>(std::move(id), std::forward<Args>(args)...)};
    auto meter_ptr = insert_if_needed(new_meter_ptr);
    if (meter_ptr->GetType() != new_meter_ptr->GetType()) {
      log_type_error(*meter_ptr->MeterId(), meter_ptr->GetType(),
                     new_meter_ptr->GetType());
      return new_meter_ptr;
    }
    return std::static_pointer_cast<M>(meter_ptr);
  }

  Publisher<Registry> publisher_;
};

}  // namespace spectator