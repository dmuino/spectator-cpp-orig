#include <gtest/gtest.h>
#include <fmt/ostream.h>
#include "../spectator/registry.h"

namespace {
using spectator::Config;
using spectator::Registry;

TEST(Registry, Counter) {
  Registry r{Config{}};
  auto c = r.Counter("foo");
  c->Increment();
  EXPECT_EQ(c->Count(), 1);
}

TEST(Registry, CounterGet) {
  Registry r{Config{}};
  auto c = r.Counter("foo");
  c->Increment();
  EXPECT_EQ(r.Counter("foo")->Count(), 1);
}

TEST(Registry, DistSummary) {
  Registry r{Config{}};
  auto ds = r.DistributionSummary("ds");
  ds->Record(100);
  EXPECT_EQ(r.DistributionSummary("ds")->TotalAmount(), 100);
}

TEST(Registry, Gauge) {
  Registry r{Config{}};
  auto g = r.Gauge("g");
  g->Set(100);
  EXPECT_EQ(r.Gauge("g")->Get(), 100);
}

TEST(Registry, Timer) {
  Registry r{Config{}};
  auto t = r.Timer("t");
  t->Record(std::chrono::microseconds(1));
  EXPECT_EQ(r.Timer("t")->TotalTime(), 1000);
}

TEST(Registry, WrongType) {
  Registry r{Config{}};
  auto t = r.Timer("meter");
  t->Record(std::chrono::nanoseconds(1));

  // this should log an error but still return a Gauge
  auto g = r.Gauge("meter");
  g->Set(42);

  // the actual meter in the registry is the timer
  EXPECT_EQ(r.Timer("meter")->TotalTime(), 1);

  // the gauge is not persisted
  EXPECT_TRUE(std::isnan(r.Gauge("meter")->Get()));
}

TEST(Registry, Meters) {
  Registry r{Config{}};
  auto t = r.Timer("t");
  auto c = r.Counter("c");
  r.Timer("t")->Count();
  auto meters = r.Meters();
  ASSERT_EQ(meters.size(), 2);
}

TEST(Registry, MeasurementTest) {
  Registry r{Config{}};
  auto c = r.Counter("c");
  c->Increment();
  auto m = c->Measure().front();
  // test to string
  auto str = fmt::format("{}", m);
  EXPECT_EQ(str, "Measurement{Id(c, [statistic->count]),1}");

  // test equals
  c->Increment();
  auto m2 = c->Measure().front();
  EXPECT_EQ(m, m2);
}
}  // namespace
