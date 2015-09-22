#include <sstream>
#include <ctime>
#include <cctype>
#ifdef DEBUG_MODE
	#include "./include/debug.h"
	#include <iostream>
#else
	#define DEB(a)
	#define VER(a)
#endif
using namespace std ;

extern unsigned int MASK ;

/** Converte una stringa contenente caratteri numerici nell' intero senza segno che rappresenta
	@param[in] stringa la stringa contenente il numero
	@param[in] size la grandezza della stringa (per controllare se la stringa contiene solo numeri)
	@param[out] num variabile in cui verra' inserito il numero
*/
static bool converti_stringa_in_numero (const char stringa[], const int size, unsigned int &num)
{
	stringstream buff ;
	for (int i = 0 ; i < size && stringa[i] != '\0' ; i++) {
		if (!isdigit(stringa[i])) {
			num = -1 ;
			return false ;
		}
	}
	buff << stringa ;
	buff >> num ;
	return true ;
}

/** Fornisce la data odierna nel formato mm/gg/aaaa
	@param[in,out] g giorno
	@param[in,out] m mese
	@param[in,out] a anno
*/
extern bool data_odierna_uint (unsigned int &g, unsigned int &m, unsigned int &a)
{
	time_t t ;
	time (&t) ;
	tm *tempo = localtime (&t);
	const int DIGITS = 3, ANNO = 5 ;
	char giorno[DIGITS], mese[DIGITS], anno[ANNO] ;
	strftime(giorno, DIGITS, "%d", tempo) ;
	strftime(mese, DIGITS, "%m", tempo) ;
	strftime(anno, ANNO, "%Y", tempo) ;
	VER(cout << "Giorno di oggi: " << giorno << "/" << mese << "/" << anno << endl ) ;
	
	if (!converti_stringa_in_numero(giorno, DIGITS, g)) {
		DEB(cout << "Errore, la stringa \"giorno\" non contiene solo numeri!" << endl
				 << "\tgiorno: " << giorno << endl ) ;
		return false ;
	}
	if (!converti_stringa_in_numero(mese, DIGITS, m)) {
		DEB(cout << "Errore, la stringa \"mese\" non contiene solo numeri!" << endl
				 << "\tmese: " << mese << endl ) ;
		return false ;
	}
	if (!converti_stringa_in_numero(anno, ANNO, a)) {
		DEB(cout << "Errore, la stringa \"mese\" non contiene solo numeri!" << endl
				 << "\tmese: " << mese << endl ) ;
		return false ;
	}
	
	return true ;
}

/** Calcola il numero di giorni che separano la data attuale da quella passata alla funzione
	@param[in] g,m,a data di cui calcolare la distanza
	@param[out] differenza di giorni dal giorno attuale*/
extern int diff_giorni_da_attuale (int g, int m, int a)
{
	time_t oggi ;
	time(&oggi) ;
	VER(const int SIZE = 9 ;
		char data[SIZE] ;
		strftime(data, SIZE, "%d/%m/%y", localtime(&oggi)) ;
		cout << "Data di oggi: " << data << endl
			 << "Data selezionata: " << g << '/' << m << '/' << a << endl ) ;
	tm data ;
	data.tm_sec = 0 ; data.tm_min = 0 ; data.tm_hour = 0 ; data.tm_isdst = -1 ;
	
	data.tm_mday = g ;
	data.tm_mon = m ;
	data.tm_year = a - 1900 ;
	
	//Calcola la differenza in secondi delle due date tramite difftime e le converte in intero
	int diff_secondi = static_cast<int>(difftime(mktime(&data), oggi)) ;
	
	VER(cout << "Differenza: " << (diff_secondi/3600)/24 << endl ) ;
	return (diff_secondi/3600)/24 + 1; // ritorna la differenza in giorni
}

/** Calcola il numero di pagine da studiare in un giorno, se la divisione intera da' un numero minore di uno, ::pagine_al_giorno viene
	incrementato di uno per rendere possibile lo studio delle pagine
	@param[in] g,m,a data dell'esame
	@param[in] pagine pagine da studiare
	@param[in] giorni_st_sett giorni di studio in una settimana
	@param[in] giorni_ripasso giorni da dedicare al ripasso
	@param[out] pagine_al_giorno numero di pagine da studiare in un giorno */
extern int calcola_pagine_al_giorno (unsigned int g, unsigned int m, unsigned int a, int pagine, int giorni_st_sett, int giorni_ripasso)
{
	VER(cout << "Calcolo il numero di pagine in un giorno" << endl ) ;
	int giorni_per_studiare = diff_giorni_da_attuale (static_cast<int>(g), static_cast<int>(m), static_cast<int>(a)) - giorni_ripasso,
		pagine_al_giorno = 0 ;
	if (giorni_per_studiare/giorni_st_sett == 0)
		pagine_al_giorno = pagine ;
	else
		pagine_al_giorno = pagine/(giorni_per_studiare/giorni_st_sett) ;
	VER(cout << "Pagine al giorno = " << pagine_al_giorno << endl ) ;
	if (pagine_al_giorno == 0)
		pagine_al_giorno ++ ;
	return pagine_al_giorno ;
}





















