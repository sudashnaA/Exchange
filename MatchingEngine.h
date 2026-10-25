#pragma once
#include "Structures.h"
#include "OrderBook.h"

class MatchingEngine
{
private:
	inline static SequenceNumber orderSequenceNumber_{ STARTING_SEQUENCE_NUMBER };
	inline static SequenceNumber tradeSequenceNumber_{ STARTING_SEQUENCE_NUMBER };

	const SymbolId symbol_;
	OrderBook book_;

	Trades handleMarketOrder(NewOrder& order);
	Trades handleLimitOrder(NewOrder& order);

public:
	explicit MatchingEngine(SymbolId symbol) 
		: symbol_{symbol}
		, book_{symbol}
	{

	}

	Trades add(NewOrder& order);
	bool cancel(OrderId order);
};

