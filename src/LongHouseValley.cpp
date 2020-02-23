/* LongHouseValley.cpp
Anasazi Model
Group 2, 6/11/19 */

#include <stdio.h>
#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>
#include <random>
#include <boost/mpi.hpp>
#include <chrono>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"
#include "repast_hpc/Random.h"
#include "LongHouseValley.h"

// LongHouseValley class constructor
LongHouseValley::LongHouseValley(std::string propsFile, int testNum, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);

	boardSize_x = repast::strToInt(props->getProperty("board.size.x"));
	boardSize_y = repast::strToInt(props->getProperty("board.size.y"));
	
	startYear = repast::strToInt(props->getProperty("start.year"));
	endYear = repast::strToInt(props->getProperty("end.year"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	
	
	upperRandAge = repast::strToInt(props->getProperty("upper.random.age"));
	lowerRandAge = repast::strToInt(props->getProperty("lower.random.age"));
	upperDeathAge = repast::strToInt(props->getProperty("upper.death.age"));
	lowerDeathAge = repast::strToInt(props->getProperty("lower.death.age"));

	deathAgeChangeYear = repast::strToInt(props->getProperty("death.age.change.year"));

	fertileAge = repast::strToInt(props->getProperty("fertile.age"));
	upperInfertilityAge = repast::strToInt(props->getProperty("upper.infertility.age"));
	lowerInfertilityAge = repast::strToInt(props->getProperty("lower.infertility.age"));
	upperFissionProb = repast::strToDouble(props->getProperty("upper.fission.prob"));
	lowerFissionProb = repast::strToDouble(props->getProperty("lower.fission.prob"));

	minimumMaize = repast::strToInt(props->getProperty("minimum.maize"));
	upperInitMaize = repast::strToInt(props->getProperty("upper.init.maize"));
	lowerInitMaize = repast::strToInt(props->getProperty("lower.init.maize"));
	meanMaizeHarvest = repast::strToDouble(props->getProperty("mean.maize.harvest"));
	varianceMaizeHarvest = repast::strToDouble(props->getProperty("variance.maize.harvest"));
	harvestAdjLevel = repast::strToInt(props->getProperty("harvest.adj.level"));
	soilQuality = repast::strToInt(props->getProperty("soil.quality"));
	householdFieldDistance = repast::strToDouble(props->getProperty("household.field.distance"));
	householdWaterDistance = repast::strToDouble(props->getProperty("household.water.distance"));

	maizeZoneColumn["\"Empty\""] = -1;
	maizeZoneColumn["\"No_Yield\""] = -1;
	maizeZoneColumn["\"Yield_1\""] = 0;
	maizeZoneColumn["\"Yield_2\""] = 1;
	maizeZoneColumn["\"Yield_3\""] = 2;
	maizeZoneColumn["\"Sand_dune\""] = 3;

    repast::Point<double> origin(1,1); 
    repast::Point<double> extent(boardSize_x+1,boardSize_y+1); 
    repast::GridDimensions gd(origin, extent);
    initializeRandom(*props, comm);

    std::vector<int> processDims; 
    processDims.push_back(1); 
    processDims.push_back(1); 

    discreteSpace = new repast::SharedDiscreteSpace<Household,repast::StrictBorders, repast::SimpleAdder<Household> >("AgentDiscreteSpace", gd, processDims, 0, comm);
	context.addProjection(discreteSpace);

	outputFile.open("model_output.txt");
    
    	numHouseholdsFile.open("output");
	numHouseholdsCSV.open("number_of_households.csv");
	repastRank = repast::RepastProcess::instance()->rank();
	current_year = startYear;
	nextAgentID = 0;
	numAgentsAlive = 0;
	numHouseholds[551] = {};
	stopModel = false;
	outputFile << "Simulation beginning..." << endl;
	outputFile << "Repast rank: " << repastRank << "\nMap bounds: " << discreteSpace->bounds().origin() << " to " << discreteSpace->bounds().extents() << endl << endl; 

}

// LongHouseValley class destructor
LongHouseValley::~LongHouseValley(){
	delete props;
	outputFile << "\nModel complete - simulation terminated." << endl;
	outputFile.close();
	numHouseholdsCSV.close();
	numHouseholdsFile.close();
}

// initialise the agents in the world
void LongHouseValley::initAgents(){
	outputFile << "Map initialising..." << endl << endl;
	// Loops over all cells and place the relevant data from maize and region into it
	for (int i=0; i <  map_x.size(); i++){
		map_info currentStruct;

		currentStruct.zone = map_zone[i];
		currentStruct.maize_zone = map_maize_zone[i];
		currentStruct.maizeFieldID = 0;
		currentStruct.maizeYield = 0;
		currentStruct.water = 0;

		map[map_x[i]][map_y[i]] = currentStruct;
	}

	defineWaterSources(current_year);
	
	shortlistMaizeWater();
	
	generateMaizeYields(current_year);

	for (int i=0; i<countOfAgents; i++){
		makeNewAgent();
	}

	updateAgentList();

	for (std::vector<Household*>::iterator agent = agentList.begin(); agent != agentList.end(); ++agent){
		chooseLocation((*agent)->getId());
	}

	outputFile << "\n...map initialised" << endl;
}

// iterative loop that runs the model
void LongHouseValley::doPerTick(){
	
	if (!stopModel){
		//if (current_year == 900)
		//	lowerDeathAge = 0;

		if (numAgentsAlive > 400)
			stopModel = true;
			
		outputFile << "\nYear: " << current_year << endl;
		cout << "Year: " << current_year << " Number of agents alive: " << numAgentsAlive << endl;
		numHouseholds[current_year-startYear] = numAgentsAlive;
		outputFile << "Number of agents alive: " << numHouseholds[current_year-startYear] << endl;
		numHouseholdsFile << numHouseholds[current_year-startYear];
		if (current_year != endYear)
			numHouseholdsFile << " ";

		numHouseholdsCSV << numHouseholds[current_year-startYear];
		if (current_year != endYear)
			numHouseholdsCSV << "\n";
		
		// creates shortlist for maize and water
		shortlistMaizeWater();

		// increase age of agents
		updateAgentList();
		for (std::vector<Household*>::iterator agent = agentList.begin(); agent != agentList.end(); ++agent){
			(*agent)->setAge((*agent)->getAge() + 1);
		}
		
		generateMaizeYields(current_year);
		
		updateAgentList();
		for (std::vector<Household*>::iterator agent = agentList.begin(); agent != agentList.end(); ++agent){
			std::vector <int> maizeLoc = context.getAgent((*agent)->getId())->getMaizeFieldPosition();
			(*agent)->setMaizeYield(map[maizeLoc[0]][maizeLoc[1]].maizeYield);
			(*agent)->calculateMaizeStock();
			(*agent)->estimateMaizeYield();
		}

		queryRemovalHousehold();
		if (stopModel)
			return;
		
		updateAgentList();
		//cout << agentList.size() << " " << context.size() << endl;
		for (std::vector<Household*>::iterator agent = agentList.begin(); agent != agentList.end(); ++agent){
			if ((*agent)->getEstimatedYield() < (*agent)->getMaizeStock()){
				chooseLocation((*agent)->getId());
			}
		}

		queryFissionDecision();

		if (current_year != endYear)
			defineWaterSources(current_year+1);

	}
	else {
		if (counter < 1){
			outputFile << "Model stopped at year: " << current_year << endl;
			cout << "Model stopped at year: " << current_year << endl;
			counter =1;
		}
		numHouseholdsFile << numHouseholds[current_year-startYear];
		if (current_year != endYear)
			numHouseholdsFile << " ";
		
		numHouseholdsCSV << numHouseholds[current_year-startYear];
		if (current_year != endYear)
			numHouseholdsCSV << "\n";
	}
	current_year++;
}

// initialise the schedule
void LongHouseValley::initSchedule(repast::ScheduleRunner& runner){
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<LongHouseValley> (this, &LongHouseValley::doPerTick)));
	runner.scheduleStop((endYear - startYear));
}

