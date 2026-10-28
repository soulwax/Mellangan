#include "menu/ShrineMenu.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {
const char *const LABELS[]={"FURY: +20% damage / -10 maximum health",
    "ROOTS: +30 maximum health / -6 movement speed",
    "VISION: gain a level / -1 armour", "KEEP THE FLAME: no blessing or cost"};
SDL_FRect card(const Screen &s,int i) {
  float w=std::min(s.w-32*s.u,440*s.u),h=42*s.u;
  return {(s.w-w)/2,s.h*0.3f+i*(h+8*s.u),w,h};
}
}

void ShrineMenu::choose(int choice) {
  auto &n=game.level->night;
  if (!n.offering)
    return;
  Player &p=*game.level->player;
  if (choice==0 && p.maxHp>20) {
    p.might+=0.2f;
    p.maxHp-=10;
    p.hp=std::min(p.hp,p.maxHp);
  } else if (choice==1 && p.speed>25) {
    p.maxHp+=30;
    p.hp=std::min(p.maxHp,p.hp+30);
    p.speed-=6;
  } else if (choice==2) {
    p.pendingLevels++;
    p.armor-=1;
  } else if (choice!=3) {
    return;
  }
  n.offering=false;
  game.setMenu(nullptr);
}

void ShrineMenu::tick() {
  Menu::tick();
  auto &in=game.input;
  if (in.escape.clicked)
    return choose(3);
  if (in.up.clicked)
    selected=(selected+3)%4;
  if (in.down.clicked)
    selected=(selected+1)%4;
  Key *keys[]={&in.one,&in.two,&in.three,&in.four};
  for (int i=0;i<4;i++) {
    if (keys[i]->clicked)
      return choose(i);
    auto r=card(game.screen,i);
    SDL_FPoint p{in.mouseX,in.mouseY};
    if (SDL_PointInRectFloat(&p,&r)) {
      if (in.mouseMoved)
        selected=i;
      if (in.mouseClicked && t>0.25f)
        return choose(i);
    }
  }
  if (in.confirm.clicked && t>0.25f)
    choose(selected);
}

void ShrineMenu::render(Screen &s) {
  s.rect(0,0,s.w,s.h,{0.02f,0.02f,0.05f,0.93f});
  float small=std::max(1.0f,s.u-1);
  s.textCenter("THE SHRINE OFFERS A BARGAIN",s.w/2,s.h*0.19f,2*s.u,col::gold);
  for (int i=0;i<4;i++) {
    auto r=card(s,i);
    s.rect(r.x,r.y,r.w,r.h,i==selected ? Color{0.2f,0.16f,0.1f,1} : Color{0.08f,0.08f,0.12f,1});
    s.textCenter(fmt::format("{}. {}",i+1,LABELS[i]),s.w/2,r.y+14*s.u,small,
                 i==selected ? col::gold : col::white);
  }
  s.textCenter("1-4 choose / arrows + Enter / Esc keep the flame",s.w/2,s.h*0.88f,small,col::muted);
}
