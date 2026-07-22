# Limit Order Book

A low-latency limit order book and matching engine in C++20, designed to demonstrate systems programming concepts used in high-frequency trading.

## Key Design Decisions

### Architecture

```
┌──────────────┐     lock-free SPSC queue     ┌──────────────────┐
│   I/O Thread │ ──────────────────────────▶   │  Matching Thread  │
│  (producer)  │                               │   (consumer)      │
│  parse input │                               │   OrderBook       │
└──────────────┘                               └──────────────────┘
```

The I/O thread parses incoming orders and pushes `OrderMessage` structs into a **lock-free single-producer single-consumer (SPSC) ring buffer**. The matching thread pops messages and processes them against the `OrderBook`. This separates I/O parsing latency from matching latency — the matching engine never blocks on I/O.

The matching engine itself is **single-threaded by design**: price-time priority matching is inherently sequential. Concurrency belongs in the I/O pipeline, not in the matching loop.

### Data Structures

```
OrderBook
├── bids_ (AVLTree)     ── best bid = tree max
│   └── PriceLevel(50)  ── Order ⇄ Order ⇄ Order  (doubly-linked list, FIFO)
│   └── PriceLevel(49)  ── Order ⇄ Order
├── asks_ (AVLTree)     ── best ask = tree min
│   └── PriceLevel(51)  ── Order ⇄ Order
│   └── PriceLevel(52)  ── Order
├── orders_    (hash map: order ID → Order*)
├── bidLevels_ (hash map: price → PriceLevel*)
└── askLevels_ (hash map: price → PriceLevel*)
```

- **AVL Tree** of `PriceLevel` nodes, sorted by price. Height stored in each node for O(1) balance factor computation. One reusable `AVLTree` class for both sides.
- **Doubly-linked list** of `Order` nodes at each price level, maintaining FIFO (time priority).
- **Hash maps** for O(1) lookup by order ID and price.

| Operation       | Complexity                            |
|-----------------|---------------------------------------|
| Add order       | O(log M) first at a price, O(1) after |
| Cancel order    | O(1)                                  |
| Execute (match) | O(1) per fill                         |
| Best bid/ask    | O(1)                                  |

Where M = number of distinct price levels (typically << N total orders).

### Low-Latency Techniques

- **Object Pool** (`ObjectPool.hpp`): Pre-allocates memory in blocks and uses a free-list. `allocate()` and `deallocate()` are O(1) pointer swaps — zero `malloc`/`free` calls on the hot path.
- **SPSC Queue** (`SPSCQueue.hpp`): Lock-free ring buffer using `std::atomic` with `acquire`/`release` memory ordering. `head_` and `tail_` are on separate cache lines (`alignas(64)`) to prevent **false sharing** between producer and consumer cores.
- **Stored AVL height**: Each `PriceLevel` stores its subtree height, updated in O(1) during rotations. Balance factor is a single subtraction, not a recursive tree walk.

## Project Structure

```
├── src/
│   ├── ObjectPool.hpp       # Arena allocator with free-list
│   ├── SPSCQueue.hpp        # Lock-free SPSC ring buffer
│   ├── Order.hpp            # Order struct
│   ├── PriceLevel.hpp/cpp   # AVL node + order linked list
│   ├── AVLTree.hpp/cpp      # Self-balancing AVL tree
│   ├── OrderBook.hpp/cpp    # Limit order book (buy/sell trees + maps + pools)
│   ├── MatchingEngine.hpp/cpp # Producer-consumer engine
│   └── main.cpp             # Benchmark driver
├── test/
│   ├── Tests.cpp            # Unit + integration tests (GoogleTest)
│   └── CMakeLists.txt
├── CMakeLists.txt
└── README.md
```

## Supported Order Types

- **Limit Order** (Add, Cancel, Modify) — rests on the book at a given price.
- **Market Order** — executes immediately against the best available price(s).
- **Market-Limit Order** — a limit order that crosses the spread executes immediately; any remainder rests on the book.

## Build & Test

Requires CMake 3.16+ and a C++20 compiler.

```bash
# Clone (with GoogleTest)
git clone --recurse-submodules <repo-url>
cd Limit-Order-Book

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run tests
./test/tests

# Run benchmark (provide an order file)
./LimitOrderBook orders.txt
```

### Order File Format

One order per line:

```
Add <id> <isBuy:0|1> <shares> <price>
Cancel <id>
Modify <id> <newShares> <newPrice>
Market <id> <isBuy:0|1> <shares>
```

Example:
```
Add 1 1 100 50
Add 2 0 200 55
Market 3 1 150
Cancel 2
Modify 1 50 48
```

## References

- [How to Build a Fast Limit Order Book — WK Selph](https://web.archive.org/web/20110219163448/http://howtohft.wordpress.com/2011/02/15/how-to-build-a-fast-limit-order-book/)
