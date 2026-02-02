#include "lumina/order_book.hpp"
#include <algorithm>
#include <cmath>

namespace lumina {

OrderBook::OrderBook(size_t max_orders)
  : pool_(max_orders),
    level_storage_(),
    best_bid_(nullptr),
    best_ask_(nullptr) {
  level_storage_.reserve(4096);
}

PriceLevel* OrderBook::get_or_create_level(Price price, Side side) {
  PriceToLevel& levels = side == Side::Buy ? bid_levels_ : ask_levels_;
  auto it = levels.find(price);
  if (it != levels.end()) return it->second;
  level_storage_.emplace_back();
  PriceLevel* level = &level_storage_.back();
  level->price = price;
  level->total_qty = 0;
  level->head = level->tail = nullptr;
  level->prev = level->next = nullptr;
  levels[price] = level;
  return level;
}

void OrderBook::remove_level_if_empty(PriceLevel* level, Side side) {
  if (!level || level->total_qty > 0) return;
  PriceToLevel& levels = side == Side::Buy ? bid_levels_ : ask_levels_;
  levels.erase(level->price);
  if (level->prev) level->prev->next = level->next;
  if (level->next) level->next->prev = level->prev;
  if (best_bid_ == level) update_best(Side::Buy);
  if (best_ask_ == level) update_best(Side::Sell);
}

void OrderBook::update_best(Side side) {
  PriceToLevel& levels = side == Side::Buy ? bid_levels_ : ask_levels_;
  if (levels.empty()) {
    if (side == Side::Buy) best_bid_ = nullptr;
    else best_ask_ = nullptr;
    return;
  }
  if (side == Side::Buy) {
    best_bid_ = std::max_element(levels.begin(), levels.end(),
      [](const auto& a, const auto& b) { return a.first < b.first; })->second;
  } else {
    best_ask_ = std::min_element(levels.begin(), levels.end(),
      [](const auto& a, const auto& b) { return a.first < b.first; })->second;
  }
}

bool OrderBook::add_order(OrderId id, Price price, Qty qty, Side side) {
  if (order_index_.count(id)) return false;
  OrderNodePool::Node* node = pool_.allocate();
  if (!node) return false;
  node->order.id = id;
  node->order.price = price;
  node->order.qty = qty;
  node->order.side = side;
  PriceLevel* level = get_or_create_level(price, side);
  node->prev = level->tail;
  node->next = nullptr;
  if (level->tail) level->tail->next = node;
  else level->head = node;
  level->tail = node;
  level->total_qty += qty;
  order_index_[id] = node;
  if (!best_bid_ || (side == Side::Buy && price > best_bid_->price))
    update_best(Side::Buy);
  if (!best_ask_ || (side == Side::Sell && price < best_ask_->price))
    update_best(Side::Sell);
  return true;
}

void OrderBook::cancel_order(OrderId id) {
  auto it = order_index_.find(id);
  if (it == order_index_.end()) return;
  OrderNodePool::Node* node = it->second;
  Side side = node->order.side;
  Price price = node->order.price;
  PriceLevel* level = get_or_create_level(price, side);
  if (node->prev) node->prev->next = node->next;
  else level->head = node->next;
  if (node->next) node->next->prev = node->prev;
  else level->tail = node->prev;
  level->total_qty -= node->order.qty;
  order_index_.erase(it);
  pool_.deallocate(node);
  remove_level_if_empty(level, side);
}

void OrderBook::cancel_order(OrderId id, Price price, Side side) {
  (void)price;
  (void)side;
  cancel_order(id);
}

void OrderBook::match(Side side, Qty qty, std::vector<Trade>& fills) {
  PriceLevel* level = side == Side::Buy ? best_ask_ : best_bid_;
  while (level && qty > 0) {
    OrderNodePool::Node* node = side == Side::Buy ? level->head : level->head;
    while (node && qty > 0) {
      Qty fill_qty = std::min(node->order.qty, qty);
      fills.push_back({node->order.id, 0, level->price, fill_qty, 0});
      node->order.qty -= fill_qty;
      level->total_qty -= fill_qty;
      qty -= fill_qty;
      if (node->order.qty == 0) {
        OrderNodePool::Node* to_remove = node;
        node = node->next;
        if (to_remove->prev) to_remove->prev->next = to_remove->next;
        else level->head = to_remove->next;
        if (to_remove->next) to_remove->next->prev = to_remove->prev;
        else level->tail = to_remove->prev;
        order_index_.erase(to_remove->order.id);
        pool_.deallocate(to_remove);
      } else
        node = node->next;
    }
    remove_level_if_empty(level, side == Side::Buy ? Side::Sell : Side::Buy);
    level = side == Side::Buy ? best_ask_ : best_bid_;
  }
}

Price OrderBook::best_bid() const {
  return best_bid_ ? best_bid_->price : 0;
}

Price OrderBook::best_ask() const {
  return best_ask_ ? best_ask_->price : 0;
}

Qty OrderBook::bid_volume() const {
  Qty v = 0;
  for (const auto& p : bid_levels_) v += p.second->total_qty;
  return v;
}

Qty OrderBook::ask_volume() const {
  Qty v = 0;
  for (const auto& p : ask_levels_) v += p.second->total_qty;
  return v;
}

Price OrderBook::mid_price() const {
  Price b = best_bid(), a = best_ask();
  if (b == 0 && a == 0) return 0;
  if (b == 0) return a;
  if (a == 0) return b;
  return (b + a) / 2;
}

BookLevel OrderBook::best_bid_level() const {
  if (!best_bid_) return {};
  return {best_bid_->price, best_bid_->total_qty, 0};
}

BookLevel OrderBook::best_ask_level() const {
  if (!best_ask_) return {};
  return {best_ask_->price, best_ask_->total_qty, 0};
}

void OrderBook::get_bid_ask_volumes(Qty& bid_vol, Qty& ask_vol) const {
  bid_vol = bid_volume();
  ask_vol = ask_volume();
}

} // namespace lumina
