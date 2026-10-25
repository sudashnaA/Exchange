#include "pch.h"
#include <gtest/gtest.h>
#include "PriceLevel.h"
#include "Structures.h"

class PriceLevelTest : public ::testing::Test {
protected:
    Price testPrice{ 1000 };
    PriceLevel level{ testPrice };

    // Helper to quickly construct a resting Order (typically on the Sell side in the book)
    Order createRestingOrder(OrderId id, Quantity qty, Price p = 1000) {
        return Order{
            .orderId = id,
            .price = p,
            .remaining = qty,
            .originalQuantity = qty,
            .sequenceNumber = 1,
            .userId = 42,
            .symbolId = 1,
            .side = Side::Sell
        };
    }

    // Helper to quickly construct an aggressive incoming Order (e.g., Buy)
    Order createAggressiveOrder(OrderId id, Quantity qty, Price p = 1000) {
        return Order{
            .orderId = id,
            .price = p,
            .remaining = qty,
            .originalQuantity = qty,
            .sequenceNumber = 2,
            .userId = 99,
            .symbolId = 1,
            .side = Side::Buy
        };
    }
};

// 1. Initial State Tests
TEST_F(PriceLevelTest, InitialState) {
    EXPECT_EQ(level.price(), testPrice);
    EXPECT_TRUE(level.empty());
    EXPECT_EQ(level.orderCount(), 0);
    EXPECT_EQ(level.totalQuantity(), 0);
}

// 2. Push_back Tests
TEST_F(PriceLevelTest, PushBackValidOrder) {
    Order order = createRestingOrder(1, 100);

    bool success = level.push_back(order);

    EXPECT_TRUE(success);
    EXPECT_FALSE(level.empty());
    EXPECT_EQ(level.orderCount(), 1);
    EXPECT_EQ(level.totalQuantity(), 100);
    EXPECT_TRUE(level.contains(1));
}

TEST_F(PriceLevelTest, PushBackZeroRemainingFails) {
    Order order = createRestingOrder(1, 0); // 0 remaining quantity

    bool success = level.push_back(order);

    EXPECT_FALSE(success);
    EXPECT_TRUE(level.empty());
    EXPECT_EQ(level.totalQuantity(), 0);
}

TEST_F(PriceLevelTest, PushBackDuplicateIdFails) {
    Order order1 = createRestingOrder(1, 100);
    Order order2 = createRestingOrder(1, 50); // Same ID

    EXPECT_TRUE(level.push_back(order1));
    EXPECT_FALSE(level.push_back(order2)); // Duplicate should be rejected

    EXPECT_EQ(level.orderCount(), 1);
    EXPECT_EQ(level.totalQuantity(), 100);
}

// 3. Cancellation Tests
TEST_F(PriceLevelTest, CancelExistingOrder) {
    level.push_back(createRestingOrder(1, 100));
    level.push_back(createRestingOrder(2, 200));

    EXPECT_EQ(level.totalQuantity(), 300);
    EXPECT_EQ(level.orderCount(), 2);

    bool cancelled = level.cancel(1);

    EXPECT_TRUE(cancelled);
    EXPECT_FALSE(level.contains(1));
    EXPECT_TRUE(level.contains(2));
    EXPECT_EQ(level.orderCount(), 1);
    EXPECT_EQ(level.totalQuantity(), 200);
}

TEST_F(PriceLevelTest, CancelNonExistentOrder) {
    level.push_back(createRestingOrder(1, 100));

    bool cancelled = level.cancel(999); // Doesn't exist

    EXPECT_FALSE(cancelled);
    EXPECT_EQ(level.orderCount(), 1);
    EXPECT_EQ(level.totalQuantity(), 100);
}

// 4. Fill Tests (Matching Engine Core Logic)
TEST_F(PriceLevelTest, FillZeroRemainingAggressorReturnsEmpty) {
    level.push_back(createRestingOrder(1, 100));

    Order aggressor = createAggressiveOrder(2, 0); // 0 quantity
    Trades trades = level.fill(aggressor);

    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(level.totalQuantity(), 100); // Nothing changed
}

TEST_F(PriceLevelTest, PartialFillSingleRestingOrder) {
    level.push_back(createRestingOrder(1, 100)); // Resting: 100

    Order aggressor = createAggressiveOrder(2, 40); // Aggressor wants 40
    Trades trades = level.fill(aggressor);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 40);
    EXPECT_EQ(trades[0].passiveId, 1);
    EXPECT_EQ(trades[0].aggressorId, 2);
    EXPECT_EQ(trades[0].price, testPrice);

    // Check states after partial fill
    EXPECT_EQ(aggressor.remaining, 0);     // Aggressor completely filled
    EXPECT_EQ(level.totalQuantity(), 60);  // Resting order has 60 left
    EXPECT_FALSE(level.empty());
    EXPECT_EQ(level.orderCount(), 1);
}

TEST_F(PriceLevelTest, FullFillSingleRestingOrder) {
    level.push_back(createRestingOrder(1, 100)); // Resting: 100

    Order aggressor = createAggressiveOrder(2, 100); // Aggressor matches exact amount
    Trades trades = level.fill(aggressor);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 100);

    // Resting order should be completely wiped out and removed
    EXPECT_TRUE(level.empty());
    EXPECT_EQ(level.orderCount(), 0);
    EXPECT_EQ(level.totalQuantity(), 0);
    EXPECT_FALSE(level.contains(1));
}

TEST_F(PriceLevelTest, AggressorExceedsRestingOrderConsumesMultipleFIFO) {
    // Add multiple orders to test FIFO queue priority
    level.push_back(createRestingOrder(1, 50));  // First in line
    level.push_back(createRestingOrder(2, 100)); // Second in line
    level.push_back(createRestingOrder(3, 200)); // Third in line

    EXPECT_EQ(level.totalQuantity(), 350);

    // Aggressor wants 120 units (should completely wipe order 1 (50), completely wipe order 2 (100 -> takes 70), and partially fill order 3)
    Order aggressor = createAggressiveOrder(4, 120);
    Trades trades = level.fill(aggressor);

    ASSERT_EQ(trades.size(), 2); // Generated two trade execution records (one for order 1, one for order 2)

    // Trade 1: matched against order 1
    EXPECT_EQ(trades[0].passiveId, 1);
    EXPECT_EQ(trades[0].quantity, 50);

    // Trade 2: matched against order 2
    EXPECT_EQ(trades[1].passiveId, 2);
    EXPECT_EQ(trades[1].quantity, 70);

    // Check level state
    EXPECT_FALSE(level.contains(1)); // Removed
    EXPECT_FALSE(level.contains(2)); // Removed
    EXPECT_TRUE(level.contains(3));  // Still exists with 130 remaining (200 - 70)

    EXPECT_EQ(level.orderCount(), 1);
    EXPECT_EQ(level.totalQuantity(), 130);
    EXPECT_EQ(aggressor.remaining, 0); // Aggressor fully satisfied
}

// 5. Lookup & Iterator Tests
TEST_F(PriceLevelTest, FindAndContainsIterators) {
    level.push_back(createRestingOrder(42, 500));

    // Non-const find
    auto it = level.find(42);
    ASSERT_NE(it, level.find(999)); // 999 is missing -> returns level_.end()
    EXPECT_EQ(it->orderId, 42);
    EXPECT_EQ(it->remaining, 500);

    // Const find
    const auto& constLevel = level;
    auto constIt = constLevel.find(42);
    EXPECT_EQ(constIt->orderId, 42);
}