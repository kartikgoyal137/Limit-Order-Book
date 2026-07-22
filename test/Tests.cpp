#include <gtest/gtest.h>
#include "OrderBook.hpp"
#include "SPSCQueue.hpp"
#include "ObjectPool.hpp"
#include "MatchingEngine.hpp"
#include <thread>

TEST(OrderBook, AddAndBestBidAsk) {
    OrderBook book;
    book.addOrder(1, true,  100, 50);
    book.addOrder(2, false, 200, 55);

    ASSERT_NE(book.bestBid(), nullptr);
    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestBid()->price, 50);
    EXPECT_EQ(book.bestAsk()->price, 55);
}

TEST(OrderBook, CancelOrder) {
    OrderBook book;
    book.addOrder(1, true, 100, 50);
    EXPECT_NE(book.bestBid(), nullptr);

    book.cancelOrder(1);
    EXPECT_EQ(book.bestBid(), nullptr);
}

TEST(OrderBook, LimitOrderCrossesSpread) {
    OrderBook book;
    book.addOrder(1, false, 100, 50);
    book.addOrder(2, true,  60,  50);

    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestAsk()->totalVolume, 40);
    EXPECT_EQ(book.bestBid(), nullptr);
}

TEST(OrderBook, PriceTimePriority) {
    OrderBook book;
    book.addOrder(1, false, 100, 50);
    book.addOrder(2, false, 100, 51);
    book.addOrder(3, false,  50, 50);

    book.addOrder(4, true, 150, 50);

    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestAsk()->price, 51);
    EXPECT_EQ(book.bestAsk()->totalVolume, 100);
    EXPECT_EQ(book.bestBid(), nullptr);
}

TEST(OrderBook, MultiplePriceLevels) {
    OrderBook book;
    for (int i = 1; i <= 10; i++)
        book.addOrder(i, true, 100, 40 + i);
    for (int i = 11; i <= 20; i++)
        book.addOrder(i, false, 100, 50 + i - 10);

    EXPECT_EQ(book.bestBid()->price, 50);
    EXPECT_EQ(book.bestAsk()->price, 51);
}

TEST(OrderBook, ModifyOrder) {
    OrderBook book;
    book.addOrder(1, true, 100, 50);
    book.modifyOrder(1, 200, 55);

    EXPECT_EQ(book.bestBid()->price, 55);
    EXPECT_EQ(book.bestBid()->totalVolume, 200);
    EXPECT_EQ(book.findLevel(50, true), nullptr);
}

TEST(OrderBook, LimitOrderCrossesMultipleLevels) {
    OrderBook book;
    book.addOrder(1, false, 50, 100);
    book.addOrder(2, false, 50, 101);
    book.addOrder(3, false, 50, 102);

    book.addOrder(4, true, 120, 102);

    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestAsk()->price, 102);
    EXPECT_EQ(book.bestAsk()->totalVolume, 30);
}

TEST(OrderBook, CancelNonexistent) {
    OrderBook book;
    book.cancelOrder(999);
}

TEST(OrderBook, AVLTreeBalance) {
    OrderBook book;
    for (int i = 1; i <= 100; i++)
        book.addOrder(i, true, 10, i);

    EXPECT_EQ(book.bestBid()->price, 100);

    for (int i = 40; i <= 60; i++)
        book.cancelOrder(i);

    EXPECT_EQ(book.bestBid()->price, 100);

    book.cancelOrder(100);
    EXPECT_EQ(book.bestBid()->price, 99);
}

TEST(SPSCQueue, PushPop) {
    SPSCQueue<int, 16> q;
    EXPECT_TRUE(q.push(42));
    int val;
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 42);
}

TEST(SPSCQueue, Empty) {
    SPSCQueue<int, 16> q;
    int val;
    EXPECT_FALSE(q.pop(val));
}

TEST(SPSCQueue, Full) {
    SPSCQueue<int, 4> q;
    EXPECT_TRUE(q.push(1));
    EXPECT_TRUE(q.push(2));
    EXPECT_TRUE(q.push(3));
    EXPECT_FALSE(q.push(4));
}

TEST(SPSCQueue, FIFO) {
    SPSCQueue<int, 64> q;
    for (int i = 0; i < 32; i++) q.push(i);
    for (int i = 0; i < 32; i++) {
        int val;
        EXPECT_TRUE(q.pop(val));
        EXPECT_EQ(val, i);
    }
}

TEST(SPSCQueue, Wraparound) {
    SPSCQueue<int, 4> q;
    for (int round = 0; round < 10; round++) {
        EXPECT_TRUE(q.push(round));
        int val;
        EXPECT_TRUE(q.pop(val));
        EXPECT_EQ(val, round);
    }
}

TEST(SPSCQueue, ConcurrentUse) {
    SPSCQueue<int, 1024> q;
    constexpr int N = 100000;

    std::thread producer([&]() {
        for (int i = 0; i < N; i++)
            while (!q.push(i)) {}
    });

    std::thread consumer([&]() {
        for (int i = 0; i < N; i++) {
            int val;
            while (!q.pop(val)) {}
            EXPECT_EQ(val, i);
        }
    });

    producer.join();
    consumer.join();
}

TEST(ObjectPool, AllocateDeallocate) {
    struct Foo { int x; int y; };
    ObjectPool<Foo> pool;

    Foo* a = pool.allocate(Foo{1, 2});
    EXPECT_EQ(a->x, 1);
    EXPECT_EQ(a->y, 2);
    pool.deallocate(a);

    Foo* b = pool.allocate(Foo{3, 4});
    EXPECT_EQ(a, b);
    EXPECT_EQ(b->x, 3);
    pool.deallocate(b);
}

TEST(ObjectPool, ManyAllocations) {
    struct Obj { int64_t val; };
    ObjectPool<Obj, 8> pool;
    Obj* ptrs[100];
    for (int i = 0; i < 100; i++)
        ptrs[i] = pool.allocate(Obj{i});
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(ptrs[i]->val, i);
        pool.deallocate(ptrs[i]);
    }
}

TEST(MatchingEngine, EndToEnd) {
    MatchingEngine engine;
    std::thread matching([&]() { engine.run(); });

    engine.submit({OrderMessage::Type::Add, 1, false, 100, 50});
    engine.submit({OrderMessage::Type::Add, 2, false, 100, 51});
    engine.submit({OrderMessage::Type::Add, 3, true, 150, 51});
    engine.submit({OrderMessage::Type::Add, 4, true, 200, 48});
    engine.submit({OrderMessage::Type::Cancel, 4, false, 0, 0});

    engine.stop();
    matching.join();

    EXPECT_EQ(engine.book().bestBid(), nullptr);
    ASSERT_NE(engine.book().bestAsk(), nullptr);
    EXPECT_EQ(engine.book().bestAsk()->price, 51);
    EXPECT_EQ(engine.book().bestAsk()->totalVolume, 50);
}
