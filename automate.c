/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2014, 2015 Adrien Boussicault
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "automate.h"
#include "table.h"
#include "ensemble.h"
#include "outils.h"
#include "fifo.h"

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 

#include <assert.h>

#include <math.h>

/* Compare la valeur max à notre élement, si celle ci est inférieure à l'élement, alors max prend la valeur de l'élement */
void action_get_max_etat( const intptr_t element, void* data){
	int * max = (int*) data;
	if ( *max < element ) *max = element;
}

/* Pour chaque état de l'automate, applique action_get_max et nous renvoie la valeur max, qui correspond à notre état maximum */
int get_max_etat( const Automate* automate ){
	int max = INT_MIN;

	pour_tout_element( automate->etats, action_get_max_etat, &max);

	return max;
}

void action_get_min_etat( const intptr_t element, void* data ){
	int * min = (int*) data;
	if( *min > element ) *min = element;
}

int get_min_etat( const Automate* automate ){
	int min = INT_MAX;

	pour_tout_element( automate->etats, action_get_min_etat, &min );

	return min;
}


int comparer_cle(const Cle *a, const Cle *b) {
	if( a->origine < b->origine )
		return -1;
	if( a->origine > b->origine )
		return 1;
	if( a->lettre < b->lettre )
		return -1;
	if( a->lettre > b->lettre )
		return 1;
	return 0;
}

void print_cle( const Cle * a){
	printf( "(%d, %c)" , a->origine, (char) (a->lettre) );
}

void supprimer_cle( Cle* cle ){
	xfree( cle );
}

void initialiser_cle( Cle* cle, int origine, char lettre ){
	cle->origine = origine;
	cle->lettre = (int) lettre;
}

Cle * creer_cle( int origine, char lettre ){
	Cle * result = xmalloc( sizeof(Cle) );
	initialiser_cle( result, origine, lettre );
	return result;
}

Cle * copier_cle( const Cle* cle ){
	return creer_cle( cle->origine, cle->lettre );
}

Automate * creer_automate(){
	Automate * automate = xmalloc( sizeof(Automate) );
	automate->etats = creer_ensemble( NULL, NULL, NULL );
	automate->alphabet = creer_ensemble( NULL, NULL, NULL );
	automate->transitions = creer_table(
		( int(*)(const intptr_t, const intptr_t) ) comparer_cle , 
		( intptr_t (*)( const intptr_t ) ) copier_cle,
		( void(*)(intptr_t) ) supprimer_cle
	);
	automate->initiaux = creer_ensemble( NULL, NULL, NULL );
	automate->finaux = creer_ensemble( NULL, NULL, NULL );
	automate->vide = creer_ensemble( NULL, NULL, NULL ); 
	return automate;
}