void LongHouseValley::makeNewAgent(){
	nextAgentID++;
	numAgentsAlive++;
	
	repast::IntUniformGenerator randXCoord = repast::Random::instance()->createUniIntGenerator(1, boardSize_x);
	repast::IntUniformGenerator randYCoord = repast::Random::instance()->createUniIntGenerator(1, boardSize_y);
	int X = randXCoord.next();
	int Y = randYCoord.next();
	repast::Point<int> householdLoc(X, Y);
	std::vector <int> position = {X, Y};
	repast::AgentId newID(nextAgentID, repastRank, 0);

	repast::IntUniformGenerator ageForRemoval = repast::Random::instance()->createUniIntGenerator(lowerDeathAge, upperDeathAge);
	repast::IntUniformGenerator initMaize = repast::Random::instance()->createUniIntGenerator(lowerInitMaize, upperInitMaize);
	repast::IntUniformGenerator infertileAge = repast::Random::instance()->createUniIntGenerator(lowerInfertilityAge, upperInfertilityAge);
	repast::DoubleUniformGenerator fissionProbability = repast::Random::instance()->createUniDoubleGenerator(lowerFissionProb, upperFissionProb);
	int Age = 0;
	if (current_year == startYear){
		repast::IntUniformGenerator randAge = repast::Random::instance()->createUniIntGenerator(lowerRandAge, upperRandAge);
		Age = randAge.next();
	}

	Household* household = new Household(newID, nextAgentID, Age, position, ageForRemoval.next(), current_year, infertileAge.next(), fissionProbability.next(), initMaize.next());
	context.addAgent(household);
	discreteSpace->moveTo(newID, householdLoc);

	outputFile << "Household created: " << nextAgentID << endl;
}

