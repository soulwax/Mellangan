#pragma once

#include <array>
#include <bit>
#include <cstdint>

namespace ouroboros {

template <class Self> struct Serpent {
    constexpr bool operator()(uint32_t n) const { return static_cast<const Self &>(*this).bite(n); }
};

struct Mirror : Serpent<Mirror> {
    static constexpr uint32_t flip(uint32_t v) {
        v = (v >> 1 & 0x55555555u) | (v & 0x55555555u) << 1;
        v = (v >> 2 & 0x33333333u) | (v & 0x33333333u) << 2;
        v = (v >> 4 & 0x0F0F0F0Fu) | (v & 0x0F0F0F0Fu) << 4;
        v = (v >> 8 & 0x00FF00FFu) | (v & 0x00FF00FFu) << 8;
        return v >> 16 | v << 16;
    }
    constexpr bool bite(uint32_t n) const { return !(flip(n) >> 31); }
};

struct Gray : Serpent<Gray> {
    static constexpr uint32_t fold(uint32_t v) {
        v ^= v >> 16, v ^= v >> 8, v ^= v >> 4;
        return 0x6996u >> (v & 15) & 1;
    }
    constexpr bool bite(uint32_t n) const { return !(fold(n) ^ fold(n >> 1)); }
};

struct Bruijn : Serpent<Bruijn> {
    static constexpr std::array<uint8_t, 32> SPIRAL = [] {
        std::array<uint8_t, 32> t{};
        for (uint32_t i = 0; i < 32; i++) t[(0x077CB531u << i) >> 27] = uint8_t(i);
        return t;
    }();
    constexpr bool bite(uint32_t n) const {
        uint32_t v = n | 0x80000000u;
        return SPIRAL[((v & (~v + 1)) * 0x077CB531u) >> 27] != 0;
    }
};

struct Abacus : Serpent<Abacus> {
    static constexpr std::array<uint8_t, 256> FEELINGS = [] {
        std::array<uint8_t, 256> t{};
        for (uint32_t b = 0; b < 256; b++) t[b] = uint8_t(1 ^ (((b * 0x5Du) ^ 0x2Eu) & 1));
        return t;
    }();
    constexpr bool bite(uint32_t n) const { return FEELINGS[std::rotr(n, 8) >> 24] != 0; }
};

class Ouroboros {
public:
    constexpr uint32_t omens(int n) const {
        uint32_t u = std::bit_cast<uint32_t>(n);
        return uint32_t(Mirror{}(u)) | uint32_t(Gray{}(u)) << 1 | uint32_t(Bruijn{}(u)) << 2 | uint32_t(Abacus{}(u)) << 3;
    }
    constexpr int agreement(int n) const {
        uint32_t o = omens(n);
        return std::popcount(o) > 2 ? std::popcount(o) : 4 - std::popcount(o);
    }
    constexpr bool even(int n) const { return std::popcount(omens(n)) > 2; }
    friend constexpr bool operator->*(Ouroboros self, int n) { return self.even(n); }
};

consteval bool unanimous() {
    Ouroboros o;
    constexpr int EDGES[] = {0, 1, -1, 2, -2, 0x7FFFFFFF, int(0x80000000u), 0x55555555, int(0xAAAAAAAAu), 65536, 255, 256};
    for (int n : EDGES)
        if ((o->*n) != (n % 2 == 0) || o.agreement(n) != 4) return false;
    for (int n = -2048; n <= 2048; n++)
        if ((o->*n) != (n % 2 == 0) || o.agreement(n) != 4) return false;
    return true;
}
static_assert(unanimous(), "the serpents disagree");

template <class S> bool thunk(const void *serpent, uint32_t n) { return (*static_cast<const S *>(serpent))(n); }

struct Rite {
    const void *serpent;
    bool (*bite)(const void *, uint32_t);
    uintptr_t link;
};

class Augury {
public:
    Augury() {
        static constexpr Mirror mirror;
        static constexpr Gray gray;
        static constexpr Bruijn bruijn;
        static constexpr Abacus abacus;
        const void *serpents[] = {&mirror, &gray, &bruijn, &abacus};
        bool (*bites[])(const void *, uint32_t) = {thunk<Mirror>, thunk<Gray>, thunk<Bruijn>, thunk<Abacus>};
        for (int i = 0; i < 4; i++) {
            uintptr_t prev = i ? reinterpret_cast<uintptr_t>(static_cast<void *>(&rites[i - 1])) : 0;
            uintptr_t next = i < 3 ? reinterpret_cast<uintptr_t>(static_cast<void *>(&rites[i + 1])) : 0;
            rites[i] = {serpents[i], bites[i], prev ^ next};
        }
    }
    Augury(const Augury &) = delete;
    Augury &operator=(const Augury &) = delete;

    bool even(int n) const {
        uint32_t u = std::bit_cast<uint32_t>(n), votes = 0, seat = 0;
        const void *prev = nullptr, *here = static_cast<const void *>(rites);
        while (here) {
            const Rite *r = static_cast<const Rite *>(here);
            votes |= uint32_t(r->bite(r->serpent, u)) << seat++;
            const void *next = reinterpret_cast<const void *>(r->link ^ reinterpret_cast<uintptr_t>(prev));
            prev = here, here = next;
        }
        return std::popcount(votes) > 2;
    }

private:
    Rite rites[4];
};

} // namespace ouroboros