Automate * translater_automate_entier( const Automate* automate, int translation ){
	Automate * res = creer_automate();

	Ensemble_iterateur it;
	for( 
		it = premier_iterateur_ensemble( get_etats( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat( res, get_element( it ) + translation );
	}

	for( 
		it = premier_iterateur_ensemble( get_initiaux( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat_initial( res, get_element( it ) + translation );
	}

	for( 
		it = premier_iterateur_ensemble( get_finaux( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat_final( res, get_element( it ) + translation );
	}

	// On ajoute les lettres
	for(
		it = premier_iterateur_ensemble( get_alphabet( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_lettre( res, (char) get_element( it ) );
	}

	Table_iterateur it1;
	Ensemble_iterateur it2;
	for(
		it1 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it1 );
		it1 = iterateur_suivant_table( it1 )
	){
		Cle * cle = (Cle*) get_cle( it1 );
		Ensemble * fins = (Ensemble*) get_valeur( it1 );
		for(
			it2 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int fin = get_element( it2 );
			ajouter_transition(
				res, cle->origine + translation, cle->lettre, fin + translation
			);
		}
	};

	return res;
}


void liberer_automate( Automate * automate ){
	assert( automate );
	liberer_ensemble( automate->vide );
	liberer_ensemble( automate->finaux );
	liberer_ensemble( automate->initiaux );
	pour_toute_valeur_table(
		automate->transitions, ( void(*)(intptr_t) ) liberer_ensemble
	);
	liberer_table( automate->transitions );
	liberer_ensemble( automate->alphabet );
	liberer_ensemble( automate->etats );
	xfree(automate);
}

const Ensemble * get_etats( const Automate* automate ){
	return automate->etats;
}

const Ensemble * get_initiaux( const Automate* automate ){
	return automate->initiaux;
}

const Ensemble * get_finaux( const Automate* automate ){
	return automate->finaux;
}

const Ensemble * get_alphabet( const Automate* automate ){
	return automate->alphabet;
}

void ajouter_etat( Automate * automate, int etat ){
	ajouter_element( automate->etats, etat );
}

void ajouter_lettre( Automate * automate, char lettre ){
	ajouter_element( automate->alphabet, lettre );
}

void ajouter_transition(
	Automate * automate, int origine, char lettre, int fin
){
	ajouter_etat( automate, origine );
	ajouter_etat( automate, fin );
	ajouter_lettre( automate, lettre );

	Cle cle;
	initialiser_cle( &cle, origine, lettre );
	Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
	Ensemble * ens;
	if( iterateur_est_vide( it ) ){
		ens = creer_ensemble( NULL, NULL, NULL );
		add_table( automate->transitions, (intptr_t) &cle, (intptr_t) ens );
	}else{
		ens = (Ensemble*) get_valeur( it );
	}
	ajouter_element( ens, fin );
}

void ajouter_etat_final(
	Automate * automate, int etat_final
){
	ajouter_etat( automate, etat_final );
	ajouter_element( automate->finaux, etat_final );
}

void ajouter_etat_initial(
	Automate * automate, int etat_initial
){
	ajouter_etat( automate, etat_initial );
	ajouter_element( automate->initiaux, etat_initial );
}

const Ensemble * voisins( const Automate* automate, int origine, char lettre ){
	Cle cle;
	initialiser_cle( &cle, origine, lettre );
	Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
	if( ! iterateur_est_vide( it ) ){
		return (Ensemble*) get_valeur( it );
	}else{
		return automate->vide;
	}
}

Ensemble * delta1(
	const Automate* automate, int origine, char lettre
){
	Ensemble * res = creer_ensemble( NULL, NULL, NULL );
	ajouter_elements( res, voisins( automate, origine, lettre ) );
	return res; 
}

Ensemble * delta(
	const Automate* automate, const Ensemble * etats_courants, char lettre
){
	Ensemble * res = creer_ensemble( NULL, NULL, NULL );

	Ensemble_iterateur it;
	for( 
		it = premier_iterateur_ensemble( etats_courants );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		const Ensemble * fins = voisins(
			automate, get_element( it ), lettre
		);
		ajouter_elements( res, fins );
	}

	return res;
}

Ensemble * delta_star(
	const Automate* automate, const Ensemble * etats_courants, const char* mot
){
	int len = strlen( mot );
	int i;
	Ensemble * old = copier_ensemble( etats_courants );
	Ensemble * new = old;
	for( i=0; i<len; i++ ){
		new = delta( automate, old, *(mot+i) );
		liberer_ensemble( old );
		old = new;
	}
	return new;
}

void pour_toute_transition(
	const Automate* automate,
	void (* action )( int origine, char lettre, int fin, void* data ),
	void* data
){
	Table_iterateur it1;
	Ensemble_iterateur it2;
	for(
		it1 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it1 );
		it1 = iterateur_suivant_table( it1 )
	){
		Cle * cle = (Cle*) get_cle( it1 );
		Ensemble * fins = (Ensemble*) get_valeur( it1 );
		for(
			it2 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int fin = get_element( it2 );
			action( cle->origine, cle->lettre, fin, data );
		}
	};
}

Automate* copier_automate( const Automate* automate ){
	Automate * res = creer_automate();
	Ensemble_iterateur it1;
	// On ajoute les états de l'automate
	for(
		it1 = premier_iterateur_ensemble( get_etats( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat( res, get_element( it1 ) );
	}
	// On ajoute les états initiaux
	for(
		it1 = premier_iterateur_ensemble( get_initiaux( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat_initial( res, get_element( it1 ) );
	}
	// On ajoute les états finaux
	for(
		it1 = premier_iterateur_ensemble( get_finaux( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat_final( res, get_element( it1 ) );
	}
	// On ajoute les lettres
	for(
		it1 = premier_iterateur_ensemble( get_alphabet( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_lettre( res, (char) get_element( it1 ) );
	}
	// On ajoute les transitions
	Table_iterateur it2;
	for(
		it2 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it2 );
		it2 = iterateur_suivant_table( it2 )
	){
		Cle * cle = (Cle*) get_cle( it2 );
		Ensemble * fins = (Ensemble*) get_valeur( it2 );
		for(
			it1 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it1 );
			it1 = iterateur_suivant_ensemble( it1 )
		){
			int fin = get_element( it1 );
			ajouter_transition( res, cle->origine, cle->lettre, fin );
		}
	}
	return res;
}

Automate * translater_automate(
	const Automate * automate, const Automate * automate_a_eviter
){
	if(
		taille_ensemble( get_etats(automate) ) == 0 ||
		taille_ensemble( get_etats(automate_a_eviter) ) == 0
	){
		return copier_automate( automate );
    }

    int translation = get_max_etat( automate_a_eviter ) - get_min_etat( automate ) + 1;
    return translater_automate_entier( automate, translation );
}

int est_une_transition_de_l_automate(
	const Automate* automate,
	int origine, char lettre, int fin
    ){
	return est_dans_l_ensemble( voisins( automate, origine, lettre ), fin );
}

int est_un_etat_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_etats( automate ), etat );
}

int est_un_etat_initial_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_initiaux( automate ), etat );
}

int est_un_etat_final_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_finaux( automate ), etat );
}

int est_une_lettre_de_l_automate( const Automate* automate, char lettre ){
	return est_dans_l_ensemble( get_alphabet( automate ), lettre );
}

void print_ensemble_2( const intptr_t ens ){
	print_ensemble( (Ensemble*) ens, NULL );
}

void print_lettre( intptr_t c ){
	printf("%c", (char) c );
}

void print_automate( const Automate * automate ){
	printf("- Etats : ");
	print_ensemble( get_etats( automate ), NULL );
	printf("\n- Initiaux : ");
	print_ensemble( get_initiaux( automate ), NULL );
	printf("\n- Finaux : ");
	print_ensemble( get_finaux( automate ), NULL );
	printf("\n- Alphabet : ");
	print_ensemble( get_alphabet( automate ), print_lettre );
	printf("\n- Transitions : ");
	print_table( 
		automate->transitions,
		( void (*)( const intptr_t ) ) print_cle, 
		( void (*)( const intptr_t ) ) print_ensemble_2,
		""
	);
	printf("\n");
}

int le_mot_est_reconnu( const Automate* automate, const char* mot ){
	Ensemble * arrivee = delta_star( automate, get_initiaux(automate) , mot ); 
	
	int result = 0;

	Ensemble_iterateur it;
	for(
		it = premier_iterateur_ensemble( arrivee );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		if( est_un_etat_final_de_l_automate( automate, get_element(it) ) ){
			result = 1;
			break;
		}
	}
	liberer_ensemble( arrivee );
	return result;
}

Automate * mot_to_automate( const char * mot ){
	Automate * automate = creer_automate();
	int i = 0;
	int size = strlen( mot );
	for( i=0; i < size; i++ ){
		ajouter_transition( automate, i, mot[i], i+1 );
	}
	ajouter_etat_initial( automate, 0 );
	ajouter_etat_final( automate, size );
	return automate;
}

/***************Fonctions completées****************/

// Action pour ajouter tous les élements de l'ensemble à une ensemble d'états Initiaux
void action_ajouter_etats_initiaux(const intptr_t element, void* data){
    Automate* a = (Automate*) data ;
    ajouter_etat_initial(a, element);
}

// Action pour ajouter tous les élements de l'ensemble à une ensemble d'états Finaux
void action_ajouter_etats_finaux(const intptr_t element, void* data){
    Automate* a = (Automate*) data ;
    ajouter_etat_final(a, element);
}

// Action pour ajouter tous les élements de l'ensemble à une ensemble d'états
void action_ajouter_etats(const intptr_t element, void* data){
    Automate* a = (Automate*) data ;
    ajouter_etat(a, element);
}

void action_ajouter_alphabet(const intptr_t element, void* data){
    Automate* a = (Automate*) data;
    ajouter_lettre(a, element);
}

void action_ajouter_transition(int origine, char lettre, int fin, void* data){
    Automate* a = (Automate*) data ;
    ajouter_transition(a, origine, lettre, fin);
}

// Creer un automate a qui est l'union de automate_1 et automate_2
// manque à gérer le cas de l'ensemble vide dans l'automate
Automate * creer_union_des_automates(const Automate * automate_1, const Automate * automate_2){
    Automate* a = copier_automate(automate_1);
    Ensemble* new_etats = copier_ensemble(get_initiaux(automate_2));
    pour_tout_element(new_etats, action_ajouter_etats_initiaux, a);
    new_etats = copier_ensemble(get_finaux(automate_2));
    pour_tout_element(new_etats, action_ajouter_etats_finaux, a);
    new_etats = copier_ensemble(get_etats(automate_2));
    pour_tout_element(new_etats, action_ajouter_etats, a);

    new_etats = copier_ensemble(get_alphabet(automate_2));
    pour_tout_element(new_etats, action_ajouter_alphabet, a);

    pour_toute_transition(automate_2,action_ajouter_transition, a);

    return a;
}

void action_recuperer_accessibles_etat(intptr_t element, void* data){
  Automate* a = (Automate*) data;
  Ensemble* e = copier_ensemble(a->vide);
  vider_ensemble(a->vide);

  Ensemble_iterateur it = premier_iterateur_ensemble(e);
  int etat = get_element(it);

  ajouter_elements(a->vide, delta1(a, etat, element));
}

Ensemble* etats_accessibles( const Automate * automate, int etat ){
    Automate* a = copier_automate(automate);
    Ensemble* alp = copier_ensemble(get_alphabet(automate));
    Ensemble* etats_acc;
    Ensemble_iterateur it = premier_iterateur_ensemble(alp);

    char lettre = get_element(it);

    etats_acc = copier_ensemble(delta1(a, etat, lettre));
    it = iterateur_suivant_ensemble(it);
    while(!iterateur_est_vide(it)){
        lettre = get_element(it);
        ajouter_elements(etats_acc, delta1(a, etat, lettre));
        it = iterateur_suivant_ensemble(it);
    }
    
    return etats_acc;
}

// On récupère l'ensemble d'états accessibles pour une lettre et l'ensemble d'états initiaux
// On les stocke dans le champs vide de l'automate.
void action_recuperer_accessibles(intptr_t element, void* data){
    Automate* a = (Automate*) data;
    Ensemble* ens = copier_ensemble(get_initiaux(a));
    ajouter_elements(a->vide, delta(a, ens, element));
}

// Pour chaque lettre de l'alphabet, on appelle action_recuperer_accessibles
// Afin de stocker un ensmble avec tous les étas accessibles pour chaque lettre de l'alphabet de l'automate.
Ensemble* accessibles( const Automate * automate ){
    Automate* a = copier_automate(automate);
    Ensemble* alp = copier_ensemble(get_alphabet(automate));
    Ensemble* old_vide = copier_ensemble(a->vide);
    Ensemble* result;

    pour_tout_element(alp, action_recuperer_accessibles, a);
    result = copier_ensemble(a->vide);
    vider_ensemble( a->vide);
    deplacer_ensemble(a->vide, old_vide);
    return result;
}

Automate *automate_accessible( const Automate * automate ){
    Automate* a = copier_automate(automate);
    Ensemble* etat_acc = accessibles(automate);
    Ensemble* etat_non_acc = creer_difference_ensemble(get_etats(automate), etat_acc);

    transferer_elements(a->etats, etat_non_acc);
    Automate* accessible = translater_automate(automate, a);
    return accessible;
}

// Ajoute les transitions d'un premier automate à un second en inversant l'oigine et la fin des transitions
void action_ajouter_transition_inverse(int origine, char lettre, int fin, void* data){
  Automate* a = (Automate*) data ;
  ajouter_transition(a, fin, lettre, origine);
}

Automate *miroir( const Automate * automate){
  Automate* a = creer_automate();
  Ensemble* ens;
  
  ens = copier_ensemble(get_initiaux(automate));
  pour_tout_element(ens, action_ajouter_etats_finaux, a);
  
  ens = copier_ensemble(get_finaux(automate));
  pour_tout_element(ens, action_ajouter_etats_initiaux, a);
  
  ens = copier_ensemble(get_alphabet(automate));
  pour_tout_element(ens, action_ajouter_alphabet, a);
  
  ens = copier_ensemble(get_etats(automate));
  pour_tout_element(ens, action_ajouter_etats, a);
  
  pour_toute_transition(automate, action_ajouter_transition_inverse, a);
  
  return a;
}

Automate * creer_automate_du_melange(const Automate* automate_1,  const Automate* automate_2){ 
  
  Ensemble* alphabet1 = copier_ensemble(get_alphabet(automate_1));
  Ensemble* alphabet2 = copier_ensemble(get_alphabet(automate_2));
  Ensemble* alphabet = creer_union_ensemble(alphabet1, alphabet2);
  
  Ensemble* init1 = copier_ensemble(get_initiaux(automate_1));
  Ensemble* init2 = copier_ensemble(get_initiaux(automate_2));
  Ensemble_iterateur it_init1 = premier_iterateur_ensemble(init1);
  Ensemble_iterateur it_init2 = premier_iterateur_ensemble(init2);
  
  Ensemble* final1 = copier_ensemble(get_finaux(automate_1));
  Ensemble* final2 = copier_ensemble(get_finaux(automate_2));
  Ensemble_iterateur it_final1 = premier_iterateur_ensemble(final1);
  Ensemble_iterateur it_final2 = premier_iterateur_ensemble(final2);

  Automate* autMelange = creer_automate();
  /* Ajouts etats initiaux */
  ajouter_etat_initial(autMelange, get_element(it_init1) * 10 + get_element(it_init2));
  /* Ajouts etats finaux */
  ajouter_etat_final(autMelange, get_element(it_final1) * 10 + get_element(it_final2));
    
  Ensemble* etats1 = copier_ensemble(get_etats(automate_1));
  Ensemble_iterateur it_etat1 = premier_iterateur_ensemble(etats1); 
  Ensemble_iterateur it_alphabet, it_etat_access1;
  
  Ensemble* etats2 = copier_ensemble(get_etats(automate_2));
  Ensemble_iterateur it_etat2, it_etat_access2;
  
  char lettre;
  int etat1, etat_access1, etat2, etat_access2;

 /* On obtient un etat de la forme 10 ou 11, le premier chiffre représente l'état de l'automate 1, le second celui du deuxième automate */

  /* Parcour de l'automate 1 */  
  while(!iterateur_est_vide(it_etat1)){

    etat1 = get_element(it_etat1);
    it_alphabet = premier_iterateur_ensemble(alphabet);

    while(!iterateur_est_vide(it_alphabet)){

      lettre = get_element(it_alphabet);
      it_etat_access1 = premier_iterateur_ensemble(etats1);

      while(!iterateur_est_vide(it_etat_access1)){

	etat_access1 = get_element(it_etat_access1);

	  if(est_une_transition_de_l_automate(automate_1, etat1, lettre, etat_access1)){
	    it_etat2 = premier_iterateur_ensemble(etats2);

	    while(!iterateur_est_vide(it_etat2)){

	      etat2 = get_element(it_etat2);
	      it_etat_access2 = premier_iterateur_ensemble(etats2);

	      while(!iterateur_est_vide(it_etat_access2)){

		etat_access2 = get_element(it_etat_access2);

		if(est_une_transition_de_l_automate(automate_2, etat2, lettre, etat_access2)){
		  ajouter_transition(autMelange, (etat1 * 10 + etat2), lettre, (etat_access1 *10 + etat_access2));   
		}
		else{
		  ajouter_transition(autMelange, (etat1 * 10 + etat2), lettre, (etat_access1 * 10 + etat2));
		}

		it_etat_access2 = iterateur_suivant_ensemble(it_etat_access2);
	      }
	      it_etat2 = iterateur_suivant_ensemble(it_etat2);
	    }
	  }
	  it_etat_access1 = iterateur_suivant_ensemble(it_etat_access1);
      }	
      it_alphabet = iterateur_suivant_ensemble(it_alphabet);
    }
    it_etat1 = iterateur_suivant_ensemble(it_etat1);
  }

  /*Parcours de l'automate 2*/
  
  it_etat2 = premier_iterateur_ensemble(etats2);

  while(!iterateur_est_vide(it_etat2)){

    etat2 = get_element(it_etat2);
    it_alphabet = premier_iterateur_ensemble(alphabet);

    while(!iterateur_est_vide(it_alphabet)){

      lettre = get_element(it_alphabet);
      it_etat_access2 = premier_iterateur_ensemble(etats2);

      while(!iterateur_est_vide(it_etat_access2)){

	etat_access2 = get_element(it_etat_access2);

	  if(est_une_transition_de_l_automate(automate_2, etat2, lettre, etat_access2)){
	    it_etat1 = premier_iterateur_ensemble(etats1);

	    while(!iterateur_est_vide(it_etat1)){

	      etat1 = get_element(it_etat1);
	      it_etat_access1 = premier_iterateur_ensemble(etats1);

	      while(!iterateur_est_vide(it_etat_access1)){

		etat_access1 = get_element(it_etat_access1);

		if(est_une_transition_de_l_automate(automate_1, etat1, lettre, etat_access1)){
		  ajouter_transition(autMelange, (etat1 * 10 + etat2), lettre, (etat_access1 *10 + etat_access2));   
		}
		else{
		  ajouter_transition(autMelange, (etat1 * 10 + etat2), lettre, (etat1 * 10 + etat_access2));
		}

		it_etat_access1 = iterateur_suivant_ensemble(it_etat_access1);
	      }
	      it_etat1 = iterateur_suivant_ensemble(it_etat1);
	    }
	  }
	  it_etat_access2 = iterateur_suivant_ensemble(it_etat_access2);
      }	
      it_alphabet = iterateur_suivant_ensemble(it_alphabet);
    }
    it_etat2 = iterateur_suivant_ensemble(it_etat2);
  }

  return autMelange;
}

  
  