void LongHouseValley::updateAgentList(){
	if (!stopModel){
		agentList.clear();
		context.selectAgents(repast::SharedContext<Household>::LOCAL, numAgentsAlive, agentList);
	}
}

// fission a household
void LongHouseValley::fissionHousehold(){	 
	makeNewAgent();
	repast::AgentId newID(nextAgentID, repastRank, 0);
	chooseLocation(newID);
}

// remove a household
void LongHouseValley::removeHousehold(repast::AgentId idToRemove){
	std::vector <int> maizePos = context.getAgent(idToRemove)->getMaizeFieldPosition();
	map[maizePos[0]][maizePos[1]].maizeFieldID = 0;
	context.removeAgent(idToRemove);
	delete context.getAgent(idToRemove);
	numAgentsAlive--;
	if (numAgentsAlive <= 0)
		stopModel = true;
}

// determine whether to remove a household
void LongHouseValley::queryRemovalHousehold(){
	// loop through all agents
	if (!stopModel){
		updateAgentList();
		for (std::vector<Household*>::iterator agent = agentList.begin(); agent != agentList.end(); ++agent){
	   		if ((*agent)->getAge() >= (*agent)->getRemovalAge()){
	   			removeHousehold((*agent)->getId());
	   			outputFile << "Household " << (*agent)->getMaizeFieldID() << " removed: reached death age" << endl;
	   		}
	   		else if ((*agent)->getMaizeStock() < minimumMaize){
	   			removeHousehold((*agent)->getId());
	   			outputFile << "Household " << (*agent)->getMaizeFieldID() << " removed: ran out of food" << endl;
	   		}
	   	}
	}
}

// determine whether and where possible to fission a household
void LongHouseValley::queryFissionDecision(){
	// loop through all agents
	if (!stopModel){
		updateAgentList();
		for (std::vector<Household*>::iterator agent = agentList.begin(); agent != agentList.end(); ++agent){
	   		if (((*agent)->getAge() >= fertileAge) && ((*agent)->getAge() < (*agent)->getInfertilityAge())){
	   			repast::DoubleUniformGenerator fission = repast::Random::instance()->createUniDoubleGenerator(0, 1);
	   			if (fission.next() < (*agent)->getFissionProb()){
	   				fissionHousehold();
	   			}
	   		}
	    }
	}
}

