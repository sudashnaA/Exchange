#pragma once
#include <cstdint>
#include <vector>
#include <optional>

enum class Side : std::uint8_t { Buy, Sell };
enum class OrderType : std::uint8_t { Limit, Market };
enum class TimeInForce : std::uint8_t { GTC, IOC, FOK };

using OrderId = std::uint64_t;
using Price = std::int64_t;
using Quantity = std::uint64_t;
using SequenceNumber = std::uint64_t;
using SymbolId = std::uint64_t;
using UserId = std::uint64_t;

inline constexpr SequenceNumber SEQUENCE_NUMBER_UNINITALIZED{ 0 };
inline constexpr SequenceNumber STARTING_SEQUENCE_NUMBER{ 1 };

struct NewOrder {
    OrderId     id{};
    Price       price{};
    Quantity    quantity{};
    UserId      userId{};
    SymbolId    symbol{};

    // enum go after
    std::optional<TimeInForce> tif{};
    Side        side{};
    OrderType   type{};
};

// resting orders
struct Order {
    OrderId        orderId{};
    Price          price{};
    Quantity       remaining{};
    Quantity       originalQuantity{};
    SequenceNumber sequenceNumber{};
    UserId         userId{};
    SymbolId       symbolId{};

    // enum go after
    Side           side{};
};

struct Trade {
    SequenceNumber sequenceNumber{};
    OrderId aggressorId{};
    OrderId passiveId{};
    UserId aggressorUserId{};
    UserId passiveUserId{};
    Price price{};
    Quantity quantity{};
    SymbolId symbol{};
    Side aggressorSide{};
};

using Trades = std::vector<Trade>;

inline Trade makeTrade(Order& aggressor, Order& passive) {
    const auto filled{ std::min(aggressor.remaining, passive.remaining) };

    aggressor.remaining -= filled;
    passive.remaining -= filled;

    return {
        SEQUENCE_NUMBER_UNINITALIZED,
        aggressor.orderId,
        passive.orderId,
        aggressor.userId,
        passive.userId,
        passive.price,
        filled,
        aggressor.symbolId,
        aggressor.side,
    };
}

inline Side getOppositeSide(Side side) {
    return (side == Side::Buy) ? Side::Sell : Side::Buy;
}

inline Order convertOrder(const NewOrder& newOrder, SequenceNumber sequenceNumber)
{
    return Order{
        .orderId = newOrder.id,
        .price = newOrder.price,
        .remaining = newOrder.quantity,
        .originalQuantity = newOrder.quantity,
        .sequenceNumber = sequenceNumber,
        .userId = newOrder.userId,
        .symbolId = newOrder.symbol,
        .side = newOrder.side
    };
}