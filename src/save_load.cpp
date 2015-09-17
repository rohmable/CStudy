#include <cstdlib>
#include <fstream>
#include <ctime>
#include <sstream>
#ifdef DEBUG_MODE
	#include "./include/debug.h"
	#include <iostream>
#else
	#define DEB(a)
	#define VER(a)
#endif
using namespace std ;

const int DATA = 3 ;

// Da main.cpp
extern unsigned int MASK ;

/** Crea una cartella di nome ::nome (se non esiste) e vi salva un file "data.txt" in cui vengono scritti:
	@param[in] pag numero di pagine da studiare per l'esame
	@param[in] data_in[] data del primo giorno di studio
		@li data_in[0] Giorno
		@li data_in[1] Mese
		@li data_in[2] Anno
	@param[in] data_es[] data dell'esame
		@li data_es[0] Giorno
		@li data_es[1] Mese
		@li data_es[2] Anno
	@param[in] gg_st Giorni di studio settimanali
	@param[in] gg_rip Giorni di ripasso
	*/
extern bool nuovo_esame (const char nome[], int pag, int data_in[DATA], int data_es[DATA], int gg_st, int gg_rip)
{
	DEB(cout << "Creo nuovo esame" << endl ) ;
	stringstream buff ;
	buff << "mkdir ./saves/" << nome ;
	VER(cout << "Creo la cartella " << buff.str().c_str() << endl ) ;
	if (system(buff.str().c_str()) != 0) {
		DEB(cout << "Stato di errore diverso da zero, forse la cartella e' gia' esistente?" << endl ) ;
		return false ;
	}
	buff.str("") ;
	buff << "./saves/" << nome << "/data.txt" ;
	VER(cout << "Scrivo su " << buff.str().c_str() << endl ) ;
	ofstream salva (buff.str().c_str()) ;
	if (!salva) {
		DEB(cout << "Impossibile aprire " << buff.str().c_str() << endl ) ;
		salva.close() ;
		return false ;
	}
	else {
		salva << "# Numero di pagine da studiare" << endl << pag << endl
			  << "# Data di inizio dello studio" << endl ;
		for (int i = 0 ; i < DATA ; i ++) salva << data_in[i] << ' ' ;
		salva << endl ;
		salva << "# Data dell'esame" << endl ;
		for (int i = 0 ; i < DATA ; i ++) salva << data_es[i] << ' ' ;
		salva << "\n# Giorni di studio alla settimana" << endl << gg_st << endl 
			  << "# Giorni di ripasso prima dell'esame" << endl << gg_rip ;
	}
	salva.close() ;
	DEB(cout << "Nuovo esame creato con successo" << endl) ;
	return true ;
}