// generate the maize yeilds each iteration
void LongHouseValley::generateMaizeYields(int year){
	// loop through all cells
	for (int x=0; x<boardSize_x; x++){
		for (int y=0; y<boardSize_y; y++){
			std::string maizeZone = map[x][y].maize_zone;
			maizeZone = maizeZone.substr(0, maizeZone.size()-1); // remove weird character at end of string
			int indexCol = maizeZoneColumn[maizeZone];
			
			int yield = 0;
			if (indexCol >= 0){
				float pdsi = 0;
				std::string mapZone = map[x][y].zone;

				if (mapZone == "\"General\"")
					pdsi = pdsi_general[year-startYear];
				else if (mapZone == "\"North\"" || mapZone == "\"North Dunes\"")
					pdsi = pdsi_north[year-startYear];
				else if (mapZone == "\"Mid\"" || mapZone == "\"Mid Dunes\"")
					pdsi = pdsi_mid[year-startYear];
				else if (mapZone == "\"Natural\"")
					pdsi = pdsi_natural[year-startYear];
				else if (mapZone == "\"Uplands\"")
					pdsi = pdsi_upland[year-startYear];
				else if (mapZone == "\"Kinbiko\"")
					pdsi = pdsi_kinbiko[year-startYear];

				int indexRow = 0;
				if (pdsi <= -3)
					indexRow = 0;
				else if (pdsi <= -1)
					indexRow = 1;
				else if (pdsi < 1)
					indexRow = 2;
				else if (pdsi < 3)
					indexRow = 3;
				else
					indexRow = 4;	
				
				yield = yieldTable3[indexRow][indexCol];
				int baseYield = yield * soilQuality * harvestAdjLevel;
				// probability mean and variance of harvest
				std::default_random_engine generator;
  				std::normal_distribution<double> distribution(meanMaizeHarvest, sqrt(varianceMaizeHarvest));
				double probability = distribution(generator);
				map[x][y].maizeYield = (int)(baseYield * probability);
				//if (x == 11 & y ==21)
				//	cout << "yield: " << yield << " pdsi: " << pdsi << " " << indexRow << " " << indexCol << " " << maizeZone << " " << mapZone << endl;
			}
		}
	}
}

// define the water sources each iteration
void LongHouseValley::defineWaterSources(int year){
	int existStreams;
	int existAlluvium;
	int water_source;
	std::string zone;

	// If the current year lies between these values, then streams exist within the model
	if ((year >= 800 && year < 930) || (year >= 1300 && year < 1450))
		existStreams = 1;
	else
		existStreams = 0;

	// If the current year lies between these values, then streams exist within the model
	if (((year >= 980) && (year < 1120)) || ((year >= 1180) && (year < 1230)))
		existAlluvium = 1;
	else
		existAlluvium = 0;

	// If the year lies between these values, then streams exist within the model
	for (int x_coord=0; x_coord<boardSize_x; x_coord++){
		for (int y_coord=0; y_coord<boardSize_y; y_coord++){
			water_source = 0;
			zone = map[x_coord][y_coord].zone;

			if (((zone == "\"General\"") || (zone == "\"North\"") || (zone == "\"Mid\"") || (zone == "\"Kinbiko\"")) && (existAlluvium == 1))
				water_source = 1;

			// If the year lies between these values, then streams exist within the model
			if ((existStreams == 1) && (zone == "\"Kinbiko\""))
				water_source = 1;

			if ((x_coord == 72 && y_coord == 114) || (x_coord == 70 && y_coord == 113) || (x_coord == 69 && y_coord == 112) || 
				(x_coord == 68 && y_coord == 111) || (x_coord == 67 && y_coord == 110) || (x_coord == 66 && y_coord == 109) || 
				(x_coord == 65 && y_coord == 108) || (x_coord == 65 && y_coord == 107)){ 
				water_source = 1;
			}
			map[x_coord][y_coord].water = water_source;
		}
	}

	// water-type, water-x, water-y are the column names, read from file 
	for (int i=0; i<water_conditions.size(); i++) { 
		if (water_conditions[i] == 2)
			map[water_y_coord[i]][water_x_coord[i]].water = 1; //the location with matching x,y coor has water source
		if ((water_conditions[i] == 3) && ((year >= startYear) && (year <= endYear)))
			map[water_y_coord[i]][water_x_coord[i]].water = 1;
	}
}

