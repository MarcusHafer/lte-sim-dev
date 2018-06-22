
#ifndef HETEROOGENEOUS_PAYDA_H_
#define HETEROOGENEOUS_PAYDA_H_


#include "../channel/LteChannel.h"
#include "../phy/enb-lte-phy.h"
#include "../phy/ue-lte-phy.h"
#include "../core/spectrum/bandwidth-manager.h"
#include "../networkTopology/Cell.h"
#include "../protocolStack/packet/packet-burst.h"
#include "../protocolStack/packet/Packet.h"
#include "../core/eventScheduler/simulator.h"
#include "../flows/application/InfiniteBuffer.h"
#include "../flows/application/VoIP.h"
#include "../flows/application/CBR.h"
#include "../flows/application/TraceBased.h"
#include "../device/IPClassifier/ClassifierParameters.h"
#include "../flows/QoS/QoSParameters.h"
#include "../flows/QoS/QoSForEXP.h"
#include "../flows/QoS/QoSForFLS.h"
#include "../flows/QoS/QoSForM_LWDF.h"
#include "../componentManagers/FrameManager.h"
#include "../utility/seed.h"
#include "../utility/RandomVariable.h"
#include "../phy/wideband-cqi-eesm-error-model.h"
#include "../phy/simple-error-model.h"
#include "../channel/propagation-model/macrocell-urban-area-channel-realization.h"
#include "../load-parameters.h"
#include <queue>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <iostream>

#include <iostream>
#include <queue>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <math.h>
#include <iostream>

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>


