
#include "../mac-entity.h"
#include "../../packet/Packet.h"
#include "../../packet/packet-burst.h"
#include "../../../device/NetworkNode.h"
#include "../../../flows/radio-bearer.h"
#include "../../../protocolStack/rrc/rrc-entity.h"
#include "../../../flows/application/Application.h"
#include "../../../device/ENodeB.h"
#include "../../../protocolStack/mac/AMCModule.h"
#include "../../../phy/lte-phy.h"
#include "../../../core/spectrum/bandwidth-manager.h"
#include "../../../core/idealMessages/ideal-control-messages.h"
#include "../../../flows/QoS/QoSParameters.h"
#include "../../../flows/MacQueue.h"
#include "../../../utility/eesm-effective-sinr.h"
#include <algorithm>
#include "../../../componentManagers/NetworkManager.h"
#include "payda_dl.h"


PayDA_DL::PayDA_DL()
{
	cout << "PayDA_DL::PayDA_DL()" << std::endl;
//	exit(0);
	SetMacEntity (0);
	CreateFlowsToSchedule();
}

PayDA_DL::~PayDA_DL()
{
	cout << "ayDA_DL::~PayDA_DL()" << std::endl;
	exit(0);
	Destroy ();
}

double
PayDA_DL::ComputeSchedulingMetric (RadioBearer *bearer, double spectralEfficiency, int subChannel)
{

	cout << "PayDA_DL::ComputeSchedulingMetric()" << std::endl;
	exit(0);

	double metric = 0.0;
	Application *app = bearer->GetApplication();
	QoSParameters *app_qos = app->GetQoSParameters();
	int netNode = bearer->GetSource()->GetIDNetworkNode();

	if (bearer->GetMacQueue()->GetQueueSize() > 0)
	{
		double now = Simulator::Init()->Now();
		double timestamp = bearer->GetMacQueue ()->Peek ().GetTimeStamp();
		double deadline = timestamp + app_qos->GetMaxDelay();
		double timeToDeadline = deadline - now;
		double maxDelay = bearer->GetQoSParameters()->GetMaxDelay();


		// data specific
		int transmittedBytes = bearer->GetTransmittedBytes();
		double avgTxRate = bearer->GetAverageTransmissionRate();
		int leftData = bearer->GetQueueSize();

		if (leftData <= 0)
		{
			leftData = 1;
		}

		metric = 1/(timeToDeadline * leftData);

		// IMPORVEMENT: NO NEGATIVE METRIC IF CheckForDropPackets NOT USED
		if (metric < 0.0)
		{
			metric = 0.0;
		}
	}

	return metric;
}

//void
//PayDA_DL::DoSchedule ()
//{
//	cout << "PayDA_DL::DoSchedule ()" << std::endl;
//	exit(0);
//
//#ifdef SCHEDULER_DEBUG
//	std::cout << "Start LOG RULE packet scheduler for node "
//			<< GetMacEntity ()->GetDevice ()->GetIDNetworkNode()<< std::endl;
//#endif
//
//  UpdateAverageTransmissionRate ();
//
//  CheckForDLDropPackets ();
//
//  SelectFlowsToSchedule ();
//
//  if (GetFlowsToSchedule ()->size() == 0)
//	{}
//  else
//	{
//	  RBsAllocation ();
//	}
//
//  StopSchedule ();
//}



