#pragma once

#include "gfx/Screen.hpp"

class Mob;

namespace ability {

void blink(Mob &caster, float reach, float damage, Color tone);
void nova(Mob &caster, float radius, float damage, float heal, Color tone);
void volley(Mob &caster, int count, double spread, float speed, float damage,
            int pierce);
void rally(Mob &caster, float reach, float damage, float guard);
void reap(Mob &caster, float reach, float damage);
void bombs(Mob &caster, int count, float damage, float heal);
void dance(Mob &caster, float reach, float damage, Color tone);
void thunder(Mob &caster, int count, float damage);
void charge(Mob &caster, float reach, float damage);
void miasma(Mob &caster, float radius, float poison, float seconds);
void chain(Mob &caster, int jumps, float reach, float damage);
void grasp(Mob &caster, float radius, float damage);
void smite(Mob &caster, int count, float damage);
void lunge(Mob &caster, float reach, float damage);
void entangle(Mob &caster, float radius, float damage);
void mines(Mob &caster, int count, float damage);
void comet(Mob &caster, int count, float damage);
void keg(Mob &caster, float damage);
void warcry(Mob &caster, float radius, float seconds);
void bandage(Mob &caster, float amount);
void tumble(Mob &caster, float reach);
void whirlwind(Mob &caster, float reach, float damage);
void quake(Mob &caster, float radius, float damage);
void firering(Mob &caster, int count, float damage);
void lodestar(Mob &caster, float radius);
void snare(Mob &caster, float reach, float damage);
void banner(Mob &caster, float radius, float seconds);
void hunt(Mob &caster, int count, float damage);
void barrage(Mob &caster, int count, float damage);
void tempest(Mob &caster, float seconds, float damage);
void drain(Mob &caster, float radius, float damage);
void sanctuary(Mob &caster, float radius, float seconds);
void flash(Mob &caster, int count, float damage);
void overgrowth(Mob &caster, float radius, float seconds);
void eclipse(Mob &caster, float radius, float seconds);
void smoke(Mob &caster, float radius, float seconds);
void rite(Mob &caster, float cost, float might, float seconds);
void well(Mob &caster, float reach, float radius, float damage);
void mirror(Mob &caster, float seconds);
void horn(Mob &caster, float radius, float seconds);
void shatter(Mob &caster, float radius, float damage);
void sight(Mob &caster, float radius);
void sprint(Mob &caster, float seconds);
void prison(Mob &caster, float radius, float seconds, float damage);
void hawk(Mob &caster, float seconds, float damage);
void stand(Mob &caster, float radius, float damage);
void transmute(Mob &caster, float radius, float seconds);
void storm(Mob &caster, float seconds, float damage);
void raise(Mob &caster, int count, float seconds, float damage);
void mercy(Mob &caster, int wards);
void parry(Mob &caster, float window);
void rampart(Mob &caster, float reach, float damage);
void overclock(Mob &caster, float seconds, float jam);
void foresight(Mob &caster, int count);
void cannonade(Mob &caster, int count, float damage);

} // namespace ability
