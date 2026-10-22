#include "PriceLevel.h"

bool PriceLevel::push_back(const Order& order)
{
	if (orders_.contains(order.orderId) or order.remaining == 0) {
		return false;
	}

	level_.push_back(order);
	totalQuantity_ += order.remaining;
	orders_.emplace(order.orderId, std::prev(level_.end()));
	return true;
}

Quantity PriceLevel::fill(Quantity amount)
{
	if (amount == 0) {
		return 0;
	}

	while (!empty() and amount > 0) {
		auto& order{ *level_.begin() };
		
		Quantity filled{ std::min(amount, order.remaining) };

		order.remaining -= filled;
		amount -= filled;
		totalQuantity_ -= filled;

		if (order.remaining == 0) {
			orders_.erase(order.orderId);
			level_.erase(level_.begin());
		}
	}

	return amount;
}

bool PriceLevel::cancel(OrderId id) {
	auto it{ orders_.find(id) };

	if (it == orders_.end()) {
		return false;
	}

	auto orderIt{ it->second };
	totalQuantity_ -= orderIt->remaining;

	level_.erase(orderIt);
	orders_.erase(it);

	return true;
}

bool PriceLevel::contains(OrderId id) const {
	return orders_.contains(id);
}

PriceLevel::iterator PriceLevel::find(OrderId id) {
	auto it{ orders_.find(id) };

	if (it == orders_.end()) {
		return level_.end();
	}

	return it->second;
}

PriceLevel::const_iterator PriceLevel::find(OrderId id) const {
	auto it{ orders_.find(id) };

	if (it == orders_.end()) {
		return level_.end();
	}

	return it->second;
}