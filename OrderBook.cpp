#include "OrderBook.h"

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
		return std::nullopt;
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

bool OrderBook::add(const Order& order)
{
	if (order.remaining == 0 or orders_.contains(order.orderId)) {
		return false;
	}

	auto& side{ getSide(order.side) };

	// create price level if it doesnt exist
	auto [it, inserted] {side.try_emplace(order.price, PriceLevel{order.price}) };
	auto& level{ it->second };

	if (!level.push_back(order)) {
		return false;
	}

	// add to orders map
	orders_.emplace(order.orderId, OrderBook::OrderLocation{ order.side, order.price });

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
	side.at(info.price).cancel(id);

	// remove info entry
	orders_.erase(id);

	return true;
}

Trades OrderBook::fill(Order& order)
{
	if (order.remaining == 0) {
		return {};
	}

	auto* level{ getBestLevel(getOppositeSide(order.side)) };

	if (!level or level->empty()) {
		return {};
	}

	auto trades{ level->fill(order) };

	for (const auto& trade : trades) {
		if (!level->contains(trade.passiveId)) {
			orders_.erase(trade.passiveId);
		}
	}

	return trades;
}

std::optional<Price> OrderBook::bestPrice(Side side) const
{
	return getBestPrice(side);
}

Quantity OrderBook::fillableQuantity(Side side, Price limit) const
{
	Quantity quantity{};

	if (side == Side::Sell) {
		// A sell order matches against Bids >= limit.
		// Since bids_ is sorted ascending, everything from lower_bound(limit) onwards is >= limit.
		auto it{ bids_.lower_bound(limit) };

		for (; it != bids_.end(); ++it) {
			quantity += it->second.totalQuantity();
		}
	}
	else {
		// A buy order matches against Asks <= limit.
		// Since asks_ is sorted ascending, everything from begin() up to upper_bound(limit) is <= limit.
		auto endIt{ asks_.upper_bound(limit) };

		for (auto it{ asks_.begin() }; it != endIt; ++it) {
			quantity += it->second.totalQuantity();
		}
	}

	return quantity;
}

bool OrderBook::empty(Side side) const
{
	auto& bookSide{ getSide(side) };
	return bookSide.empty();
}
