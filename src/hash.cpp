#include "hash.h"

#include <array>
#include <cstddef>

namespace hash {

constexpr std::size_t PEZZI = 12 * 64;
constexpr std::size_t TURNO = PEZZI;
constexpr std::size_t ARROCCO = TURNO + 1;
constexpr std::size_t EN_PASSANT = ARROCCO + 16;
constexpr std::size_t NUMERO_CHIAVI = EN_PASSANT + 65;

constexpr uint64_t splitmix64(uint64_t valore) {
    valore += 0x9e3779b97f4a7c15ULL;
    valore = (valore ^ (valore >> 30)) * 0xbf58476d1ce4e5b9ULL;
    valore = (valore ^ (valore >> 27)) * 0x94d049bb133111ebULL;
    return valore ^ (valore >> 31);
}

constexpr auto crea_chiavi() {
    std::array<uint64_t, NUMERO_CHIAVI> risultato {};
    uint64_t seme = 0x676865737333ULL;
    for (uint64_t& chiave : risultato) {
        seme = splitmix64(seme);
        chiave = seme;
    }
    return risultato;
}

constexpr auto CHIAVI = crea_chiavi();

constexpr int indice(Pezzo pezzo) {
    switch (pezzo) {
    case 'P': return 0;
    case 'N': return 1;
    case 'B': return 2;
    case 'R': return 3;
    case 'Q': return 4;
    case 'K': return 5;
    case 'p': return 6;
    case 'n': return 7;
    case 'b': return 8;
    case 'r': return 9;
    case 'q': return 10;
    case 'k': return 11;
    default: return -1;
    }
}

constexpr uint64_t chiave(Pezzo pezzo, Casella casellaPezzo) {
    const int i = indice(pezzo);
    return i < 0 ? 0 : CHIAVI[i * 64 + casellaPezzo];
}

constexpr uint64_t chiave_arrocco(uint8_t diritti) {
    return CHIAVI[ARROCCO + (diritti & 0x0f)];
}

constexpr uint64_t chiave_en_passant(Casella casellaEnPassant) {
    return CHIAVI[EN_PASSANT + (casella_valida(casellaEnPassant) ? casellaEnPassant : 64)];
}

uint64_t calcola(const Scacchiera& scacchiera) {
    uint64_t hash = 0;
    for (Casella casellaPezzo = 0; casellaPezzo < 64; ++casellaPezzo)
        hash ^= chiave(scacchiera.caselle[casellaPezzo], casellaPezzo);

    if (!scacchiera.turno)
        hash ^= CHIAVI[TURNO];
    return hash ^ chiave_arrocco(scacchiera.dirittiArrocco) ^ chiave_en_passant(scacchiera.enpassant);
}

uint64_t dopo_mossa(uint64_t hash, const Scacchiera& scacchiera, const Mossa& mossa, const StatoUnMossa& stato) {
    const bool bianco = !scacchiera.turno;
    const Pezzo pezzo = mossa.promozione == NESSUN_PEZZO ? scacchiera.caselle[mossa.a] : (bianco ? 'P' : 'p');

    hash ^= chiave(pezzo, mossa.da);
    hash ^= chiave(scacchiera.caselle[mossa.a], mossa.a);

    if (stato.cattura != NESSUN_PEZZO) {
        const bool enPassant = (pezzo == 'P' || pezzo == 'p') && mossa.a == stato.enpassant && colonna(mossa.da) != colonna(mossa.a);
        const Casella catturata = enPassant ? mossa.a + (bianco ? 8 : -8) : mossa.a;
        hash ^= chiave(stato.cattura, catturata);
    }

    const int spostamento = mossa.a - mossa.da;
    if ((pezzo == 'K' || pezzo == 'k') && (spostamento == 2 || spostamento == -2)) {
        const bool corto = spostamento > 0;
        const Casella torreDa = mossa.da + (corto ? 3 : -4);
        const Casella torreA = mossa.da + (corto ? 1 : -1);
        const Pezzo torre = bianco ? 'R' : 'r';
        hash ^= chiave(torre, torreDa) ^ chiave(torre, torreA);
    }

    return hash ^ chiave_arrocco(stato.dirittiArrocco) ^ chiave_arrocco(scacchiera.dirittiArrocco) ^ chiave_en_passant(stato.enpassant) ^ chiave_en_passant(scacchiera.enpassant) ^ CHIAVI[TURNO];
}

bool triplice_ripetizione(const std::vector<uint64_t>& storico, uint64_t hash) {
    int occorrenze = 0;
    for (std::size_t i = storico.size(); i > 0;) {
        --i;
        if (storico[i] == hash && ++occorrenze == 3)
            return true;
        if (i < 2)
            break;
        --i;
    }
    return false;
}

}
