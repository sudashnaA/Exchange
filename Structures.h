#pragma once
#include <cstdint>

enum class Side : std::uint8_t { Buy, Sell };
enum class OrderType : std::uint8_t { Limit, Market };
enum class TimeInForce : std::uint8_t { GTC, IOC, FOK };

using OrderId = std::uint64_t;
using Price = std::int64_t;
using Quantity = std::uint64_t;
using SequenceNumber = std::uint64_t;
using SymbolId = std::uint64_t;
using UserId = std::uint64_t;

struct NewOrder {
    OrderId     id{};
    Price       price{};
    Quantity    quantity{};
    UserId      userId{};
    SymbolId    symbol{};

    // enum go after
    Side        side{};
    OrderType   type{};
    TimeInForce tif{};
};

// resting orders
struct Order {
    OrderId        orderId{};
    Price          price{};
    Quantity       remaining{};
    Quantity       originalQuantity{};
    SequenceNumber sequenceNumber{};
    UserId         userId{};

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