void LongHouseValley::shortlistMaizeWater(){
	maizeShortlist.clear();
	waterShortlist.clear();
	for (int x=0; x<boardSize_x; x++){
		for (int y=0; y<boardSize_y; y++){
			// Adding maize field to maize field shortlist
			if ((map[x][y].maizeFieldID == 0))
				maizeShortlist.push_back({0, x, y});

			// Adding water source to water source shortlist
			if (map[x][y].water == 1)
				waterShortlist.push_back({x, y});
		}
	}
}

// using namespace std::chrono;
// auto start = high_resolution_clock::now();

// auto stop = high_resolution_clock::now(); 
// auto duration = duration_cast<microseconds>(stop - start);
// cout << "make new agent time: ";
// cout << duration.count() << endl; 


// choose a new location for a maize field/household
// CHANGE WATER CONDITIONS TO HYDRO DATA - NOT WATER DATA
void LongHouseValley::chooseLocation(repast::AgentId ID){
	if (!stopModel){
		float distance = 0;
		float prevShortDist = 9999;
		int shortestDistInd = 0;
		
		// agent chooses new maize field location:
		// loop through all cells:
		std::vector <std::vector <int> > shortlistX;
		// if cell empty && expected maize of cell > 800
		for (int i=0; i<maizeShortlist.size(); i++){
			if ((maizeShortlist[i][0] == 0) && (map[maizeShortlist[i][1]][maizeShortlist[i][2]].maizeYield + context.getAgent(ID)->getMaizeStock() > minimumMaize)){
				// shortlistX cell
				shortlistX.push_back({maizeShortlist[i][1], maizeShortlist[i][2]});
				maizeShortlist[i][0] == 1;
			}
		}
		if (shortlistX.size() == 0){
			outputFile << "Household " << context.getAgent(ID)->getMaizeFieldID() << " removed: left the valley" << endl;
			removeHousehold(ID);
			return;
		}
		// calculate distance between agent and each shortlistX cell
		std::vector <int> agentPos = context.getAgent(ID)->getHouseholdPosition();
		for (int i=0; i<shortlistX.size(); i++){
			distance = sqrt(pow((agentPos[0]-shortlistX[i][0]), 2) + pow((agentPos[1]-shortlistX[i][1]), 2));
			if (distance < prevShortDist){
				prevShortDist = distance;
				shortestDistInd = i;
			}
		}
		// remove old maize field from map
		std::vector <int> oldMaizeLoc = context.getAgent(ID)->getMaizeFieldPosition();
		map[oldMaizeLoc[0]][oldMaizeLoc[1]].maizeFieldID = 0;
		// choose closest cell as new maize field
		std::vector <int> newMaizeLoc = shortlistX[shortestDistInd];
		map[newMaizeLoc[0]][newMaizeLoc[1]].maizeFieldID = context.getAgent(ID)->getMaizeFieldID();
		context.getAgent(ID)->setMaizeFieldPosition(newMaizeLoc);
		outputFile << "Household " << context.getAgent(ID)->getMaizeFieldID() << " moved its maize field" << endl;

		// ======================================
		// agent chooses new location for itself:
		std::vector <int> newHouseholdLoc;
		std::vector <std::vector <int> > shortlistA;
		// loop through all cells:
		for (int i=0; i<maizeShortlist.size(); i++){
			// if cell is not a field, shortlistA cell
			if (maizeShortlist[i][0] == 0){
				// shortlistA cell
				shortlistA.push_back({maizeShortlist[i][1], maizeShortlist[i][2]});
			}
		}
		// loop through shortlistA:
		std::vector <std::vector <int> > shortlistB;
		for (int i=0; i<shortlistA.size(); i++){
			distance = sqrt(pow((newMaizeLoc[0]-shortlistA[i][0]), 2) + pow((newMaizeLoc[1]-shortlistA[i][1]), 2));
			// if cell within 1km of its field, shortlistB cell
			if (distance < householdFieldDistance){
				shortlistB.push_back(shortlistA[i]);
			}
		}
		// if shortlistB empty, choose from shortlistA closest to water
		distance = 0;
		prevShortDist = 9999;
		shortestDistInd = 0;
		if (shortlistB.size() == 0){
			for (int i=0; i<shortlistA.size(); i++){
				for (int j=0; j<waterShortlist.size(); j++){
					distance = sqrt(pow((waterShortlist[j][0] - shortlistA[i][0]), 2) + pow((waterShortlist[j][1] - shortlistA[i][1]), 2));
					if (distance < prevShortDist){
						prevShortDist = distance;
						shortestDistInd = i;
					}
				}
			}
			// set new agentPos on map and in agent itself
			newHouseholdLoc = shortlistA[shortestDistInd];
			context.getAgent(ID)->setHouseholdPosition(newHouseholdLoc);
			// set location of agent in discrete space
			repast::Point<int> newRepastLoc(newHouseholdLoc[0]+1, newHouseholdLoc[1]+1);
			discreteSpace->moveTo(ID, newRepastLoc);
			outputFile << "Household " << context.getAgent(ID)->getMaizeFieldID() << " moved further than 1km nearest water" << endl;
			return;
		}
		// loop through shortlistB:
		std::vector <std::vector <int> > shortlistC;
		for (int i=0; i<shortlistB.size(); i++){
			// if cell potential yield > its field potential yield, shortlist C cell
			if (map[shortlistB[i][0]][shortlistB[i][1]].maizeYield > map[newMaizeLoc[0]][newMaizeLoc[1]].maizeYield){
				shortlistC.push_back(shortlistB[i]);
			}
		}
		// if shortlistC empty, choose from shortlistB closest to water
		if (shortlistC.size() == 0){
			for (int i=0; i<shortlistB.size(); i++){
				for (int j=0; j<waterShortlist.size(); j++){
					distance = sqrt(pow((waterShortlist[j][0] - shortlistB[i][0]), 2) + pow((waterShortlist[j][1] - shortlistB[i][1]), 2));
					if (distance < prevShortDist){
						prevShortDist = distance;
						shortestDistInd = i;
					}
				}
			}
			// set new agentPos on map and in agent itself
			newHouseholdLoc = shortlistB[shortestDistInd];
			context.getAgent(ID)->setHouseholdPosition(newHouseholdLoc);
			// set location of agent in discrete space
			repast::Point<int> newRepastLoc(newHouseholdLoc[0]+1, newHouseholdLoc[1]+1);
			discreteSpace->moveTo(ID, newRepastLoc);
			outputFile << "Household " << context.getAgent(ID)->getMaizeFieldID() << " moved within 1km nearest water" << endl;
			return;
		}
		// else choose from shortlistC closest to water
		else {
			for (int i=0; i<shortlistC.size(); i++){
				for (int j=0; j<waterShortlist.size(); j++){
					distance = sqrt(pow((waterShortlist[j][0] - shortlistC[i][0]), 2) + pow((waterShortlist[j][1] - shortlistC[i][1]), 2));
					if (distance < prevShortDist){
						prevShortDist = distance;
						shortestDistInd = i;
					}
				}
			}
			// set new agentPos on map and in agent itself
			newHouseholdLoc = shortlistC[shortestDistInd];
			context.getAgent(ID)->setHouseholdPosition(newHouseholdLoc);
			// set location of agent in discrete space
			repast::Point<int> newRepastLoc(newHouseholdLoc[0]+1, newHouseholdLoc[1]+1);
			discreteSpace->moveTo(ID, newRepastLoc);
			outputFile << "Household " << context.getAgent(ID)->getMaizeFieldID() << " moved within 1km with good yield nearest water" << endl;
		}
	}
}


