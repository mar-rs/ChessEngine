#include "strutture.h"

std::string nome_casella(Casella posizione) {
    std::string nome(2, ' ');
    nome[0] = 'a' + colonna(posizione);
    nome[1] = '8' - riga(posizione);
    return nome;
}

void aggiungi_mossa(ListaMosse& mosse, Casella da, Casella a, Pezzo promozione) {
    mosse.push_back({da, a, promozione, 0});
}

void aggiungi_promozioni(ListaMosse& mosse, Casella da, Casella a, bool bianco) {
    if (bianco) {
        aggiungi_mossa(mosse, da, a, 'Q');
        aggiungi_mossa(mosse, da, a, 'R');
        aggiungi_mossa(mosse, da, a, 'B');
        aggiungi_mossa(mosse, da, a, 'N');
    } else {
        aggiungi_mossa(mosse, da, a, 'q');
        aggiungi_mossa(mosse, da, a, 'r');
        aggiungi_mossa(mosse, da, a, 'b');
        aggiungi_mossa(mosse, da, a, 'n');
    }
}

void aggiungi_mosse_a_raggio(const Scacchiera& s, Casella da, const int direzioni[][2], int numeroDirezioni, ListaMosse& mosse) {
    const bool bianco = pezzo_bianco(s.caselle[da]);

    for (int i = 0; i < numeroDirezioni; ++i) {
        int r = riga(da) + direzioni[i][0];
        int c = colonna(da) + direzioni[i][1];
        while (dentro_scacchiera(r, c)) {
            const Casella a = casella(r, c);
            const Pezzo occupante = s.caselle[a];
            if (occupante == NESSUN_PEZZO) {
                aggiungi_mossa(mosse, da, a);
            } else {
                if (pezzo_nemico(occupante, bianco))
                    aggiungi_mossa(mosse, da, a);
                break;
            }
            r += direzioni[i][0];
            c += direzioni[i][1];
        }
    }
}

void aggiungi_mosse_a_salto(const Scacchiera& s, Casella da, const int salti[][2], int numeroSalti, ListaMosse& mosse) {
    const bool bianco = pezzo_bianco(s.caselle[da]);
    for (int i = 0; i < numeroSalti; ++i) {
        const int r = riga(da) + salti[i][0];
        const int c = colonna(da) + salti[i][1];
        if (!dentro_scacchiera(r, c))
            continue;
        const Casella a = casella(r, c);
        if (!s.caselle[a] || pezzo_nemico(s.caselle[a], bianco))
            aggiungi_mossa(mosse, da, a);
    }
}

void rimuovi_diritti_torre(Scacchiera& s, Casella posizione) {
    switch (posizione) {
    case 63: s.dirittiArrocco &= ~ARROCCO_CORTO_BIANCO;     break;
    case 56: s.dirittiArrocco &= ~ARROCCO_LUNGO_BIANCO;     break;
    case 7:  s.dirittiArrocco &= ~ARROCCO_CORTO_NERO;       break;
    case 0:  s.dirittiArrocco &= ~ARROCCO_LUNGO_NERO;       break;
    default: break;
    }
}

bool attaccata_da_raggio(const Scacchiera& s, Casella obiettivo, const int direzioni[][2], int numeroDirezioni, Pezzo pezzo, Pezzo regina) {
    const int rigaObiettivo = riga(obiettivo);
    const int colonnaObiettivo = colonna(obiettivo);

    for (int i = 0; i < numeroDirezioni; ++i) {
        int r = rigaObiettivo + direzioni[i][0];
        int c = colonnaObiettivo + direzioni[i][1];

        while (dentro_scacchiera(r, c)) {
            const Pezzo occupante = s.caselle[casella(r, c)];
            if (occupante != NESSUN_PEZZO) {
                if (occupante == pezzo || occupante == regina)
                    return true;
                break;
            }

            r += direzioni[i][0];
            c += direzioni[i][1];
        }
    }

    return false;
}

