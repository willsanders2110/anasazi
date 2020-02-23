/* Household.h
Anasazi Model
Group 2, 6/11/19 */

#ifndef HOUSEHOLD
#define HOUSEHOLD

#include <stdio.h>
#include <vector>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"


class Household {
private:
	repast::AgentId householdID;
	int age;
	int maizeFieldID;
	int removalAge;
	int yearCreated;
	int infertilityAge;
	double fissionProb;

	int maizeYield;
	int maizeStock;
	int estimatedYield;
	int yearRemoved;
	int maxMaizeStock;
	int maizeConsumption;
	int maizeLosses;
	
	std::vector<int> householdPosition;
	std::vector<int> maizeFieldPosition;

	
public:
	Household(repast::AgentId ID, int maizeField, int Age, std::vector<int> Position, int ageForRemoval, int yearMade, int infertileAge, double fissionProbability, int initMaize);
	~Household();
	
	/* Required Getters */
	virtual repast::AgentId& getId() { return householdID; }
	virtual const repast::AgentId& getId() const { return householdID; }
	
	/* Actions */
	int getMaizeStock() { return maizeStock; }
	int getAge() { return age; }
	int getMaizeFieldID() { return maizeFieldID; }
	int getRemovalAge() { return removalAge; }
	int getInfertilityAge() { return infertilityAge; }
	int getEstimatedYield() { return estimatedYield; }
	double getFissionProb() {return fissionProb; }
	std::vector<int> getHouseholdPosition() { return householdPosition; }
	std::vector<int> getMaizeFieldPosition() { return maizeFieldPosition; }

	void setAge(int newAge) { age = newAge; }
	void setMaizeYield(int newMaizeYield) { maizeYield = newMaizeYield; }
	void setHouseholdPosition(std::vector<int> newPos) { householdPosition = newPos; }
	void setMaizeFieldPosition(std::vector<int> newPos) { maizeFieldPosition = newPos; }
	
	void estimateMaizeYield();
	void calculateMaizeStock();
	
};

#endif