// read the data files at initialisation and store in structs, return true if successful
void LongHouseValley::readArchaeologicalData(){
	// calling the functions to open the files
	open_map_file("map.csv");
    //open_sett_file("settlement.csv");
    open_water_file("water.csv");
    open_pdsi_file("pdsi.csv");
    open_hydro_file("hydro.csv");    
}

void LongHouseValley::open_map_file(std::string filename){

    filename = "data/" + filename;
	std::ifstream fin(filename.c_str());                 // as data is in subfolder                             // inputting file in
	std::string line;

    int nLines = 0;
	int nCol = 0;

	if (!fin.is_open()) {                                                                // if file is not open
		std::cout << "failed to open file: " << filename << "\n\n";
	}
	else {
		while (std::getline(fin, line)) {   // looping through lines of file
			std::stringstream  ss(line);
			std::string        cell;
			int temp_int;
			nCol = 0;

			while (std::getline(ss, cell, ',')) {     //dividing each line between commas


				if (nCol == 0 && nLines>0) {                            // Pushing back all string values which are at the column position specified
					std::stringstream(cell) >> temp_int;
					map_x.push_back(temp_int);                  // converting the values if not strings and pushing back to vectors
                }else if(nCol == 1 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    map_y.push_back(temp_int);
                }else if(nCol == 2 && nLines>0){
                    map_colour.push_back(cell);
                }else if(nCol == 3 && nLines>0){
                    map_zone.push_back(cell);
                }else if(nCol == 4 && nLines>0){
                    map_maize_zone.push_back(cell);
                } nCol += 1;
            } nLines += 1;

        }
   	}
}

