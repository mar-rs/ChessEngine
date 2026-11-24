#include "ghess3.h"

//#define PROFONDITA_SHIFT 7.f
#define PROFONDITA 4.f

namespace ghess3 {

float bonus_posizione_pedone(Casella posizione, Pezzo pezzo) {
    static constexpr int valori[8][8] = {
        {  0,  0,  0,  0,  0,  0,  0,  0 },
        { 10, 10, 10, 10, 10, 10, 10, 10 },
        {  8,  8,  8,  8,  8,  8,  8,  8 },
        {  5,  5,  5,  5,  5,  5,  5,  5 },
        {  3,  3,  3,  4,  4,  3,  3,  3 },
        {  2,  2,  2,  2,  2,  2,  2,  2 },
        {  1,  1,  1,  1,  1,  1,  1,  1 },
        {  0,  0,  0,  0,  0,  0,  0,  0 },
    };

    const int r = riga(posizione);
    return valori[pezzo == 'P' ? r : 7 - r][colonna(posizione)];
}

float malus_raddoppio_pedone(const Scacchiera& s, Casella posizione, Pezzo pezzo) {
    const int c = colonna(posizione);
    for (int r = 1; r < 7; ++r) {
        if (r != riga(posizione) && s.caselle[casella(r, c)] == pezzo)
            return -10.f;
    }
    return 0.f;
}

float malus_isolamento_pedone(const Scacchiera& s, Casella posizione, Pezzo pezzo) {
    const int r = riga(posizione);
    const int c = colonna(posizione);
    const int rigaSpeciale = pezzo == 'P' ? 4 : 5;

    for (int spostamento = -1; spostamento < 3; ++spostamento) {
        if (r != rigaSpeciale && spostamento == 2)
            break;

        const int rigaVicina = r + spostamento;
        for (const int colonnaVicina : {c - 1, c + 1}) {
            if (dentro_scacchiera(rigaVicina, colonnaVicina) && s.caselle[casella(rigaVicina, colonnaVicina)] == pezzo)
                return 0.f;
        }
    }
    return -10.f;
}

int conta_visibilita(const Scacchiera& s, Casella posizione, const int direzioni[][2], int numeroDirezioni) {
    const bool bianco = pezzo_bianco(s.caselle[posizione]);
    int visibili = 0;

    for (int i = 0; i < numeroDirezioni; ++i) {
        int r = riga(posizione) + direzioni[i][0];
        int c = colonna(posizione) + direzioni[i][1];
        while (dentro_scacchiera(r, c)) {
            const Pezzo occupante = s.caselle[casella(r, c)];
            if (occupante == NESSUN_PEZZO) {
                ++visibili;
            } else {
                if (pezzo_nemico(occupante, bianco))
                    ++visibili;
                break;
            }
            r += direzioni[i][0];
            c += direzioni[i][1];
        }
    }
    return visibili;
}

float bonus_visibilita_alfiere(const Scacchiera& s, Casella posizione) {
    static constexpr int direzioni[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    return conta_visibilita(s, posizione, direzioni, 4) * 10.f / 13.f;
}

float bonus_visibilita_torre(const Scacchiera& s, Casella posizione) {
    static constexpr int direzioni[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    return conta_visibilita(s, posizione, direzioni, 4) * 10.f / 14.f;
}

float bonus_visibilita_regina(const Scacchiera& s, Casella posizione) {
    return (bonus_visibilita_torre(s, posizione) + bonus_visibilita_alfiere(s, posizione)) / 2.f;
}

float bonus_visibilita_cavallo(const Scacchiera& s, Casella posizione) {
    static constexpr int salti[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    const bool bianco = pezzo_bianco(s.caselle[posizione]);
    int visibili = 0;

    for (const auto& salto : salti) {
        const int r = riga(posizione) + salto[0];
        const int c = colonna(posizione) + salto[1];
        if (!dentro_scacchiera(r, c))
            continue;

        const Pezzo occupante = s.caselle[casella(r, c)];
        if (occupante == NESSUN_PEZZO || pezzo_nemico(occupante, bianco))
            ++visibili;
    }
    return visibili * 10.f / 8.f;
}

float interpola_valore(const Scacchiera& s, float vmid, float vend) {
    if (vmid == vend) return vmid;
    float x = 0;
    for (int i = 0; i < 64; ++i)
        if (s.caselle.at(i) != NESSUN_PEZZO)
            ++x;
    // valori geogebra
    return (vend - vmid) / (1 + std::pow(1.7f, -x + 18.f)) + vmid;
}

float analizza(const Scacchiera& s) {
    float risultato = 0.f;

    for (Casella posizione = 0; posizione < 64; ++posizione) {
        const Pezzo pezzo = s.caselle[posizione];
        const float segno = pezzo_bianco(pezzo) ? 1.f : -1.f;

        switch (pezzo) {
        case 'P': case 'p':
            risultato += 10.f * segno;
            risultato += interpola_valore(s, .32f, .32f) * bonus_posizione_pedone(posizione, pezzo) * segno;
            risultato += interpola_valore(s, .05f, .05f) * malus_raddoppio_pedone(s, posizione, pezzo) * segno;
            risultato += interpola_valore(s, .04f, .04f) * malus_isolamento_pedone(s, posizione, pezzo) * segno;
            break;
        case 'R': case 'r':
            risultato += 50.f * segno;
            risultato += interpola_valore(s, .07f, .07f) * bonus_visibilita_torre(s, posizione) * segno;
            break;
        case 'B': case 'b':
            risultato += 31.f * segno;
            risultato += interpola_valore(s, .15f, .15f) * bonus_visibilita_alfiere(s, posizione) * segno;
            break;
        case 'N': case 'n':
            risultato += 30.f * segno;
            risultato += interpola_valore(s, .09f, .09f) * bonus_visibilita_cavallo(s, posizione) * segno;
            break;
        case 'Q': case 'q':
            risultato += 90.f * segno;
            risultato += interpola_valore(s, .02f, .02f) * bonus_visibilita_regina(s, posizione) * segno;
            break;
        case 'K': case 'k':
            risultato += 999.f * segno;
            break;
        default:
            break;
        }
    }

    return risultato;
}

bool stessa_mossa(const Mossa& a, const Mossa& b) {
    return a.da == b.da && a.a == b.a && a.promozione == b.promozione;
}

void ordina_tranquille(const Scacchiera& scacchiera, ListaMosse& mosse, const std::array<Mossa, 2>& killer, const int (&storicoMosse)[2][64][64]) {
    Mossa* inizio = mosse.begin();
    const auto fine = mosse.end();
    while (inizio != fine && inizio->priorita != 0)
        ++inizio;

    bool daOrdinare = false;
    for (Mossa* corrente = inizio; corrente != fine; ++corrente) {
        if (stessa_mossa(*corrente, killer[0]))
            corrente->priorita = 4000;
        else if (stessa_mossa(*corrente, killer[1]))
            corrente->priorita = 3999;
        else
            corrente->priorita = storicoMosse[scacchiera.turno][corrente->da][corrente->a] / 8;
        daOrdinare |= corrente->priorita != 0;
    }
    if (daOrdinare)
        std::sort(inizio, fine, [](const Mossa& a, const Mossa& b) { return a.priorita > b.priorita; });
}

void registra_taglio(const Scacchiera& scacchiera, std::array<Mossa, 2>& killer, const Mossa& mossa, int profondita, int (&storicoMosse)[2][64][64]) {
    if (mossa.promozione != NESSUN_PEZZO || pezzo_catturato(scacchiera, mossa) != NESSUN_PEZZO)
        return;
    if (!stessa_mossa(mossa, killer[0])) {
        killer[1] = killer[0];
        killer[0] = mossa;
    }
    const int peso = std::min(profondita, 16);
    const int bonus = peso * peso * 16;
    int& valore = storicoMosse[scacchiera.turno][mossa.da][mossa.a];
    valore += bonus - valore * bonus / 16384;
}

float punteggio_quiescenza(Scacchiera& scacchiera, float alpha, float beta, std::vector<uint64_t>& storico) {
    const bool massimizza = scacchiera.turno;
    const uint64_t hash = storico.back();
    if (hash::triplice_ripetizione(storico, hash))
        return 0.f;

    const bool sottoScacco = re_sottoscacco(scacchiera, massimizza);
    ListaMosse mosse;
    if (sottoScacco) {
        trova_mosse(scacchiera, mosse);
        if (mosse.empty())
            return massimizza ? -100000.f : 100000.f;
    } else if (!trova_mosse_quiescenza(scacchiera, mosse)) {
        return 0.f;
    }

    float migliore;
    if (sottoScacco) {
        migliore = massimizza ? -FLT_MAX : FLT_MAX;
    } else {
        migliore = analizza(scacchiera);
        if (massimizza) {
            if (migliore >= beta)
                return migliore;
            alpha = std::max(alpha, migliore);
        } else {
            if (migliore <= alpha)
                return migliore;
            beta = std::min(beta, migliore);
        }
    }

    StatoUnMossa stato;
    for (const Mossa& mossa : mosse) {
        applica_mossa(scacchiera, mossa, stato);
        const uint64_t hashFiglio = hash::dopo_mossa(hash, scacchiera, mossa, stato);
        storico.push_back(hashFiglio);

        const float punteggio = punteggio_quiescenza(scacchiera, alpha, beta, storico);

        storico.pop_back();
        undo_mossa(scacchiera, mossa, stato);

        if (massimizza) {
            migliore = std::max(migliore, punteggio);
            alpha = std::max(alpha, migliore);
        } else {
            migliore = std::min(migliore, punteggio);
            beta = std::min(beta, migliore);
        }

        if (alpha >= beta)
            break;
    }

    return migliore;
}

float punteggio_minimax(Scacchiera& scacchiera, int pr, float alpha, float beta, std::vector<ListaMosse>& listeMosse, std::vector<std::array<Mossa, 2>>& mosseKiller, int (&storicoMosse)[2][64][64], std::vector<uint64_t>& storico) {
    if (pr <= 0)
        return punteggio_quiescenza(scacchiera, alpha, beta, storico);

    const bool massimizza = scacchiera.turno;
    const uint64_t hash = storico.back();
    if (hash::triplice_ripetizione(storico, hash))
        return 0.f;

    ListaMosse& mosse = listeMosse[pr];
    trova_mosse(scacchiera, mosse);

    if (mosse.empty()) {
        if (re_sottoscacco(scacchiera, massimizza))
            return massimizza ? -100000.f : 100000.f;
        return 0.f;
    }

    ordina_tranquille(scacchiera, mosse, mosseKiller[pr], storicoMosse);
    StatoUnMossa stato;
    float migliore = massimizza ? -FLT_MAX : FLT_MAX;
    for (const Mossa& mossa : mosse) {
        applica_mossa(scacchiera, mossa, stato);
        const uint64_t hashFiglio = hash::dopo_mossa(hash, scacchiera, mossa, stato);
        storico.push_back(hashFiglio);

        const float p = punteggio_minimax(scacchiera, pr - 1, alpha, beta, listeMosse, mosseKiller, storicoMosse, storico);

        storico.pop_back();
        undo_mossa(scacchiera, mossa, stato);

        if (massimizza) {
            migliore = std::max(migliore, p);
            alpha = std::max(alpha, p);
        } else {
            migliore = std::min(migliore, p);
            beta = std::min(beta, p);
        }

        if (alpha >= beta) {
            registra_taglio(scacchiera, mosseKiller[pr], mossa, pr, storicoMosse);
            break;
        }
    }

    return migliore;
}

/*
int get_profondita(Scacchiera& scacchiera) {
    float v = 0;
    for (int i = 0; i < 63; ++i) {
        const Pezzo p = scacchiera.caselle.at(i);
        if (p == 'R' || p == 'r' || p == 'B' || p == 'b')
            v += 2;
        else if (p == 'Q' || p == 'q')
            v += 3;
        else if (p != NESSUN_PEZZO)
            v += 1;
    }
    // valore geogebra
    v = 57.f / (v + 4.f) - 1.3f + PROFONDITA_SHIFT;
    const int parteIntera = v;
    return v - parteIntera > .5f ? v + 1 : v;
}
*/

RisultatoRicerca prossima(Scacchiera& scacchiera, int profondita) {
    RisultatoRicerca risultato;
    ListaMosse mosse;
    trova_mosse(scacchiera, mosse);

    if (mosse.empty())
        return risultato;

    if (profondita <= 0)
        profondita = /*get_profondita(scacchiera);*/ PROFONDITA;
    const std::size_t livelli = profondita + 1u;
    std::vector<ListaMosse> listeMosse(livelli);
    std::vector<std::array<Mossa, 2>> mosseKiller(livelli, {MOSSA_NESSUNA, MOSSA_NESSUNA});
    int storicoMosse[2][64][64] {};

    const bool massimizza = scacchiera.turno;
    const uint64_t hashRadice = hash::calcola(scacchiera);
    std::vector<uint64_t> storico;
    storico.reserve(scacchiera.storicoHash.size() + livelli + 64);
    storico.insert(storico.end(), scacchiera.storicoHash.begin(), scacchiera.storicoHash.end());
    if (storico.empty() || storico.back() != hashRadice)
        storico.push_back(hashRadice);

    if (hash::triplice_ripetizione(storico, hashRadice))
        return risultato;

    float migliore = massimizza ? -FLT_MAX : FLT_MAX;
    float alpha = -FLT_MAX, beta = FLT_MAX;
    StatoUnMossa stato;
    for (const Mossa& mossa : mosse) {
        applica_mossa(scacchiera, mossa, stato);
        const uint64_t hashFiglio = hash::dopo_mossa(hashRadice, scacchiera, mossa, stato);
        storico.push_back(hashFiglio);

        const float punteggio = punteggio_minimax(scacchiera, profondita - 1, alpha, beta, listeMosse, mosseKiller, storicoMosse, storico);
        storico.pop_back();
        undo_mossa(scacchiera, mossa, stato);
        if (!risultato.haMossa || (massimizza ? punteggio > migliore : punteggio < migliore)) {
            migliore = punteggio;
            risultato.migliore = mossa;
            risultato.punteggio = punteggio;
            risultato.haMossa = true;
        }

        if (massimizza)
            alpha = std::max(alpha, migliore);
        else
            beta = std::min(beta, migliore);
    }

    risultato.profonditaRaggiunta = profondita;
    return risultato;
}

}
