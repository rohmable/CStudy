/** @file
	Header file contenente le strutture dati necessarie a far funzionare il programma
*/

/** Informazioni relative ad un esame.*/
struct esame_t {
	int pag, /**< Pagine da studiare per l'esame.*/
		gg_ripasso, /**< Giorni di ripasso prima dell'esame.*/
		gg_studio_sett, /**< Giorni di studio per settimana.*/
		pag_per_giorno ; /**< Numero di pagine da studiare in un giorno.*/
	unsigned int g_esame, /**< Giorno dell'esame.*/
				 m_esame, /**< Mese dell'esame.*/
				 a_esame, /**< Anno dell'esame.*/
				 g_ultimo_avvio, /**< Giorno dell'ultimo avvio.*/
				 m_ultimo_avvio, /**< Mese dell'ultimo avvio.*/
				 a_ultimo_avvio ; /**< Anno dell'ultimo avvio.*/
} ;

/** Tempi per il timer.*/
struct pomodoro_t {
	unsigned int lavoro, /**< Tempo di lavoro, di default 25 minuti.*/
				 pausa_corta, /**< Tempo di una pausa corta, di default 5 minuti.*/
				 pausa_lunga ; /**< Tempo di una pausa lunga, di default 15 minuti.*/
} ;

/** Informazioni relative ad una carta.*/
struct flashcards_t {
	int scadenza ; /**< Scadenza in giorni della carta.	Quando questa e' minore od uguale a zero la carta verra' chiesta nel ripasso.*/
	string domanda, risposta ;
} ;
