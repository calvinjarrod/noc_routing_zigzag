
/*
 * ZigZag_router.cpp
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
 * Author: Jarrod Smith, Pedro Valencia, Eric Muller
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file ZigZag_router.cpp
/// \brief Implements ZigZag routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ZigZag_router.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method to set unique id
////////////////////////////////////////////////
void ZigZag_router::setID(UI id_tile) {
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
UI ZigZag_router::calc_next(UI ip_dir, ULL source_id, ULL dest_id) {
	// insert route logic here
	// return next hop direction(N, S, E, W, C)
	cout<<"My ID: "<<id<<", Flit from port: "<<ip_dir<<"\n";
	int xco = id % num_cols;
	int yco = id / num_cols;
	int dest_xco = dest_id % num_cols;
	int dest_yco = dest_id / num_cols;
	if (xco == dest_xco && yco == dest_yco) return C; // found node!
	// rule 2 packet will be sent in one direction until the X and Y directions
	// are equal
	else {
		int xco_diff = dest_xco - xco;
		int yco_diff = dest_yco - yco;
		// when X and Y distance are equal, then ZigZag!
		if (abs(xco_diff) == abs(yco_diff)) {
			cout<<"Can ZigZag"<<endl;
			// if the flit came from N or S then it must go either E or W
			if (ip_dir == N || ip_dir == S) {
				if (xco_diff < 0) {
					cout<<"Sending W"<<endl;
					return W;
				} else if (xco_diff > 0) {
					cout<<"Sending E"<<endl;
					return E;
				}
			// if the flit came from E or W then it must go either N or S
			} else if (ip_dir == W || ip_dir == E) {
				if (yco_diff < 0) {
					cout<<"Sending N"<<endl;
					return N;
				} else if (yco_diff > 0) {
					cout<<"Sending S"<<endl;
					return S;
				}
			} else { // must be the originator of flit so choose Y direction first
				if (yco_diff < 0) {
					cout<<"From Core, send it N"<<endl;
					return N;
					// return (yco_diff < 0) ? N : S;
				} else if (yco_diff > 0) {
					cout<<"From Core, send it S"<<endl;
					return S;
				}
			}
		// if greater X distance
		} else if (abs(yco_diff) < abs(xco_diff)) {
			cout<<"Can't ZigZag, yco_diff="<<yco_diff<<", xco_diff="<<xco_diff<<endl;
			return (xco_diff < 0) ? W : E;
		// if greater Y distance
		} else if (abs(yco_diff) > abs(xco_diff)) {
			cout<<"Can't ZigZag, yco_diff="<<yco_diff<<", xco_diff="<<xco_diff<<endl;
			return (yco_diff < 0) ? N : S;
		}
	} 	


}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base class
void ZigZag_router::initialize() {

}

UI ZigZag_router::get_estimate(UI d) {

}

void ZigZag_router::update_estimate(UI ip_dir, UI dest_id, UI estimate, ULL q) {

}

void ZigZag_router::update_power(int i, double y) {

}


// for dynamic linking
extern "C" {
router *maker() {
	return new ZigZag_router;
}
}
