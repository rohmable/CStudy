extern char *percorso ;
extern esame_t esame ;
extern pomodoro_t timer ;
extern stat_t stats ;
extern GSList *lista ;
bool nuovo_esame (unsigned int g, unsigned int m, unsigned int a, int pag, int giorni_st_sett, int giorni_rip) ;
bool carica_base ( ) ;
bool carica_timer ( ) ;
bool salva_timer ( ) ;
bool aggiungi_carta (const char *domanda, const char *risposta, int scadenza) ;
gint compara_scadenze (gconstpointer a, gconstpointer b) ;
void carica_carte () ;
bool salva_carte ( ) ;
