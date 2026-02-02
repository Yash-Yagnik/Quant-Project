#include "lumina/market_data_handler.hpp"
#include "lumina/strategy_engine.hpp"
#include "lumina/risk_checks.hpp"
#include "lumina/fix_engine.hpp"
#include "lumina/kdb_mock.hpp"
#include <iostream>
#include <memory>
#include <chrono>

using namespace lumina;

int main() {
  auto ring = std::make_shared<MarketDataHandler::MDRing>();
  PreTradeRisk risk(10'000'000, 10'000);

  MarketDataHandler md(ring);
  StrategyEngine strategy(ring, 0.1, 0.02, 3600.0, risk);

  strategy.set_order_callback([](OrderId id, Price price, Qty qty, Side side, bool is_bid) {
    (void)id;
    std::cout << (is_bid ? "BID" : "ASK") << " " << price << " x " << qty << "\n";
  });

  md.start();
  md.on_trade(10000, 100, 0);
  md.on_trade(10001, 50, 1000000);
  strategy.poll();

  KDBMock kdb;
  kdb.insert_trade(0, 10000, 100);
  std::cout << "KDB mock last price: " << kdb.last_price().value_or(0) << "\n";

  md.stop();
  std::cout << "Lumina-HFT core OK.\n";
  return 0;
}
