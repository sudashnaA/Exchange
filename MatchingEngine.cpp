#include "MatchingEngine.h"

Trades MatchingEngine::handleMarketOrder(NewOrder& order)
{
    // tif should be undefined for market
    if (order.type != OrderType::Market or order.tif) {
        return {};
    }

    auto convertedOrder{ convertOrder(order, orderSequenceNumber_++) };
    auto trades{ book_.fill(convertedOrder) };

    for (auto& trade : trades) {
        trade.sequenceNumber = tradeSequenceNumber_++;
    }

    return trades;
}

Trades MatchingEngine::handleLimitOrder(NewOrder& order)
{
    // if we have a limit order, we must have tif
    if (order.type != OrderType::Limit or !order.tif) {
        return {};
    }

    auto convertedOrder{ convertOrder(order, orderSequenceNumber_++) };

    // cant fill FOK order
    if (*order.tif == TimeInForce::FOK and !book_.canFillFOK(convertedOrder)) {
        return {};
    }

    auto trades{ book_.fill(convertedOrder) };

    for (auto& trade : trades) {
        trade.sequenceNumber = tradeSequenceNumber_++;
    }

    // GTC orders can rest in the book
    if (convertedOrder.remaining and *order.tif == TimeInForce::GTC) {
        book_.add(convertedOrder);
    }

    return trades;
}

Trades MatchingEngine::add(NewOrder& order)
{
    if (order.type == OrderType::Market) {
        return handleMarketOrder(order);
    }
    else if (order.type == OrderType::Limit) {
        return handleLimitOrder(order);
    }

    return {};
}

bool MatchingEngine::cancel(OrderId order) 
{
    return book_.cancel(order);
}