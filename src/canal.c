#include "canal.h"
#include "typedefs.h"

const Canal CANALS[CANALS_COUNT] = {"Panama", {0, 1}, {0, 1}, "Suez", {0, 2}, {1, 2}};
inline LandIndex get_canal_land(CanalState canal_idx, LandConnIndex conn_idx) {
  return CANALS[canal_idx].lands[conn_idx];
}
inline SeaIndex get_canal_sea(CanalState canal_idx, SeaConnIndex conn_idx) {
  return CANALS[canal_idx].seas[conn_idx];
}