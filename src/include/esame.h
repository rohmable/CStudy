struct esame_t {
	int pag, gg_ripasso, gg_studio_sett, pag_per_giorno ;
	unsigned int g_inizio, m_inizio, a_inizio,
				 g_esame, m_esame, a_esame ;
} ;

struct pomodoro_t {
	unsigned int lavoro, pausa_corta, pausa_lunga ;
} ;

struct stat_t {
	unsigned int g_ultimo_agg, m_ultimo_agg, a_ultimo_agg ;
	int lavoro_giorno, lavoro_sett ;
} ;

struct flashcards_t {
	int scadenza ;
	string domanda, risposta ;
} ;