void aggiungi_mosse_pedone(const Scacchiera& s, Casella da, ListaMosse& mosse) {
    const bool bianco = s.caselle[da] == 'P';
    const int passo = bianco ? -1 : 1;
    const int rigaPartenza = bianco ? 6 : 1;
    const int rigaPromozione = bianco ? 0 : 7;
    const int r = riga(da);
    const int c = colonna(da);
    const int rAvanti = r + passo;

    if (dentro_scacchiera(rAvanti, c)) {
        const Casella avanti = casella(rAvanti, c);
        if (s.caselle[avanti] == NESSUN_PEZZO) {
            if (rAvanti == rigaPromozione)
                aggiungi_promozioni(mosse, da, avanti, bianco);
            else
                aggiungi_mossa(mosse, da, avanti);

            const int rDoppio = r + 2 * passo;
            if (r == rigaPartenza && s.caselle[casella(rDoppio, c)] == NESSUN_PEZZO)
                aggiungi_mossa(mosse, da, casella(rDoppio, c));
        }
    }

    for (const int dc : {-1, 1}) {
        const int cCattura = c + dc;
        if (!dentro_scacchiera(rAvanti, cCattura))
            continue;

        const Casella a = casella(rAvanti, cCattura);
        if (!pezzo_nemico(s.caselle[a], bianco) && a != s.enpassant)
            continue;

        if (rAvanti == rigaPromozione)
            aggiungi_promozioni(mosse, da, a, bianco);
        else
            aggiungi_mossa(mosse, da, a);
    }
}

