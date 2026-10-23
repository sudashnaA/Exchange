#pragma once
#include <map>
#include <unordered_map>
#include <optional>
#include "PriceLevel.h"

class OrderBook
{
private:
	struct OrderLocation {
		Side side;
		Price price;
	};

private:
	using BookSide = std::map<Price, PriceLevel>;

	BookSide bids_{};
	BookSide asks_{};

	std::unordered_map<OrderId, OrderLocation> orders_{};

	const BookSide& getSide(Side side) const;
	BookSide& getSide(Side side);

	std::optional<Price> getBestPrice(Side side) const;
	PriceLevel* getBestLevel(Side side);

public:
	bool add(const Order& order);

	bool cancel(OrderId id);

	Trades fill(Order& order);

	std::optional<Price> bestPrice(Side side) const;

	// Use for FOK
	Quantity fillableQuantity(Side side, Price limit) const;

	bool empty(Side side) const;

	std::size_t totalQuantity() const;

	Quantity quantity(Side side) const;

	Quantity quantityAt(Side side, PriceLevel level) const;

	std::size_t levelCount(Side side) const;

	bool contains(OrderId id) const;
};

