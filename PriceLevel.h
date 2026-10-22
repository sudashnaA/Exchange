#pragma once
#include <list>
#include <stdexcept>
#include <unordered_map>
#include "Structures.h"

class PriceLevel
{
private:
	using PriceLevelType = std::list<Order>;

public:
	using iterator = PriceLevelType::iterator;
	using const_iterator = PriceLevelType::const_iterator;
	using OrderMap = std::unordered_map<OrderId, iterator>;

private:
	const Price price_{};
	PriceLevelType level_{};
	Quantity totalQuantity_{};
	OrderMap orders_{};

public:
	PriceLevel(Price price) 
		: price_{price}
	{
	};

	bool push_back(const Order& order);

	// return amount remaining
	Quantity fill(Quantity amount);

	bool cancel(OrderId id);

	bool contains(OrderId id) const;

	PriceLevel::iterator find(OrderId id);

	PriceLevel::const_iterator find(OrderId id) const;

	Quantity totalQuantity() const { return totalQuantity_; };
	
	bool empty() const { return level_.empty(); };

	std::size_t orderCount() const { return level_.size(); };

	Price price() const { return price_; };
};

