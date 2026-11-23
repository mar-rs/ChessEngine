#include "utente.h"

#include <cctype>
#include <iostream>
#include <string>

namespace utente {

Casella leggi_casella(const std::string& testo) {
    const char file = std::tolower(testo[0]);
    return casella(8 - (testo[1] - '0'), file - 'a');
}

std::string minuscolo(std::string testo) {
    for (char& carattere : testo)
        carattere = std::tolower(carattere);
    return testo;
}

Pezzo leggi_promozione(const std::string& testo, bool bianco) {
    const char pezzo = std::tolower(testo[0]);
    return bianco ? std::toupper(pezzo) : pezzo;
}

RisultatoRicerca prossima(Scacchiera& scacchiera) {
    std::cout << "mossa: ";
    std::string primo;
    if (!(std::cin >> primo))
        return {};

    const std::string comando = minuscolo(primo);
    if (comando == "undo")
        return {};

    const bool lungo = comando == "ooo";
    if (comando == "oo" || lungo) {
        const int rigaRe = scacchiera.turno ? 7 : 0;
        const Casella da = casella(rigaRe, 4);
        const Casella a = casella(rigaRe, lungo ? 2 : 6);
        RisultatoRicerca risultato;
        risultato.migliore = {da, a, NESSUN_PEZZO, 0};
        risultato.haMossa = true;
        return risultato;
    }

    std::string secondo;
    if (!(std::cin >> secondo))
        return {};

    const Casella da = leggi_casella(primo);
    const Casella a = leggi_casella(secondo);
    const Pezzo pezzo = scacchiera.caselle[da];
    const bool promuove = (pezzo == 'P' && riga(a) == 0) || (pezzo == 'p' && riga(a) == 7);
    Pezzo promozione = NESSUN_PEZZO;
    if (promuove) {
        std::string promozioneTesto;
        if (!(std::cin >> promozioneTesto))
            return {};
        promozione = leggi_promozione(promozioneTesto, scacchiera.turno);
    }

    RisultatoRicerca risultato;
    risultato.migliore = {da, a, promozione, 0};
    risultato.haMossa = true;
    return risultato;
}

}
