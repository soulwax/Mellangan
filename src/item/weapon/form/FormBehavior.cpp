#include "item/weapon/form/FormBehavior.hpp"

#include "gfx/Screen.hpp"
#include "item/weapon/form/Forms.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"

static_assert(int(Form::Spiral) == 14, "every Form needs a FormBehavior");

const FormBehavior &FormBehavior::of(Form form) {
  static const BoltForm bolt;
  static const VolleyForm volley;
  static const OrbitForm orbit;
  static const NovaForm nova;
  static const RainForm rain;
  static const FieldForm field;
  static const SeekerForm seeker;
  static const CleaveForm cleave;
  static const BoomerangForm boomerang;
  static const LobForm lob;
  static const TrapForm trap;
  static const BeamForm beam;
  static const WhipForm whip;
  static const WallForm wall;
  static const SpiralForm spiral;
  static const FormBehavior *const all[] = {&bolt,  &volley, &orbit,     &nova, &rain,
                                            &field, &seeker, &cleave,    &boomerang,
                                            &lob,   &trap,   &beam,      &whip, &wall,
                                            &spiral};
  return *all[int(form)];
}

RecipeShot *FormCast::shot(double x, double y, double a, int i) const {
  RecipeShot *s = p.level->add<RecipeShot>(r, x, y, std::cos(a), std::sin(a),
                                           damage, reach, i, count);
  s->source = source;
  return s;
}

void FormBehavior::light(RecipeShot &s, Screen &screen) const {
  screen.glow(s.xp, s.yp, 30, s.r.tint * 0.12f);
}

void FormBehavior::glint(RecipeShot &s, Screen &screen, float rot) {
  float k = s.child ? 0.7f : 1.0f;
  screen.color(s.r.tint.alpha(0.35f));
  screen.center(atlas::Glow, s.xp, s.yp, 14 * k / atlas::Glow.w);
  screen.color(col::white);
  screen.particle(s.r.particle, s.xp, s.yp, s.r.tint, k, rot);
}

void FormBehavior::wake(RecipeShot &s) {
  if (!rng.chance(0.4f) || !Drift::room())
    return;
  Drift *d = s.level->add<Drift>(s.x, s.y, -s.dx * 20 + rng.range(-6, 6),
                                 -s.dy * 20 + rng.range(-6, 6), s.r.particle,
                                 s.r.tint.alpha(0.6f), rng.range(0.2f, 0.35f));
  d->scale = s.child ? 0.4f : 0.6f, d->spin = rng.range(-4, 4);
}
