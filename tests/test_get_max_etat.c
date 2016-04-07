/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2015 Adrien Boussicault
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
#include "outils.h"
#include "limits.h"


int test_get_max_etat() {
  int result = 1;

  {
    Automate * auto1 = creer_automate();
    Automate * auto2 = creer_automate();
    Automate * auto3 = creer_automate();
    
    ajouter_transition(auto1, 1, 'a', 1);
    ajouter_transition(auto1, 1, 'b', 1);
    ajouter_transition(auto1, 1, 'c', 2);
    ajouter_transition(auto1, 2, 'b', 2);
    ajouter_transition(auto1, 2, 'c', 2);
    ajouter_transition(auto1, 2, 'a', 3);
    ajouter_transition(auto1, 3, 'a', 3);
    ajouter_transition(auto1, 3, 'b', 3);
    ajouter_transition(auto1, 3, 'c', 3);
    ajouter_etat_initial(auto1, 1);
    ajouter_etat_final(auto1, 3);
    
    ajouter_transition(auto2, 1, 'a', 44);
    ajouter_etat_initial(auto2, 1);
    ajouter_etat_final(auto2, 44);
    
    TEST(
	 1
	 && auto1
	 && auto2
	 && auto3
	 && (get_max_etat(auto1) == 3)
	 && (get_max_etat(auto2) == 44)
	 && (get_max_etat(auto3) == INT_MIN)
	 , result);
    
    liberer_automate(auto1);
    liberer_automate(auto2);
    liberer_automate(auto3);
  }
  
  return result;

}

int main() {
  
  if (! test_get_max_etat()) {
    return 1;
  }

  return 0;

}
