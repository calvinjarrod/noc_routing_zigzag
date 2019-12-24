/*
 * Controller.cpp
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
 * Author: Lavina Jain
 *
 */

//////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Controller.cpp
/// \brief Implements routing
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "Controller.h"
#include "../router/src/CZ_router.h"
#include "../config/extern.h"

////////////////////////
/// Constructor
////////////////////////
	template<UI num_ip>
Controller<num_ip>::Controller(sc_module_name Controller): sc_module(Controller)
{

	void *hndl;
	void *mkr;


	sim_count=0;    //doubt

	string libname = string("./router/lib/");

	switch (RT_ALGO)
	{
		case OE: //tg = new BurstyTraffic("TG");
			libname = libname + string("OE_router.so");
			break;
		case XY:
			libname = libname + string("XY_router.so");
			break;
		case SOURCE:
			libname = libname + string("source_router.so");
			break;
		case QRT:
			libname = libname + string("Q_router.so");
			break;
			//DYAD//
		case DYAD:
			libname=libname+ string("DYADrouter.so");
			break;
			//MAXY//
		case MAXY:
			libname=libname+ string("maxy_router.so");
			break;
			//FTXY//
		case FTXY:
			libname=libname+ string("ftxy.so");
			break;
			//PROM
		case PROM:
			libname=libname+ string("PROM_router.so");
			break;
			//ERA
		case ER:
			libname=libname+ string("ER_router.so");
			break;
		case ZigZag:
			libname=libname+ string("ZigZag_router.so");
			break;
		case Encircle:
			libname=libname+ string("Encircle_router.so");
			break;
		case CZ:
			libname=libname+ string("CZ_router.so");
			break;
	}
	hndl = dlopen(libname.c_str(), RTLD_NOW);
	if (hndl == NULL)
	{
		cerr << dlerror() << endl;
		exit(-1);
	}
	mkr = dlsym(hndl, "maker");
	rtable = ((router*(*)())(mkr))();

	SC_THREAD(CC_refresh); // used in CZ algorithm 
	SC_THREAD(allocate_route);
	for (UI i = 0; i < num_ip; i++)
		sensitive << rtRequest[i];
	/*added */
	// process sensitive to power credit info, updates power info corresponding to the neighbouring tiles
	SC_THREAD(update_power_credit);
	for (UI i = 0; i < num_ip-1; i++)
	{
		sensitive << power_credit_in[i];
	}
	SC_THREAD(send_power_info);	// Thread sensitive to clock
	sensitive_pos << switch_cntrl;

	//qrt************************************************************
	rs.est_out = 0;
	rs.est_buffer = 0;

	for (UI i = 0; i < num_ip - 1; i++)
	{
		r_in[i].free = true;
	}

	SC_THREAD(transmitEst);
	sensitive_pos << switch_cntrl;

	SC_THREAD(rcv_estimate);
	for (UI i = 0; i < num_ip-1; i++)
		sensitive << estIn[i];

	rtable->num_nb = num_ip-1;
	//end for Q-ROuter
	//qrt************************************************************
	/*added */
	for (int i=0; i < num_ip-1 ; i++)
	{
		powerCreditLine t;
		t.Power = 0;
		power_credit_out[i].initialize(t);
	}
	/*end*/
}

//added
///////////////////////////////////////////////////////////////////////////
//Process Sesitive to clock
//////////////////////////////////////////////////////////////////////////

	template<UI num_ip>
void Controller<num_ip>::send_power_info()
{
	while (true)
	{
		wait();	// wait for the next clk cycle
		if (switch_cntrl.event())
		{
			powerCreditLine t;
			t.Power=P;
			for (int i=0; i<num_ip-1; i++)
				power_credit_out[i].write(t);
		}
	}
}


///////////////////////////////////////////////////////////////////////////
/// Process sensitive to incoming power credit information.
/// updates credit info (power values) of neighbor tiles
///////////////////////////////////////////////////////////////////////////
/*added*/
	template<UI num_ip>
