#include <utility>

#include "logger.h"
#include "registry.h"
#include <fmt/ostream.h>

namespace spectator {

Registry::Registry(Config config) noexcept
    : config_{std::move(config)}, publisher_(this) {}

const Config& Registry::GetConfig() const noexcept { return config_; }

IdPtr Registry::CreateId(std::string name, Tags tags) noexcept {
  return std::make_shared<Id>(name, tags);
}

std::shared_ptr<DefaultCounter> Registry::Counter(IdPtr id) noexcept {
  return create_and_register_as_needed<DefaultCounter>(std::move(id));
}

std::shared_ptr<DefaultCounter> Registry::Counter(std::string name) noexcept {
  return Counter(CreateId(std::move(name), Tags{}));
}

std::shared_ptr<DefaultDistributionSummary> Registry::DistributionSummary(
    IdPtr id) noexcept {
  return create_and_register_as_needed<DefaultDistributionSummary>(
      std::move(id));
}

std::shared_ptr<DefaultDistributionSummary> Registry::DistributionSummary(
    std::string name) noexcept {
  return DistributionSummary(CreateId(std::move(name), Tags{}));
}

std::shared_ptr<DefaultGauge> Registry::Gauge(IdPtr id) noexcept {
  return create_and_register_as_needed<DefaultGauge>(std::move(id));
}

std::shared_ptr<DefaultGauge> Registry::Gauge(std::string name) noexcept {
  return Gauge(CreateId(std::move(name), Tags{}));
}

std::shared_ptr<DefaultTimer> Registry::Timer(IdPtr id) noexcept {
  return create_and_register_as_needed<DefaultTimer>(std::move(id));
}

std::shared_ptr<DefaultTimer> Registry::Timer(std::string name) noexcept {
  return Timer(CreateId(std::move(name), Tags{}));
}

// only insert if it doesn't exist, otherwise return the existing meter
std::shared_ptr<Meter> Registry::insert_if_needed(
    std::shared_ptr<Meter> meter) noexcept {
  std::lock_guard<std::mutex> lock(meters_mutex);
  auto insert_result = meters_.emplace(meter->MeterId(), meter);
  auto ret = insert_result.first->second;
  return ret;
}

void Registry::log_type_error(const Id& id, MeterType prev_type,
                              MeterType attempted_type) const noexcept {
  Logger()->error(
      "Attempted to register meter {} as type {} but previously registered as "
      "{}",
      id, attempted_type, prev_type);
}

void Registry::Start() noexcept { publisher_.Start(); }

void Registry::Stop() noexcept { publisher_.Stop(); }

std::vector<Measurement> Registry::Measurements() const noexcept {
  std::vector<Measurement> res;
  std::lock_guard<std::mutex> lock{meters_mutex};
  for (const auto& pair : meters_) {
    auto ms = pair.second->Measure();
    std::move(ms.begin(), ms.end(), std::back_inserter(res));
  }
  return res;
}

std::vector<std::shared_ptr<Meter>> Registry::Meters() const noexcept {
  std::vector<std::shared_ptr<Meter>> res;
  std::lock_guard<std::mutex> lock{meters_mutex};
  for (const auto& pair : meters_) {
    res.emplace_back(pair.second);
  }
  return res;
}

}  // namespace spectator