/*
void LongHouseValley::open_sett_file(std::string filename){
	
    filename = "data/" + filename;                         // as data is in subfolder
	std::ifstream fin(filename.c_str());                                           // inputting file in
	std::string line;                                       // declaring empty string

    int nLines = 0;
	int nCol = 0;

	if (!fin.is_open()) {                                                                // if file is not open
		std::cout << "failed to open file: " << filename << "\n\n";
	}
	else {

		while (std::getline(fin, line)) {   // looping through lines of file


			std::stringstream  ss(line);
			std::string        cell;
			int temp_int;
			nCol = 0;

			while (std::getline(ss, cell, ',')) {     //diving each line between commas

				if (nCol == 0 && nLines>0) {                            // Pushing back all string values which are at the column position specified
					std::stringstream(cell) >> temp_int;
					sett_m_north.push_back(temp_int);                  // converting the values and pushing back to vectors
                }else if(nCol == 1 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    sett_m_east.push_back(temp_int);
                }else if(nCol == 2 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    sett_hist_start.push_back(temp_int);
                }else if(nCol == 3 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    sett_hist_end.push_back(temp_int);
                }else if(nCol == 4 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    sett_median_date.push_back(temp_int);
                }else if(nCol == 5 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    sett_x_coord.push_back(temp_int);
                }else if(nCol == 6 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    sett_y_coord.push_back(temp_int);
                }
                 nCol += 1;
            } nLines += 1;

        }
      }
}
*/

void LongHouseValley::open_water_file(std::string filename){

    filename = "data/" + filename;
	std::ifstream fin(filename.c_str());                                           // inputting file in
	std::string line;

    int nLines = 0;
	int nCol = 0;

	if (!fin.is_open()) {                                                                // if file is not open
		std::cout << "failed to open file: " << filename << "\n\n";
	}
	else {

		while (std::getline(fin, line)) {   // looping through lines of file


			std::stringstream  ss(line);
			std::string        cell;
			int temp_int;
			nCol = 0;

			while (std::getline(ss, cell, ',')) {     //diving each line between commas

				if (nCol == 0 && nLines>0) {                            // Pushing back all string values which are at the column position specified
					std::stringstream(cell) >> temp_int;
					water_ID.push_back(temp_int);                  // converting the values and pushing back to vectors
                }else if(nCol == 1 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    water_m_north.push_back(temp_int);
                }else if(nCol == 2 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    water_m_east.push_back(temp_int);
                }else if(nCol == 3 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    water_conditions.push_back(temp_int);
                }else if(nCol == 4 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    water_hist_start.push_back(temp_int);
                }else if(nCol == 5 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    water_hist_end.push_back(temp_int);
                }else if(nCol == 6 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    water_x_coord.push_back(temp_int);
                }else if(nCol == 7 && nLines>0){
                    std::stringstream(cell) >> temp_int;   // converting string to int
                    water_y_coord.push_back(temp_int);
                } nCol += 1;
            } nLines += 1;

        }
    }
}

