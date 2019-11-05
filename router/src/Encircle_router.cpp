
/*
 * Encircle_router.cpp
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Jarrod Smith, Shylesh Umapathy, Meet Shah
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Encircle_router.cpp
/// \brief Implements Encircle routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Encircle_router.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method to set unique id
////////////////////////////////////////////////
void Encircle_router::setID(UI id_tile) {
	id = id_tile;
	initialize();
}

////////////////////////////////////////////////
/// Method that implements routing
/// inherited from base class router
/// Parameters:
/// - input direction from which flit entered the tile
/// - tileID of source tile
/// - tileID of destination tile
/// returns next hop direction
////////////////////////////////////////////////
UI Encircle_router::calc_next(UI ip_dir, ULL source_id, ULL dest_id) {
	// insert route logic here
	// return next hop direction(N, S, E, W, C)
	cout<<"My ID: "<<id<<", Flit from port: "<<ip_dir<<"\n";
	int xco = id % num_cols;
	int yco = id / num_cols;
	int dest_xco = dest_id % num_cols;
	int dest_yco = dest_id / num_cols;
	int x_diff = xco-dest_xco;
	int y_diff = yco-dest_yco;
	
	cout<<"x_diff="<<x_diff<<" y_diff="<<y_diff<<endl;
	cout<<"xco="<<xco<<" yco="<<yco<<endl;
	if (xco == dest_xco && yco == dest_yco) return C; // found node!
	// determine which case for Encircle
	// if source is in region 1
	else if (getRegion(xco,yco) == 1) {
		// CASE 1, both source and dest are in region 1
		// ENCIRCLE ALGORITHM
		if (getRegion(dest_xco,dest_yco) == 1) {
			// move based on greatest difference in distance
			if (abs(x_diff) < abs(y_diff)) {
				if (y_diff < 0) { // normally go S
					// check if going S takes us to region 2
					if (getRegion(xco,yco+1) == 2) {
						// figure way to the W or E around region 2
						if (x_diff < 0) return E;
						else if (x_diff > 0) return W;
						else {
							// check which edge xco is closer to
							if (xco < (num_cols - xco)) return W;
							else return E;
						}
					} else return S;
				} else { // normally go N
					if (getRegion(xco,yco-1) == 2) {
						// figure way to the W or E around region 2
						if (x_diff < 0) return E;
						else if (x_diff > 0) return W;
						else {
							// check which edge xco is closer to
							if (xco < (num_cols - xco)) return W;
							else return E;
						}
					} else return N;
				}
			} else if (abs(x_diff) > abs(y_diff)) {
				if (x_diff < 0) { // normally go E
					// check if going E takes us to region 2
					if (getRegion(xco+1,yco) == 2) {
						// figure way to the N or S around region 2
						if (y_diff < 0) return S;
						else if (y_diff > 0) return N;
						else {
							// check which edge yco is closer to
							if (yco < (num_rows - yco)) return N;
							else return S;
						}
					} else return E;
				} else { // normally go W
					if (getRegion(xco-1,yco) == 2) {
						// figure way to the N or S around region 2
						if (y_diff < 0) return S;
						else if (y_diff > 0) return N;
						else {
							// check which edge yco is closer to
							if (yco < (num_rows - yco)) return N;
							else return S;
						}
					} else return W;
				}
			} else { // distances are equal, choose Y direction
				if (y_diff < 0) {
					if (getRegion(xco,yco+1) == 2) {
						return (x_diff < 0) ? E:W;
					} else return S;
				} else {
					if (getRegion(xco,yco-1) == 2) {
						return (x_diff < 0) ? E:W;
					} else return N;
				}
			}	
		// CASE 3, simple XY routing
		} else {
			if (x_diff != 0) return (x_diff < 0) ? E:W;
			else return (y_diff < 0) ? S:N;
		}
	// source is in region 2
	} else { // CASE 2, 4, BOTH SIMPLE XY ROUTING
		if (x_diff != 0) return (x_diff < 0) ? E:W;
		else return (y_diff < 0) ? S:N;
	}	
}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base clas

int Encircle_router::getRegion(int x, int y) {
	if ((x<2 || x>((num_rows-1)-2))||(y<2 || y>((num_cols-1)-2))) 
		return 1;
	else return 2;
}

void Encircle_router::initialize() {

}

UI Encircle_router::get_estimate(UI d) {

}

void Encircle_router::update_estimate(UI ip_dir, UI dest_id, UI estimate, ULL q) {

}

void Encircle_router::update_power(int i, double y) {

}


// for dynamic linking
extern "C" {
router *maker() {
	return new Encircle_router;
}
}
