#include "level/biome/ExpansionBiome.hpp"

#include <array>

#include "level/World.hpp"
#include "level/biome/Carpet.hpp"

namespace {
const char *const NAMES[] = {"SALT FLATS", "FLOODPLAIN", "MOONLIT GROVE",
    "SUNKEN RUINS", "EMBER FEN", "AMBER WOODS", "STORM STEPPE",
    "GLASS DESERT", "HOLLOW CAVERNS", "STARFALL BASIN"};
const Color COLORS[] = {{0.8f,0.8f,0.7f,1},{0.3f,0.65f,0.6f,1},
    {0.4f,0.4f,0.7f,1},{0.3f,0.55f,0.65f,1},{0.65f,0.4f,0.2f,1},
    {0.8f,0.55f,0.15f,1},{0.5f,0.65f,0.55f,1},{0.7f,0.65f,0.8f,1},
    {0.3f,0.25f,0.4f,1},{0.4f,0.45f,0.85f,1}};

const std::array<Feature,30> &props() {
  static const auto value = [] {
    std::array<Feature,30> out{};
    for (int i=0;i<30;i++)
      out[i] = {NAMES[i/3],placeStack,i,-1,false,0.7f,
                i%3==0 ? 3.0f : 0.0f,
                i%3==2 ? COLORS[i/3]*0.2f : Color{0,0,0,0},
                false,atlas::biomeProp};
    return out;
  }();
  return value;
}

BiomeData data(int index) {
  BiomeData d{};
  d.name=NAMES[index];
  d.debug=COLORS[index];
  d.tint={1,1,1,1};
  d.dirt=0.12f;
  d.structureChance=65;
  for (int i=0;i<int(StructureKind::Count);i++)
    d.structureWeights[i]=10;
  d.structureWeights[index%int(StructureKind::Count)]=45;
  d.foes=foe(EnemyKind(int(EnemyKind::MireLeech)+index)) |
         foe(EnemyKind(int(EnemyKind::MireLeech)+(index+1)%10)) |
         foe(EnemyKind::Bat) | foe(EnemyKind::Slime);
  for (int i=0;i<3;i++)
    d.spawns[i]={&props()[index*3+i],10+i*4,20+i*10};
  d.fringe=&props()[index*3+2];
  return d;
}

bool own(int x,int y,BiomeId id) { return world::biomeAt(x,y)==id; }
}

ExpansionBiome::ExpansionBiome(int index) : Biome(data(index)),index(index) {}

bool ExpansionBiome::ground(int x,int y,Sprite *out) const {
  BiomeId id=BiomeId(BIOME_SALT+index);
  if (!blockOf(own,id,x,y))
    return false;
  int mask=int(blockOf(own,id,x,y-1)) | blockOf(own,id,x,y+1)<<1 |
           blockOf(own,id,x-1,y)<<2 | blockOf(own,id,x+1,y)<<3;
  int cell=0;
  if (mask!=15) {
    int o=world::rim(mask);
    cell=3+o/12*3+o%12;
  } else {
    int r=hash2(x,y)%100;
    cell=r<12 ? 1 : r<18 ? 2 : 0;
  }
  *out=atlas::biomeGround(index,cell);
  return true;
}

const Biome *expansionBiome(int index) {
  static const ExpansionBiome value[]={ExpansionBiome(0),ExpansionBiome(1),
      ExpansionBiome(2),ExpansionBiome(3),ExpansionBiome(4),ExpansionBiome(5),
      ExpansionBiome(6),ExpansionBiome(7),ExpansionBiome(8),ExpansionBiome(9)};
  return &value[index];
}
