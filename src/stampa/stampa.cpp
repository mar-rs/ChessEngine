#include "stampa.h"

namespace stampa {

// non rifaccio la mappatura
char inverti_colore(char pezzo) {
    return (tolower(pezzo) == pezzo) ? toupper(pezzo) : tolower(pezzo);
}

void stampa_casella(char pezzo, bool a, bool da, int r, int c) {
    int base = (pezzo == NESSUN_PEZZO) ? ' ' : inverti_colore(pezzo);
    int offsetSfondo = ((r + c) % 2 == 1) ? 0x100 : 0x000;

    int carattere = 0xE000 + base + offsetSfondo;

    char buf[] = {
        static_cast<char>(0xE0 | (carattere >> 12)),
        static_cast<char>(0x80 | ((carattere >> 6) & 0x3F)),
        static_cast<char>(0x80 | (carattere & 0x3F)),
        '\0'
    };

    
    if (a)          std::cout << "\x1b[48;2;92;113;148m" << buf << ' ' << "\x1b[49m";
    else if (da)    std::cout << "\x1b[48;2;68;84;110m" << buf << ' ' << "\x1b[49m";
    else            std::cout << buf << ' ';
}

void stampa_scacchiera(const Scacchiera& scacchiera, const Casella a, const Casella da) {
    std::cout << '\n';
    for (int r = 0; r < 8; ++r) {
        std::cout << 8 - r << "   ";
        for (int c = 0; c < 8; ++c) {
            stampa_casella(scacchiera.caselle[casella(r, c)], a == r * 8 + c, da == r * 8 + c, r, c);
        }
        std::cout << '\n';
    }
    std::cout << "\n    a b c d e f g h\n\n";
}

}