void Controller<num_ip>::update_power_credit()
{
	for (int i=0; i<4; i++)
	{
		rtable->power[i]=0;
	}
	while (true)
	{
		wait();	// wait until change in credit info
		powerCreditLine t;
		for (UI i = 0; i < num_ip-1; i++)
		{
			if (power_credit_in[i].event())
			{

				Pnb[i] = power_credit_in[i].read().Power;	// update credit

				UI dir=idToDir(i);
				rtable->update_power(dir,Pnb[i]);
			}
		}

	}//end while
}
//end

///////////////////////////////////////////////////////////////////////////
//Process Sesitive to clock
//////////////////////////////////////////////////////////////////////////
	template<UI num_ip>
void Controller<num_ip>::transmitEst()
{

	int sim = 0;
	while (sim_count <SIM_NUM)
	{
		flit flit_out;
		wait(); //wait for clock
		//sim++;
		if (switch_cntrl.event() )  //&& (sim % 2)
		{
			sim_count++;
		} // end of if switch_cntrl
	}// end of while
} // end transmitest



///////////////////////////////////////////////////////////////////////////
//Process Sesitive to Estimate Inport
//////////////////////////////////////////////////////////////////////////
	template<UI num_ip>
void Controller<num_ip>::rcv_estimate()
{
	while (true)
	{
		wait();
		//cout<<"Inpor Event!!"<<endl;
		for (UI i = 0; i < num_ip-1; i++)
		{
			if (estIn[i].event() )//&& r_in[i].free)
			{
				rs.est_buffer++;
				r_in[i].val = estIn[i].read();
				r_in[i].free = false;
				UI dest = r_in[i].val.pkthdr.esthdr.d ;
				UI est = r_in[i].val.pkthdr.esthdr.estimate;
				//		cout<<"Est received: for "<<r_in[i].val.pkthdr.esthdr.d<<" destination and value "<< r_in[i].val.pkthdr.esthdr.estimate<<endl;
				rtable->update_estimate(i, dest , est ,0);
				r_in[i].free = true;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//NSF Process used to refresh congestion values in CZ algroithm
//////////////////////////////////////////////////////////////////////////
	template<UI num_ip>
void Controller<num_ip>::CC_refresh()
{
	while (true) {
		//rtable->congestCount = (rtable->congestCount == 100) ? 0 :\
			(rtable->congestCount+1);
		wait(10,SC_NS);
	}
}

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to route request
/// Calls routing algorithm
///////////////////////////////////////////////////////////////////////////
	template<UI num_ip>
void Controller<num_ip>::allocate_route()
{
	int flag=0;

	while (true)
	{
		wait();
		//	cout<<this->name()<<":(route) "<<sim_count<<endl;


		if (LOG >= 4)
			eventlog<<this->name()<<" Says ";
		for (UI i = 0; i < num_ip; i++)
		{
			if (LOG >= 4)
				eventlog<<" i = "<<i;
			if (rtRequest[i].event() && rtRequest[i].read() == ROUTE)
			{
				
				//
				sc_uint<ADDR_SIZE> src = sourceAddress[i].read();
				sc_uint<ADDR_SIZE> dest = destRequest[i].read();

				//qrt************************************************************
				sc_uint<64> timestamp = timestamp_ip[i].read();
				//qrt************************************************************

				UI ip_dir = idToDir(i);


				//qrt************************************************************
				UI temp = 0;
				//added for Q-Routing
				if (RT_ALGO == QRT && sim_count < PWAIT-1)
				{
					if ( DELTA != BETA)
						QXY_SIMCOUNT = DELTA;
					DELTA = .8;
				}
				else
					DELTA = QXY_SIMCOUNT;

				if (RT_ALGO == QRT && sim_count < QXY_SIMCOUNT)
					temp = 1;
				else if (RT_ALGO == QRT)
					temp = 0;
				else
					temp = ip_dir;
				/////////////////////////
				//qrt************************************************************
				// NSF BULLDOG MOTE -- CONGESTION ZONED CHANGES
				UI op_dir;
				if (RT_ALGO == XY) { // <-- change this back to CZ
					int flitCC = flitCCin[i].read();
					cout<<"CNT: Flit CC input: "<<flitCC<<endl;
					// update router's congestion count with MAX(rtableCC+1,flitCC+1)
					if ((rtable->congestCount+1) > (flitCC+1)) { 
						rtable->congestCount++;
						flitCCout[i].write(rtable->congestCount++);
					} else {
						rtable->congestCount = flitCC+1;	
						flitCCout[i].write(flitCC+1);
					}
					cout<<"CNT: Flit CC output: "<<rtable->congestCount<<endl;
					// assign incoming port the congestion value from flit
					// portCC is CC of neighbor
					if (ip_dir == N) rtable->congestN = flitCC;
					else if (ip_dir == S) rtable->congestS = flitCC;
					else if (ip_dir == E) rtable->congestE = flitCC;
					else if (ip_dir == W) rtable->congestW = flitCC;

					//cout<<"Controller "<<tileID<<" received CC of "<<flitCC<<\
						" from "<<ip_dir<<", new CC for node: "<<rtable->congestCount<<endl;
					// get route to next node
					op_dir = rtable->calc_next(temp, src, dest);
					//cout<<"OutputPort: "<<op_dir<<endl;		
					//cout<<"i="<<i<<endl;
					
					// determine portCC value for next node
					if (op_dir == N) {
						if ((rtable->congestCount-1) > (rtable->congestN+1))
							rtable->congestN = rtable->congestCount-1;
						else rtable->congestN++;
					} else if (op_dir == S) {
						if ((rtable->congestCount-1) > (rtable->congestS+1))
							rtable->congestS = rtable->congestCount-1;
						else rtable->congestS++;
					} else if (op_dir == E) {
						if ((rtable->congestCount-1) > (rtable->congestE+1))
							rtable->congestE = rtable->congestCount-1;
						else rtable->congestE++;
					} else if (op_dir == W) {
						if ((rtable->congestCount-1) > (rtable->congestW+1))
							rtable->congestW = rtable->congestCount-1;
						else rtable->congestW++;
					}
					
				} else {
					//UI op_dir = rtable->calc_next(temp, src, dest);
					op_dir = rtable->calc_next(temp,src,dest);
				}
#define iprint(value)	" " #value "= "<<value


				//qrt************************************************************
				if (LOG == 10)
					cout<<this->name()<<iprint(num_tiles)<<": "<<iprint(temp)<<iprint(ip_dir)<<iprint(op_dir)<<iprint(dest)<<iprint(src)<<iprint(i)<<iprint(timestamp)<<endl;
				if (RT_ALGO == QRT && !temp)
					op_dir = idToDir(op_dir);
				if (LOG == 10)
					cout<<iprint(op_dir)<<endl;
				if (RT_ALGO == QRT && ip_dir != C)
				{

					sc_uint<64> timestamp = timestamp_ip[i].read();
					UI estimate = op_dir == C ? 0 : rtable->get_estimate(dest);
					estimate += (sim_count - timestamp);
					flit* est_flit = new flit;
					create_est_flit(estimate,dest,est_flit);
					//	cout<<"Ctr: i "<<i<<" est value " <<est_flit->pkthdr.esthdr.estimate<<" dest = "<<est_flit->pkthdr.esthdr.estimate<<endl;

					if (est_flit != NULL)
					{
						rs.est_out++;
						// NSF testing
						//est_flit->pkthdr.nochdr.flithdr.payload.data_int = rtable->congestCount;
						estOut[i].write(*est_flit);
					}
					else
						cout<< "Error: Est_flit not created"<<endl;
				}
				//qrt************************************************************
				rtReady[i].write(true);
				nextRt[i].write(op_dir);

			}

			// request from IC to update //////////////////////////
			if (rtRequest[i].event() && rtRequest[i].read() == UPDATE)
			{
				sc_uint<ADDR_SIZE> src = sourceAddress[i].read();
				sc_uint<ADDR_SIZE> dest = destRequest[i].read();
				//rtable.update(dest, i);
				rtReady[i].write(true);
			}
			if (rtRequest[i].event() && rtRequest[i].read() == NONE)
			{
				rtReady[i].write(false);
			}
		}
		if (LOG >= 4)
			eventlog<<endl;
	}// end while
}// end allocate_route

//qrt************************************************************
	template<UI num_ip>
void Controller<num_ip>::create_est_flit(UI estimate,UI d, flit *est_out )   //CODEFLIT
{

	est_out->simdata.gtime = sc_time_stamp(); //CODENR
	/*	flit_out->simdata.ctime = sc_time_stamp();//CODENR
			flit_out->simdata.atime = SC_ZERO_TIME;//CODENR
			flit_out->simdata.atimestamp = 0;//CODENR
			flit_out->simdata.num_waits = 0;//CODENR
			flit_out->simdata.num_sw = 0;*///CODENR
	est_out->pkthdr.esthdr.d = d;
	est_out->pkthdr.esthdr.estimate = estimate;
}



/*template<UI num_ip>
	void Controller<num_ip>::store_flit(flit *flit_in) {
	if(ctrQ.full == true) {
	if(LOG >= 2)
	eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Error(QRT): DATA has arrived. ctrQ is full!"<<endl;
	cout<<"Controller Est Dropped Queue Full: estpkt id :"<<flit_in->pkthdr.esthdr.pktID <<"Src :"<<flit_in->pkthdr.esthdr.src<<endl;
	}
	else
	{
	ctrQ.flit_in(*flit_in);
	rs.est_out++;
	}
	}*/


//qrt************************************************************

	template<UI num_ip>
UI Controller<num_ip>::get_avgest()
{
	ULL total_est = 0;
	int i;
	for (i = 0; i < rtable->num_tiles; i++)
	{
		total_est += rtable->get_estimate(i);
	}

	return (double)total_est/i;

}


///Returns the reverse direction
	template<UI num_ip>
UI Controller<num_ip>::reverseDir(UI dir)
{
	if (dir == N)
		return S;
	else if (dir == S)
		return N;
	else if (dir == E)
		return W;
	else if (dir == W)
		return E;
}

//qrt************************************************************




///////////////////////////////////////////////////////////////////////////
/// Method to assign tile IDs and port IDs
///////////////////////////////////////////////////////////////////////////
	template<UI num_ip>
void Controller<num_ip>::setTileID(UI id, UI port_N, UI port_S, UI port_E, UI port_W)
{
	tileID = id;
	portN = port_N;
	portS = port_S;
	portE = port_E;
	portW = port_W;
	rtable->setID(id);
}


/////////////////////////////////////////////////////////////////////
/// Returns direction (N, S, E, W) corresponding to a given port id
////////////////////////////////////////////////////////////////////
	template<UI num_ip>
UI Controller<num_ip>::idToDir(UI port_id)
{
	if (port_id == portN)
		return N;
	else if (port_id == portS)
		return S;
	else if (port_id == portE)
		return E;
	else if (port_id == portW)
		return W;
	return C;
}

/////////////////////////////////////////////////////////////////////
/// Returns id corresponding to a given port direction (N, S, E, W)
////////////////////////////////////////////////////////////////////
	template<UI num_ip>
UI Controller<num_ip>::dirToId(UI port_dir)
{
	switch (port_dir)
	{
		case N:
			return portN;
			break;
		case S:
			return portS;
			break;
		case E:
			return portE;
			break;
		case W:
			return portW;
			break;
		case C:
			return num_ip - 1;
			break;
	}
	return num_ip - 1;
}

template struct Controller<NUM_IC>;
template struct Controller<NUM_IC_B>;
template struct Controller<NUM_IC_C>;
