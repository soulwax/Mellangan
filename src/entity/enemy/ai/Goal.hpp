#pragma once

#include <cstdint>

class Enemy;

struct Sense {
    double xd, yd, dist, xn, yn;
    bool seen;
};

enum class GoalState { Running, Done };

class Goal {
public:
    virtual ~Goal() = default;
    virtual void start(Enemy &, const Sense &) const {}
    virtual GoalState tick(Enemy &e, const Sense &s, double &xs, double &ys) const = 0;
    virtual bool urgent(const Enemy &, const Sense &) const { return false; }
    virtual float tell(const Enemy &) const { return 0; }
    virtual bool conceals() const { return false; }
};

struct Plan {
    const Goal *steps[6];
    const Goal *interrupts[2];
};

class GoalQueue {
public:
    const Goal *front() const { return size ? items[head] : nullptr; }
    bool empty() const { return size == 0; }
    void push(const Goal *g) {
        if (size < 8) items[(head + size++) & 7] = g;
    }
    void pushFront(const Goal *g) {
        if (size < 8) head = (head + 7) & 7, items[head] = g, size++;
    }
    void pop() {
        if (size) head = (head + 1) & 7, size--;
    }

private:
    const Goal *items[8]{};
    uint8_t head = 0, size = 0;
};