static void Heterogeneous_PayDA_Scenario(int interferer)
{
	bool DEBUG = false;

	double simulationTime = 300.01;

	// APPLICATIONS
	int nb_volt_control = 1;
	int nb_crash_avoidance = 1;
	int nb_rt_iperf = 1;
	int nb_nrt_iperf = interferer;

	// Start and stop times
	double RT_start = 0.1;
	double RT_end = simulationTime;

	double NRT_start = 0.1;
	double NRT_end = simulationTime;

	// iPerf RT Application Parameters
	double iperfRT_datasize = 33333;
	double iperfRT_interval = 0.1;
	double iperfRT_maxDelay = 0.1;

	// iPerf NRT Application Parameters
	double iperfNRT_datasize = 3333300;
	double iperfNRT_interval = 1.0;
	double iperfNRT_maxDelay = 0.3;

	// Volt Control Application Parameters
	double voltControl_datasize = 5000.0/8.0;
	double voltControl_interval = 1.0;
	double voltControl_maxDelay = 0.05;

	// Crash Avoidance Application Parameters
	double crashAvoidance_datasize = 4000.0/8.0;
	double crashAvoidance_interval = 0.05;
	double crashAvoidance_maxDelay = 0.1;


	//////////// SCENARIO ////////////

	if (interferer >= 0)
	{
		cout << "Simulation run with " << interferer << " interferers"<< endl;
	}

	int nbUEs_RT = 0;
	int nbUEs_NRT = 0;

	double cell_size = 1.0;
	double cell_bandwidth = 5.0;


	// mobility model
	enum Mobility::MobilityModel mobility = Mobility::CONSTANT_POSITION;
	int speed = 0;


	// Human-To-Human communication
	///////////////////////////////

	int rt_iperf, rt_volt_control, crash_avoidance, rt_voip;

	rt_iperf = nb_rt_iperf;
	rt_volt_control = nb_volt_control;
	crash_avoidance = nb_crash_avoidance;

	rt_voip = interferer;

	// count RT users
	int sum_rt_H2H = 0;
	sum_rt_H2H = rt_iperf + rt_volt_control + crash_avoidance;
	nbUEs_RT += sum_rt_H2H;

	int nrt_iperf = 0;

	nrt_iperf = nb_nrt_iperf;

	int sum_nrt_H2H = 0;
	sum_nrt_H2H = nrt_iperf;
	nbUEs_NRT += sum_nrt_H2H;


	// check bf of users
	cout << "\n\nOverall RT and NRT users: " << std::endl;
	cout << "nbUEs_RT: " << nbUEs_RT << std::endl;
	cout << "nbUEs_NRT: " << nbUEs_NRT << "\n\n" << std::endl;

	// update number of total UEs for simulator
	Simulator *simulator = Simulator::Init();

	// GENERATE NEW SEED
	time_t t;
	time(&t);
	srand((unsigned int)t);


	// CREATE COMPONENT MANAGERS
	NetworkManager* networkManager = NetworkManager::Init();

	//Create CHANNELS
	LteChannel *dlCh = new LteChannel ();
	LteChannel *ulCh = new LteChannel ();

	// CREATE SPECTRUM
	BandwidthManager* spectrum = new BandwidthManager (cell_bandwidth, cell_bandwidth, 0, 0);

	// CREATE CELL
	int idCell = 0;
	int minDistance = 0.0035; //km
	int posX = 0;
	int posY = 0;
	Cell* cell = networkManager->CreateCell (idCell, cell_size, minDistance, posX, posY);

	// CREATE ENODEB
	int idEnb = 1;
	ENodeB* enb = networkManager->CreateEnodeb (idEnb, cell, posX, posY, dlCh, ulCh, spectrum);
	enb->SetDLScheduler (ENodeB::DLScheduler_TYPE_PROPORTIONAL_FAIR);
	enb->SetULScheduler(ENodeB::ULScheduler_TYPE_MAXIMUM_THROUGHPUT);

	//Create GW
	Gateway *gw = networkManager->CreateGateway ();
	gw->SetIDNetworkNode(200);

	int lastRandVal = 0;

	std::vector<int> appsofUEs;

	for (int a = 0; a < nbUEs_RT + nbUEs_NRT; a++)
	{
		appsofUEs.push_back(0);
	}


	// save start and end time for each application
	struct app_start_end_time
	{
		 double start;
		 double end;
	} app;


	app_start_end_time apps[nbUEs_RT + nbUEs_NRT];
	for (int i=0; i<nbUEs_RT + nbUEs_NRT; i++)
	{
		apps[i] = app;
	}


	// SAVE CURRENT TIME FOR SAVE FILE
	time_t timer;
	time (&timer);
	std::string s_timer = ctime(&timer);

	// remove last character from string
	s_timer = s_timer.substr(0, s_timer.size()-1);


	int idUe = 100;
	int applicationID = 0;
	int srcPort = 0;
	int dstPort = 100;
	int idQoS = 1;


	// CREATE UEs
	for (int currentUE = 0; currentUE < (nbUEs_RT + nbUEs_NRT); currentUE++)
	{
		// use different pseudo random numbers for each UE
		while (lastRandVal == std::rand())
		{
			sleep(0.1);
		}

		lastRandVal = std::rand();

		// overwrite start and end times for RT applications
		if (currentUE < nbUEs_RT)
		{
			apps[currentUE].start = RT_start + (rand() / (float)RAND_MAX);
			apps[currentUE].end = RT_end;
		}
		else
		{
			apps[currentUE].start = NRT_start + rand() / (float)RAND_MAX;
			apps[currentUE].end = NRT_end;
		}

		cout << "Current start time: " << apps[currentUE].start << std::endl;
				cout << "Current end time: " << apps[currentUE].end << std::endl;


		// random position
		double posX, posY = 0.0;
		double speedDirection = 0;

		// uniform distribution of users in cell
		vector<CartesianCoordinates*>* newPosition = GetUniformUsersDistribution (cell->GetIdCell(), 1);

		posX = newPosition->front()->GetCoordinateX();
		posY = newPosition->front()->GetCoordinateY();

		int randX = (int) (std::rand() % 100);
		int randY = (int) (std::rand() % 100);

		if ((randX % 2) == 0)
			posX = posX * (-1);

		if ((randY % 2) == 0)
			posY = posY * (-1);


		// CREATE USER EQUIPMENT
		UserEquipment* ue = new UserEquipment (idUe, posX, posY, speed, speedDirection,cell, enb, 0, mobility);
		ue->GetPhy ()->SetDlChannel (enb->GetPhy()->GetDlChannel ());
		ue->GetPhy ()->SetUlChannel (enb->GetPhy()->GetUlChannel ());

		FullbandCqiManager *cqiManager = new FullbandCqiManager ();
		cqiManager->SetCqiReportingMode (CqiManager::PERIODIC);
		cqiManager->SetReportingInterval (1);
		cqiManager->SetDevice (ue);
		ue->SetCqiManager (cqiManager);

		enb->RegisterUserEquipment (ue);

		MacroCellUrbanAreaChannelRealization* c_dl = new MacroCellUrbanAreaChannelRealization (enb, ue);
		enb->GetPhy()->GetDlChannel ()->GetPropagationLossModel ()->AddChannelRealization (c_dl);
		MacroCellUrbanAreaChannelRealization* c_ul = new MacroCellUrbanAreaChannelRealization (ue, enb);
		enb->GetPhy()->GetUlChannel ()->GetPropagationLossModel ()->AddChannelRealization (c_ul);


		networkManager->GetUserEquipmentContainer ()->push_back (ue);

		ue->GetPhy ()->GetDlChannel ()->AddDevice (ue);
		ue->GetPhy ()->GetUlChannel ()->AddDevice (enb);


		// CREATE APPLICATIONS
		QoSParameters* qos = new QoSParameters ();

		ClassifierParameters *cp = new ClassifierParameters;
		cp->SetSourceID(enb->GetIDNetworkNode());
		cp->SetDestinationID(ue->GetIDNetworkNode());
		cp->SetSourcePort(dstPort);
		cp->SetDestinationPort(srcPort);
		cp->SetTransportProtocol(TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP);

		bool application_attached_to_UE = false;

		std::cout << "\n\nUE " << idUe << ": " << apps[currentUE].start << "," << apps[currentUE].end << "," << posX << "," << posY << "," <<
														speedDirection << std::endl;

		// RT USERS
		if (currentUE < nbUEs_RT)
		{
			if (!application_attached_to_UE)
			{

				if (rt_voip > 0 && (!application_attached_to_UE))
				{
					VoIP* voip = new VoIP();

					voip->SetQoSParameters(qos);
					voip->SetApplicationID(applicationID);
					voip->SetStartTime(apps[currentUE].start);
					voip->SetStopTime(apps[currentUE].end);
					voip->SetSource(enb);
					voip->SetDestination(ue);
					voip->SetSourcePort(dstPort);
					voip->SetDestinationPort(srcPort);
					voip->SetClassifierParameters(cp);

					qos->SetMaxDelay(0.1);
					cout << "rt_voip application created!" << std::endl;
					cout << "Start time: " << voip->GetStartTime() << std::endl;
					cout << "Stop time: " << voip->GetStopTime() << std::endl;
					cout << "Size: " << voip->GetSize() << std::endl;
					cout << "MaxDelay: " << voip->GetQoSParameters()->GetMaxDelay() << std::endl;

					rt_voip--;

					// application is attached
					application_attached_to_UE = true;
				}

				if (rt_iperf > 0 && (!application_attached_to_UE))
				{
					CBR *smartApp = new CBR();

					smartApp->SetQoSParameters(qos);
					smartApp->SetApplicationID(applicationID);
					smartApp->SetStartTime(apps[currentUE].start);
					smartApp->SetStopTime(apps[currentUE].end);
					smartApp->SetSource(enb);
					smartApp->SetDestination(ue);
					smartApp->SetSourcePort(dstPort);
					smartApp->SetDestinationPort(srcPort);
					smartApp->SetClassifierParameters(cp);

					smartApp->SetInterval(iperfRT_interval);
					smartApp->SetSize(iperfRT_datasize);
					qos->SetMaxDelay(iperfRT_maxDelay);

					cout << "rt_iperf application created!" << std::endl;
					cout << "Start time: " << smartApp->GetStartTime() << std::endl;
					cout << "Stop time: " << smartApp->GetStopTime() << std::endl;
					cout << "Interval: " << smartApp->GetInterval() << std::endl;
					cout << "Size: " << smartApp->GetSize() << std::endl;
					cout << "MaxDelay: " << smartApp->GetQoSParameters()->GetMaxDelay() << std::endl;

					rt_iperf--;
					application_attached_to_UE = true;
				}

				if (rt_volt_control > 0 && (!application_attached_to_UE))
				{
					CBR *smartApp = new CBR();

					smartApp->SetQoSParameters(qos);
					smartApp->SetApplicationID(applicationID);
					smartApp->SetStartTime(apps[currentUE].start);
					smartApp->SetStopTime(apps[currentUE].end);
					smartApp->SetSource(enb);
					smartApp->SetDestination(ue);
					smartApp->SetSourcePort(dstPort);
					smartApp->SetDestinationPort(srcPort);
					smartApp->SetClassifierParameters(cp);

					smartApp->SetInterval(voltControl_interval);
					smartApp->SetSize(voltControl_datasize);

					qos->SetMaxDelay(voltControl_maxDelay);

					cout << "rt_volt_control application created!" << std::endl;
					cout << "Start time: " << smartApp->GetStartTime() << std::endl;
					cout << "Stop time: " << smartApp->GetStopTime() << std::endl;
					cout << "Interval: " << smartApp->GetInterval() << std::endl;
					cout << "Size: " << smartApp->GetSize() << std::endl;
					cout << "MaxDelay: " << smartApp->GetQoSParameters()->GetMaxDelay() << std::endl;

					rt_volt_control--;
					application_attached_to_UE = true;
				}

				if (crash_avoidance > 0 && (!application_attached_to_UE))
				{
					CBR *smartApp = new CBR();

					smartApp->SetQoSParameters(qos);
					smartApp->SetApplicationID(applicationID);
					smartApp->SetStartTime(apps[currentUE].start);
					smartApp->SetStopTime(apps[currentUE].end);
					smartApp->SetSource(enb);
					smartApp->SetDestination(ue);
					smartApp->SetSourcePort(dstPort);
					smartApp->SetDestinationPort(srcPort);
					smartApp->SetClassifierParameters(cp);

					smartApp->SetInterval(crashAvoidance_interval);
					smartApp->SetSize(crashAvoidance_datasize);

					qos->SetMaxDelay(crashAvoidance_maxDelay);

					cout << "crash_avoidance application created!" << std::endl;
					cout << "Start time: " << smartApp->GetStartTime() << std::endl;
					cout << "Stop time: " << smartApp->GetStopTime() << std::endl;
					cout << "Interval: " << smartApp->GetInterval() << std::endl;
					cout << "Size: " << smartApp->GetSize() << std::endl;
					cout << "MaxDelay: " << smartApp->GetQoSParameters()->GetMaxDelay() << std::endl;

					crash_avoidance--;
					application_attached_to_UE = true;
				}
			}
		}

		// NRT APPLICATIONS
		else if (currentUE >= nbUEs_RT && currentUE < (nbUEs_RT + nbUEs_NRT))
		{
			if (!application_attached_to_UE)
			{
				// iPerf
				if (nrt_iperf > 0 && (!application_attached_to_UE))
				{
					CBR *smartApp = new CBR();

					smartApp->SetQoSParameters(qos);
					smartApp->SetApplicationID(applicationID);
					smartApp->SetStartTime(apps[currentUE].start);
					smartApp->SetStopTime(apps[currentUE].end);
					smartApp->SetSource(enb);
					smartApp->SetDestination(ue);
					smartApp->SetSourcePort(dstPort);
					smartApp->SetDestinationPort(srcPort);
					smartApp->SetClassifierParameters(cp);

					smartApp->SetInterval(iperfNRT_interval);
					smartApp->SetSize(iperfNRT_datasize);

					qos->SetMaxDelay(iperfNRT_maxDelay);

					cout << "nrt_iperf application created!" << std::endl;
					cout << "Start time: " << smartApp->GetStartTime() << std::endl;
					cout << "Stop time: " << smartApp->GetStopTime() << std::endl;
					cout << "Interval: " << smartApp->GetInterval() << std::endl;
					cout << "Size: " << smartApp->GetSize() << std::endl;
					cout << "MaxDelay: " << smartApp->GetQoSParameters()->GetMaxDelay() << std::endl;

					nrt_iperf--;
					application_attached_to_UE = true;
				}
			}
		}

		idUe++;
		applicationID++;
		dstPort++;
		idQoS++;
	}

	if (DEBUG)
		exit(0);


	Simulator::Init()->SetStop(simulationTime);
	Simulator::Init()->Run();

}



#endif /* HETEROGENEOUS_PAYDA_ */
