/* LongHouseValley.h
Anasazi Model
Group 2, 6/11/19 */

#ifndef LONGHOUSEVALLEY
#define LONGHOUSEVALLEY

#include <vector>
#include <map>
#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"
#include "repast_hpc/AgentRequest.h"
#include "repast_hpc/TDataSource.h"
#include "repast_hpc/SVDataSet.h"
#include "Household.h"

class LongHouseValley{
private:

	// from props file
	int boardSize_x;
	int boardSize_y;

	int startYear;
	int endYear;
	int countOfAgents;

	int upperRandAge;
	int lowerRandAge;
	int upperDeathAge;
	int lowerDeathAge;
	int deathAgeChangeYear;
	
	int fertileAge;
	int upperInfertilityAge;
	int lowerInfertilityAge;
	double upperFissionProb;
	double lowerFissionProb;

	int minimumMaize;
	int upperInitMaize;
	int lowerInitMaize;
	double meanMaizeHarvest;
	double varianceMaizeHarvest;
	int harvestAdjLevel;
	int soilQuality;
	double householdFieldDistance;
	double householdWaterDistance;

	int counter = 0;

	int repastRank;
	int current_year;
	int nextAgentID;
	int numAgentsAlive;
	std::vector<Household*> agentList;
	std::map<std::string, int> maizeZoneColumn;
	int yieldTable3[5][4] = {617, 514, 411, 642, 719, 599, 479, 749, 821, 684, 547, 855, 988, 824, 659, 1030, 1153, 961, 769, 1201};
	std::ofstream outputFile;
	std::ofstream numHouseholdsFile;
	std::ofstream numHouseholdsCSV;
	int numHouseholds[551];
	bool stopModel;

	repast::Properties* props;
	repast::SharedContext<Household> context;
	repast::SharedDiscreteSpace<Household, repast::StrictBorders, repast::SimpleAdder<Household> >* discreteSpace;

	std::vector<std::vector<int> > maizeShortlist;
	std::vector<std::vector<int> > waterShortlist;

	void makeNewAgent();
	void updateAgentList();
	void removeHousehold(repast::AgentId idToRemove);
	void fissionHousehold();
	void queryRemovalHousehold();
	void queryFissionDecision();
	void generateMaizeYields(int year);
	void defineWaterSources(int year);
	void shortlistMaizeWater();
	void chooseLocation(repast::AgentId iD);
	
	void open_map_file(std::string filename_map);
    void open_sett_file(std::string filename_sett);
    void open_water_file(std::string filename_water);
    void open_pdsi_file(std::string filename_pdsi);
    void open_hydro_file(std::string filename_hydro);

    // map.csv
    std::vector<int> map_x;
    std::vector<int> map_y;
    std::vector<std::string> map_colour;
    std::vector<std::string> map_zone;
    std::vector<std::string> map_maize_zone;

    // settlement.csv
    // std::vector<int> sett_m_north;
    // std::vector<int> sett_m_east;
    // std::vector<int> sett_hist_start;
    // std::vector<int> sett_hist_end;
    // std::vector<int> sett_median_date;
    // std::vector<int> sett_x_coord;
    // std::vector<int> sett_y_coord;

    // water.csv
    std::vector<int> water_ID;
    std::vector<int> water_m_north;
    std::vector<int> water_m_east;
    std::vector<int> water_conditions;
    std::vector<int> water_hist_start;
    std::vector<int> water_hist_end;
    std::vector<int> water_x_coord;
    std::vector<int> water_y_coord;

    // pdsi.csv
    std::vector<int> pdsi_date;
    std::vector<float> pdsi_general;
    std::vector<float> pdsi_north;
    std::vector<float> pdsi_mid;
    std::vector<float> pdsi_natural;
    std::vector<float> pdsi_upland;
    std::vector<float> pdsi_kinbiko;

    // hydro.csv
    std::vector<int> hydro_date;
    std::vector<float> hydro_general;
    std::vector<float> hydro_north;
    std::vector<float> hydro_mid;
    std::vector<float> hydro_natural;
    std::vector<float> hydro_upland;
    std::vector<float> hydro_kinbiko;


public:

	struct map_info
		{
			std::string zone;
			std::string maize_zone;
			int water;
			int maizeFieldID;
			int maizeYield;
		};

	map_info map[120][80] = {};

	LongHouseValley(std::string propsFile, int testNum, int argc, char** argv, boost::mpi::communicator* comm);
	~LongHouseValley();

	void readArchaeologicalData();
	void initAgents();
	void initSchedule(repast::ScheduleRunner& runner);
	void doPerTick();

};

#endif
