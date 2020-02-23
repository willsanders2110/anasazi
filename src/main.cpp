/* main.cpp
Anasazi Model
Group 2, 6/11/19 */

#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "LongHouseValley.h"


int main(int argc, char** argv){
	
	std::string configFile = argv[1]; // The name of the configuration file is Arg 1
	std::string propsFile  = argv[2]; // The name of the properties file is Arg 2
	
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	repast::RepastProcess::init(configFile);
	
	LongHouseValley* model = new LongHouseValley(propsFile, 0, argc, argv, &world);
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	
	model->readArchaeologicalData();
	model->initAgents();
	model->initSchedule(runner);

	runner.run();

	delete model;
		
	repast::RepastProcess::instance()->done();
}