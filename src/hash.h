#pragma once

#include "strutture.h"

#include <cstdint>
#include <vector>

namespace hash {

uint64_t calcola(const Scacchiera& scacchiera);
uint64_t dopo_mossa(uint64_t hashPrecedente, const Scacchiera& scacchiera, const Mossa& mossa, const StatoUnMossa& stato);
bool triplice_ripetizione(const std::vector<uint64_t>& storico, uint64_t hash);

}
