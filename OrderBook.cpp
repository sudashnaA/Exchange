#include "OrderBook.h"
#include <any>

const OrderBook::BookSide& OrderBook::getSide(Side side) const
{
	return (side == Side::Buy) ? bids_ : asks_;
}

OrderBook::BookSide& OrderBook::getSide(Side side)
{
	return (side == Side::Buy) ? bids_ : asks_;
}

std::optional<Price> OrderBook::getBestPrice(Side side) const
{
	auto& sideRef{ getSide(side) };

	if (sideRef.empty()) {
		return {};
	}

	if (side == Side::Sell) {
		return sideRef.begin()->first;
	}
	else {
		return std::prev(sideRef.end())->first;
	}
}

PriceLevel* OrderBook::getBestLevel(Side side)
{
	auto& sideRef{ getSide(side) };

	if (sideRef.empty()) {
		return nullptr;
	}
	
	if (side == Side::Sell) {
		return &sideRef.begin()->second;
	}
	else {
		return &std::prev(sideRef.end())->second;
	}
}

bool OrderBook::insert(const Order& order)
{
	if (order.remaining == 0 or orders_.contains(order.orderId)) {
		return false;
	}

	auto& side{ getSide(order.side) };

	// create price level if it doesnt exist
	auto [it, inserted] {side.try_emplace(order.price, PriceLevel{order.price}) };

	// add order
	auto orderPosition{ it->second.push_back(order) };

	// add to orders map
	orders_.emplace(order.orderId, OrderBook::OrderLocation{ order.side, order.price, orderPosition });

	return true;
}

bool OrderBook::cancel(OrderId id)
{
	auto it{ orders_.find(id) };

	if (it == orders_.end()) {
		return false;
	}
	
	auto& info{ it->second };
	auto& side{ getSide(info.side) };

	side.at(info.price).erase(info.it);
	orders_.erase(id);

	return true;
}

Quantity OrderBook::fillBest(Side side, Quantity amount)
{
	if (amount == 0) {
		return 0;
	}

	auto* level{ getBestLevel(side) };

	if (!level or level->empty()) {
		return false;
	}

	return level->fill(amount);
}

std::optional<Price> OrderBook::bestPrice(Side side) const
{
	return getBestPrice(side);
}
