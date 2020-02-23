/* Household.cpp
Anasazi Model
Group 2, 6/11/19 */

#include "Household.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"
#include <stdio.h>
#include <vector>

Household::Household(repast::AgentId ID, int maizeField, int Age, std::vector<int> Position, int ageForRemoval, int yearMade, int infertileAge, double fissionProbability, int initMaize) {
	householdID = ID;
	maizeFieldID = maizeField;
	age = Age;
	householdPosition = Position;
	removalAge = ageForRemoval;
	yearCreated = yearMade;
	infertilityAge = infertileAge;
	fissionProb = fissionProbability;
	maizeStock = initMaize;

	maizeYield = 0;
	estimatedYield = 0;
	yearRemoved = 0;
	maizeFieldPosition.assign({0, 0});
	maxMaizeStock = 1600;
	maizeConsumption = 800;
	maizeLosses = 10;
}

Household::~Household() {}

// estimates the Maize Yield produced in the following year
void Household::estimateMaizeYield(){
	estimatedYield = maizeStock + maizeYield;
}

// adds the new Maize Yield to the existing Stock 
void Household::calculateMaizeStock(){
	// 'bucket' solution as stock builds up over years/goes out of date
	maizeStock = maizeStock + maizeYield - maizeConsumption - maizeLosses;
	if (maizeStock > maxMaizeStock)
		maizeStock = maxMaizeStock;
}