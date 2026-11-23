#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <string>

using Casella = uint8_t;
using Pezzo = char;

constexpr Casella NESSUNA_CASELLA = 64;
constexpr char NESSUN_PEZZO = 0;

constexpr uint8_t ARROCCO_CORTO_BIANCO = 1;
constexpr uint8_t ARROCCO_LUNGO_BIANCO = 1 << 1;
constexpr uint8_t ARROCCO_CORTO_NERO = 1 << 2;
constexpr uint8_t ARROCCO_LUNGO_NERO = 1 << 3;

constexpr bool casella_valida(Casella c) { return c < 64; }
constexpr int riga(Casella c) { return c >> 3; }
constexpr int colonna(Casella c) { return c & 7; }
constexpr bool dentro_scacchiera(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }
constexpr Casella casella(int r, int c) { return (r << 3) + c; }
constexpr bool pezzo_bianco(Pezzo p) { return p >= 'A' && p <= 'Z'; }
constexpr bool pezzo_nero(Pezzo p) { return p >= 'a' && p <= 'z'; }
constexpr bool pezzo_nemico(Pezzo p, bool bianco) { return bianco ? pezzo_nero(p) : pezzo_bianco(p); }

struct Scacchiera {
    std::vector<uint64_t> storicoHash {};
    std::array<Pezzo, 64> caselle {};
    Casella posReBianco = 60, posReNero = 4;
    uint8_t dirittiArrocco = 0;
    Casella enpassant = NESSUNA_CASELLA;
    // true -> bianco, il player che massimizza
    bool turno = true;
};

struct Mossa {
    Casella da;
    Casella a;
    Pezzo promozione;
    int priorita;
};

constexpr Mossa MOSSA_NESSUNA {NESSUNA_CASELLA, NESSUNA_CASELLA, NESSUN_PEZZO, 0};

struct ListaMosse {
    static constexpr uint16_t CAPIENZA = 256;

    std::array<Mossa, CAPIENZA> dati;
    uint16_t numero = 0;

    void clear() { numero = 0; }
    bool empty() const { return numero == 0; }
    void push_back(Mossa mossa) { dati[numero++] = mossa; }

    Mossa* begin() { return dati.data(); }
    Mossa* end() { return dati.data() + numero; }
    const Mossa* begin() const { return dati.data(); }
    const Mossa* end() const { return dati.data() + numero; }
};

struct StatoUnMossa {
    Pezzo cattura;
    uint8_t dirittiArrocco;
    Casella enpassant;
};

struct RisultatoRicerca {
    Mossa migliore = MOSSA_NESSUNA;
    float punteggio = 0.f;
    int profonditaRaggiunta = 0;
    bool haMossa = false;
};

std::string nome_casella(Casella posizione);

void applica_mossa(Scacchiera& s, const Mossa& m, StatoUnMossa& stato);
void undo_mossa(Scacchiera& s, const Mossa& m, const StatoUnMossa& stato);
void aggiungi_mossa(ListaMosse& mosse, Casella da, Casella a, Pezzo promozione = NESSUN_PEZZO);
void aggiungi_promozioni(ListaMosse& mosse, Casella da, Casella a, bool bianco);
void aggiungi_mosse_a_raggio(const Scacchiera& s, Casella da, const int direzioni[][2], int numeroDirezioni, ListaMosse& mosse);
void aggiungi_mosse_a_salto(const Scacchiera& s, Casella da, const int salti[][2], int numeroSalti, ListaMosse& mosse);
void rimuovi_diritti_torre(Scacchiera& s, Casella posizione);
bool attaccata_da_raggio(const Scacchiera& s, Casella obiettivo, const int direzioni[][2], int numeroDirezioni, Pezzo pezzo, Pezzo regina);
void aggiungi_arrocchi(const Scacchiera& s, Casella da, bool bianco, ListaMosse& mosse);
void aggiungi_mosse_pedone(const Scacchiera& s, Casella da, ListaMosse& mosse);
void aggiungi_mosse_cavallo(const Scacchiera& s, Casella da, ListaMosse& mosse);
void aggiungi_mosse_alfiere(const Scacchiera& s, Casella da, ListaMosse& mosse);
void aggiungi_mosse_torre(const Scacchiera& s, Casella da, ListaMosse& mosse);
void aggiungi_mosse_regina(const Scacchiera& s, Casella da, ListaMosse& mosse);
void aggiungi_mosse_re(const Scacchiera& s, Casella da, ListaMosse& mosse);
void aggiungi_mosse_pezzo(const Scacchiera& s, Casella da, ListaMosse& mosse);
bool casella_attaccata(const Scacchiera& s, Casella casella, bool daBianchi);
bool re_sottoscacco(const Scacchiera& s, bool re);
Pezzo pezzo_catturato(const Scacchiera& s, const Mossa& mossa);
void trova_mosse(Scacchiera& s, ListaMosse& mosse);
bool trova_mosse_quiescenza(Scacchiera& s, ListaMosse& mosse);
