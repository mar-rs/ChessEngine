#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "bots/ghess3.h"
#include "hash.h"
#include "strutture.h"
#include "utente.h"

Scacchiera posizione_iniziale() {
    Scacchiera scacchiera;
    constexpr Pezzo posizione[64] = {
        'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
        'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
        'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
        'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'
    };

    std::copy(std::begin(posizione), std::end(posizione), scacchiera.caselle.begin());
    scacchiera.dirittiArrocco = ARROCCO_CORTO_BIANCO | ARROCCO_LUNGO_BIANCO | ARROCCO_CORTO_NERO | ARROCCO_LUNGO_NERO;
    scacchiera.storicoHash.push_back(hash::calcola(scacchiera));
    return scacchiera;
}

void stampa_scacchiera(const Scacchiera& scacchiera) {
    std::cout << '\n';
    for (int r = 0; r < 8; ++r) {
        std::cout << 8 - r << "   ";
        for (int c = 0; c < 8; ++c) {
            const Pezzo pezzo = scacchiera.caselle[casella(r, c)];
            std::cout << (pezzo == NESSUN_PEZZO ? '.' : pezzo) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "\n    a b c d e f g h\n\n";
}

bool partita_finita(Scacchiera& scacchiera) {
    const uint64_t hashCorrente = hash::calcola(scacchiera);
    if (hash::triplice_ripetizione(scacchiera.storicoHash, hashCorrente)) {
        std::cout << "Patta per ripetizione\n";
        return true;
    }

    ListaMosse mosse;
    trova_mosse(scacchiera, mosse);
    if (!mosse.empty())
        return false;

    if (re_sottoscacco(scacchiera, scacchiera.turno))
        std::cout << "Matto vince il " << (scacchiera.turno ? "nero" : "bianco") << "\n";
    else
        std::cout << "Patta per stallo\n";
    return true;
}

int main() {
    Scacchiera scacchiera = posizione_iniziale();
    std::vector<Scacchiera> precedenti;

    stampa_scacchiera(scacchiera);

    while (!partita_finita(scacchiera)) {
        RisultatoRicerca risultato = scacchiera.turno ? utente::prossima(scacchiera) : ghess3::prossima(scacchiera);

        if (!risultato.haMossa) {
            if (std::cin.eof())
                break;

            precedenti.pop_back();
            scacchiera = precedenti.back();
            precedenti.pop_back();
            stampa_scacchiera(scacchiera);
            continue;
        }
        precedenti.push_back(scacchiera);

        StatoUnMossa stato;
        applica_mossa(scacchiera, risultato.migliore, stato);
        scacchiera.storicoHash.push_back(hash::calcola(scacchiera));

        stampa_scacchiera(scacchiera);
    }

    return 0;
}
