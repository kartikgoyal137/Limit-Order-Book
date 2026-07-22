import random

NUM_ORDERS = 200000
CENTER_PRICE = 1000
SPREAD = 50
OUTPUT = "orders.txt"

def main():
    random.seed(42)
    next_id = 1
    active_orders = {}
    lines = []

    for i in range(2500):
        price = CENTER_PRICE - random.randint(1, SPREAD)
        lines.append(f"Add {next_id} 1 {random.randint(10, 500)} {price}")
        active_orders[next_id] = (True, price)
        next_id += 1

    for i in range(2500):
        price = CENTER_PRICE + random.randint(1, SPREAD)
        lines.append(f"Add {next_id} 0 {random.randint(10, 500)} {price}")
        active_orders[next_id] = (False, price)
        next_id += 1

    for _ in range(NUM_ORDERS):
        r = random.random()

        if r < 0.50:
            isBuy = random.random() < 0.5
            if isBuy:
                price = CENTER_PRICE - random.randint(0, SPREAD)
            else:
                price = CENTER_PRICE + random.randint(0, SPREAD)
            shares = random.randint(1, 200)
            lines.append(f"Add {next_id} {int(isBuy)} {shares} {price}")
            active_orders[next_id] = (isBuy, price)
            next_id += 1

        elif r < 0.80 and active_orders:
            oid = random.choice(list(active_orders.keys()))
            lines.append(f"Cancel {oid}")
            del active_orders[oid]

        elif active_orders:
            oid = random.choice(list(active_orders.keys()))
            isBuy, old_price = active_orders[oid]
            new_price = old_price + random.randint(-5, 5)
            if new_price <= 0:
                new_price = 1
            new_shares = random.randint(1, 200)
            lines.append(f"Modify {oid} {new_shares} {new_price}")
            active_orders[oid] = (isBuy, new_price)

    with open(OUTPUT, "w") as f:
        f.write("\n".join(lines) + "\n")

    print(f"Generated {len(lines):,} orders to {OUTPUT}")

if __name__ == "__main__":
    main()