void LongHouseValley::open_pdsi_file(std::string filename){

    filename = "data/" + filename;
	std::ifstream fin(filename.c_str());                                           // inputting file in
	std::string line;

    int nLines = 0;
	int nCol = 0;

	if (!fin.is_open()) {                                                                // if file is not open
		std::cout << "failed to open file: " << filename << "\n\n";
	}
	else {

		while (std::getline(fin, line)) {   // looping through lines of file


			std::stringstream  ss(line);
			std::string        cell;
			int temp_int;
			float temp_float;
			nCol = 0;

			while (std::getline(ss, cell, ',')) {     //diving each line between commas

				if (nCol == 0 && nLines>0) {                            // Pushing back all string values which are at the column position specified
					std::stringstream(cell) >> temp_int;
					pdsi_date.push_back(temp_int);                  // converting the values if not strings and pushing back to vectors
                }else if(nCol == 1 && nLines>0){
                    std::stringstream(cell) >> temp_float;   // converting string to float
                    pdsi_general.push_back(temp_float);
                }else if(nCol == 2 && nLines>0){
                    std::stringstream(cell) >> temp_float;
                    pdsi_north.push_back(temp_float);
                }else if(nCol == 3 && nLines>0){
                    std::stringstream(cell) >> temp_float;
                    pdsi_mid.push_back(temp_float);
                }else if(nCol == 4 && nLines>0){
                    std::stringstream(cell) >> temp_float;
                    pdsi_natural.push_back(temp_float);
                }else if(nCol == 5 && nLines>0){
                    std::stringstream(cell) >> temp_float;
                    pdsi_upland.push_back(temp_float);
                }else if(nCol == 6 && nLines>0){
                    std::stringstream(cell) >> temp_float;
                    pdsi_kinbiko.push_back(temp_float);
                }nCol += 1;
            } nLines += 1;

        }
	}
}

void LongHouseValley::open_hydro_file(std::string filename){

    filename = "data/" + filename;
	std::ifstream fin(filename.c_str());                                           // inputting file in
	std::string line;

    int nLines = 0;
	int nCol = 0;

	if (!fin.is_open()) {                                                                // if file is not open
		std::cout << "failed to open file: " << filename << "\n\n";
	}
	else {

		while (std::getline(fin, line)) {   // looping through lines of file


			std::stringstream  ss(line);
			std::string        cell;
			int temp_int;
			float temp_float;
			nCol = 0;

			while (std::getline(ss, cell, ',')) {     //diving each line between commas

				if (nCol == 0 && nLines>0) {                            // Pushing back all string values which are at the column position specified
					std::stringstream(cell) >> temp_int;
					hydro_date.push_back(temp_int);                  // converting the values to floats and pushing back to vectors
                }else if(nCol == 1 && nLines>0){
                    std::stringstream(cell) >> temp_float;   // converting string to int
                   hydro_general.push_back(temp_float);
                }else if(nCol == 2 && nLines>0){
                    std::stringstream(cell) >> temp_float;   // converting string to int
                    hydro_north.push_back(temp_float);
                }else if(nCol == 3 && nLines>0){
                    std::stringstream(cell) >> temp_float;   // converting string to int
                    hydro_mid.push_back(temp_float);
                }else if(nCol == 4 && nLines>0){
                    std::stringstream(cell) >> temp_float;   // converting string to int
                    hydro_natural.push_back(temp_float);
                }else if(nCol == 5 && nLines>0){
                    std::stringstream(cell) >> temp_float;   // converting string to int
                    hydro_upland.push_back(temp_float);
                }else if(nCol == 6 && nLines>0){
                    std::stringstream(cell) >> temp_float;   // converting string to int
                    hydro_kinbiko.push_back(temp_float);
                }nCol += 1;
            } nLines += 1;
        }
   	}
}