void aggiungi_mosse_cavallo(const Scacchiera& s, Casella da, ListaMosse& mosse) {
    constexpr int salti[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    aggiungi_mosse_a_salto(s, da, salti, 8, mosse);
}

void aggiungi_mosse_alfiere(const Scacchiera& s, Casella da, ListaMosse& mosse) {
    constexpr int direzioni[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    aggiungi_mosse_a_raggio(s, da, direzioni, 4, mosse);
}

void aggiungi_mosse_torre(const Scacchiera& s, Casella da, ListaMosse& mosse) {
    constexpr int direzioni[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    aggiungi_mosse_a_raggio(s, da, direzioni, 4, mosse);
}

void aggiungi_mosse_regina(const Scacchiera& s, Casella da, ListaMosse& mosse) {
    constexpr int direzioni[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    aggiungi_mosse_a_raggio(s, da, direzioni, 8, mosse);
}

void aggiungi_arrocchi(const Scacchiera& s, Casella da, bool bianco, ListaMosse& mosse) {
    if (da != (bianco ? 60 : 4))
        return;

    const Pezzo torre = bianco ? 'R' : 'r';
    const uint8_t corto = bianco ? ARROCCO_CORTO_BIANCO : ARROCCO_CORTO_NERO;
    const uint8_t lungo = bianco ? ARROCCO_LUNGO_BIANCO : ARROCCO_LUNGO_NERO;
    const bool daBianchi = !bianco;

    if ((s.dirittiArrocco & corto) && s.caselle[da + 3] == torre && !s.caselle[da + 1] && !s.caselle[da + 2] && !casella_attaccata(s, da, daBianchi) && !casella_attaccata(s, da + 1, daBianchi) && !casella_attaccata(s, da + 2, daBianchi))
        aggiungi_mossa(mosse, da, da + 2);

    if ((s.dirittiArrocco & lungo) && s.caselle[da - 4] == torre && !s.caselle[da - 1] && !s.caselle[da - 2] && !s.caselle[da - 3] && !casella_attaccata(s, da, daBianchi) && !casella_attaccata(s, da - 1, daBianchi) && !casella_attaccata(s, da - 2, daBianchi))
        aggiungi_mossa(mosse, da, da - 2);
}

void aggiungi_mosse_re(const Scacchiera& s, Casella da, ListaMosse& mosse) {
    constexpr int salti[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    const bool bianco = s.caselle[da] == 'K';
    aggiungi_mosse_a_salto(s, da, salti, 8, mosse);
    aggiungi_arrocchi(s, da, bianco, mosse);
}

void aggiungi_mosse_pezzo(const Scacchiera& s, Casella da, ListaMosse& mosse) {
    switch (s.caselle[da]) {
    case 'P': case 'p': aggiungi_mosse_pedone(s, da, mosse); break;
    case 'N': case 'n': aggiungi_mosse_cavallo(s, da, mosse); break;
    case 'B': case 'b': aggiungi_mosse_alfiere(s, da, mosse); break;
    case 'R': case 'r': aggiungi_mosse_torre(s, da, mosse); break;
    case 'Q': case 'q': aggiungi_mosse_regina(s, da, mosse); break;
    case 'K': case 'k': aggiungi_mosse_re(s, da, mosse); break;
    default: break;
    }
}

void applica_mossa(Scacchiera& s, const Mossa& m, StatoUnMossa& stato) {
    const Pezzo pezzo = s.caselle[m.da];
    const int spostamento = m.a - m.da;
    const bool pedone = pezzo == 'P' || pezzo == 'p';
    const bool re = pezzo == 'K' || pezzo == 'k';
    const bool enPassant = pedone && m.a == s.enpassant && colonna(m.da) != colonna(m.a) && s.caselle[m.a] == NESSUN_PEZZO;

    stato.dirittiArrocco = s.dirittiArrocco;
    stato.enpassant = s.enpassant;

    if (enPassant) {
        const Casella catturata = m.a + (pezzo == 'P' ? 8 : -8);
        stato.cattura = s.caselle[catturata];
        s.caselle[catturata] = NESSUN_PEZZO;
    } else {
        stato.cattura = s.caselle[m.a];
    }

    if (stato.cattura == 'R' || stato.cattura == 'r')
        rimuovi_diritti_torre(s, m.a);

    s.caselle[m.da] = NESSUN_PEZZO;
    s.caselle[m.a] = m.promozione != NESSUN_PEZZO ? m.promozione : pezzo;

    if (re) {
        if (pezzo == 'K') {
            s.posReBianco = m.a;
            s.dirittiArrocco &= ~(ARROCCO_CORTO_BIANCO | ARROCCO_LUNGO_BIANCO);
        } else {
            s.posReNero = m.a;
            s.dirittiArrocco &= ~(ARROCCO_CORTO_NERO | ARROCCO_LUNGO_NERO);
        }

        if (spostamento == 2 || spostamento == -2) {
            const bool arroccoCorto = spostamento > 0;
            const Casella torreDa = m.da + (arroccoCorto ? 3 : -4);
            const Casella torreA = m.da + (arroccoCorto ? 1 : -1);
            s.caselle[torreA] = s.caselle[torreDa];
            s.caselle[torreDa] = NESSUN_PEZZO;
        }
    } else if (pezzo == 'R' || pezzo == 'r') {
        rimuovi_diritti_torre(s, m.da);
    }

    s.enpassant = NESSUNA_CASELLA;
    if (pedone && (spostamento == 16 || spostamento == -16))
        s.enpassant = (m.da + m.a) / 2;

    s.turno = !s.turno;
}

void undo_mossa(Scacchiera& s, const Mossa& m, const StatoUnMossa& stato) {
    s.turno = !s.turno;

    const int spostamento = m.a - m.da;

    Pezzo pezzo = s.caselle[m.a];
    if (m.promozione != NESSUN_PEZZO)
        pezzo = s.turno ? 'P' : 'p';

    const bool pedone = pezzo == 'P' || pezzo == 'p';
    const bool re = pezzo == 'K' || pezzo == 'k';
    const bool enPassant = pedone && m.a == stato.enpassant && colonna(m.da) != colonna(m.a);

    s.caselle[m.da] = pezzo;
    s.caselle[m.a] = enPassant ? NESSUN_PEZZO : stato.cattura;

    if (enPassant) {
        const Casella catturata = m.a + (pezzo == 'P' ? 8 : -8);
        s.caselle[catturata] = stato.cattura;
    }

    if (re) {
        if (pezzo == 'K')
            s.posReBianco = m.da;
        else
            s.posReNero = m.da;

        if (spostamento == 2 || spostamento == -2) {
            const bool arroccoCorto = spostamento > 0;
            const Casella torreDa = m.da + (arroccoCorto ? 3 : -4);
            const Casella torreA = m.da + (arroccoCorto ? 1 : -1);
            s.caselle[torreDa] = s.caselle[torreA];
            s.caselle[torreA] = NESSUN_PEZZO;
        }
    }

    s.dirittiArrocco = stato.dirittiArrocco;
    s.enpassant = stato.enpassant;
}

bool casella_attaccata(const Scacchiera& s, Casella obiettivo, bool daBianchi) {
    const int rigaObiettivo = riga(obiettivo);
    const int colonnaObiettivo = colonna(obiettivo);

    const Pezzo pedone = daBianchi ? 'P' : 'p';
    const int rigaPedone = rigaObiettivo + (daBianchi ? 1 : -1);
    for (const int dc : {-1, 1}) {
        const int c = colonnaObiettivo + dc;
        if (dentro_scacchiera(rigaPedone, c) && s.caselle[casella(rigaPedone, c)] == pedone)
            return true;
    }

    constexpr int saltiCavallo[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    const Pezzo cavallo = daBianchi ? 'N' : 'n';
    for (const auto& salto : saltiCavallo) {
        const int r = rigaObiettivo + salto[0];
        const int c = colonnaObiettivo + salto[1];
        if (dentro_scacchiera(r, c) && s.caselle[casella(r, c)] == cavallo)
            return true;
    }

    const Pezzo re = daBianchi ? 'K' : 'k';
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0)
                continue;
            const int r = rigaObiettivo + dr;
            const int c = colonnaObiettivo + dc;
            if (dentro_scacchiera(r, c) && s.caselle[casella(r, c)] == re)
                return true;
        }
    }

    constexpr int ortogonali[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    if (attaccata_da_raggio(s, obiettivo, ortogonali, 4, daBianchi ? 'R' : 'r', daBianchi ? 'Q' : 'q'))
        return true;

    constexpr int diagonali[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    return attaccata_da_raggio(s, obiettivo, diagonali, 4, daBianchi ? 'B' : 'b', daBianchi ? 'Q' : 'q');
}

bool re_sottoscacco(const Scacchiera& s, bool reBianco) {
    const Casella posizioneRe = reBianco ? s.posReBianco : s.posReNero;
    return casella_attaccata(s, posizioneRe, !reBianco);
}

constexpr int valore_pezzo(Pezzo pezzo) {
    switch (pezzo) {
    case 'P': case 'p': return 10;
    case 'N': case 'n': return 30;
    case 'B': case 'b': return 31;
    case 'R': case 'r': return 50;
    case 'Q': case 'q': return 90;
    case 'K': case 'k': return 999;
    default: return 0;
    }
}

Pezzo pezzo_catturato(const Scacchiera& s, const Mossa& mossa) {
    const Pezzo attaccante = s.caselle[mossa.da];
    const bool pedone = attaccante == 'P' || attaccante == 'p';
    const bool enPassant = pedone && mossa.a == s.enpassant && colonna(mossa.da) != colonna(mossa.a) && s.caselle[mossa.a] == NESSUN_PEZZO;

    if (enPassant) {
        const Casella catturata = mossa.a + (attaccante == 'P' ? 8 : -8);
        return s.caselle[catturata];
    }
    return s.caselle[mossa.a];
}

int calcola_priorita(const Scacchiera& s, const Mossa& mossa) {
    const Pezzo catturato = pezzo_catturato(s, mossa);
    int priorita = 0;

    if (catturato != NESSUN_PEZZO) {
        // MVV-LVA: prima le vittime di maggior valore e gli attaccanti economici.
        priorita += 10000 + 10 * valore_pezzo(catturato) - valore_pezzo(s.caselle[mossa.da]);
    }
    if (mossa.promozione != NESSUN_PEZZO)
        priorita += 5000 + valore_pezzo(mossa.promozione);

    return priorita;
}

void ordina_mosse(ListaMosse& mosse) {
    std::sort(mosse.begin(), mosse.end(), [](const Mossa& a, const Mossa& b) { return a.priorita > b.priorita; });
}

template<bool SoloTattiche>
bool trova_mosse_impl(Scacchiera& s, ListaMosse& mosse) {
    mosse.clear();

    const bool bianco = s.turno;
    for (Casella da = 0; da < 64; ++da) {
        const Pezzo pezzo = s.caselle[da];
        if (bianco ? pezzo_bianco(pezzo) : pezzo_nero(pezzo))
            aggiungi_mosse_pezzo(s, da, mosse);
    }

    uint16_t valide = 0;
    bool haMossaLegale = false;
    StatoUnMossa stato;
    for (uint16_t i = 0; i < mosse.numero; ++i) {
        Mossa mossa = mosse.dati[i];
        bool tattica = true;
        if constexpr (SoloTattiche) {
            tattica = mossa.promozione != NESSUN_PEZZO || pezzo_catturato(s, mossa) != NESSUN_PEZZO;
            if (!tattica && haMossaLegale)
                continue;
        }
        applica_mossa(s, mossa, stato);
        const bool legale = !re_sottoscacco(s, bianco);
        undo_mossa(s, mossa, stato);
        if (!legale)
            continue;
        haMossaLegale = true;
        if (tattica) {
            mossa.priorita = calcola_priorita(s, mossa);
            mosse.dati[valide++] = mossa;
        }
    }
    mosse.numero = valide;
    ordina_mosse(mosse);
    return haMossaLegale;
}

void trova_mosse(Scacchiera& s, ListaMosse& mosse) {
    trova_mosse_impl<false>(s, mosse);
}

bool trova_mosse_quiescenza(Scacchiera& s, ListaMosse& mosse) {
    return trova_mosse_impl<true>(s, mosse);
}
