/* Anasazi_model
Group 2 ACS6132 */

#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <cmath>
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"

using namespace std;

int main(void){
	std::ofstream props_write;
	std::string fileName = "props/model.props";
	props_write.open(fileName);
	std::vector<std::string> data;
	data.clear();

	// read input file
    std::ifstream file;
    std::string input_file = "input";
    file.open(input_file.c_str());
    if (!file.is_open())
    	std::cout << "ERROR: File Open\n";
    std::string value;

    while(getline(file, value,'\n') && file.good()){
        data.push_back(value);
    }

	// write to props file
	props_write << "lower.death.age = " << repast::strToInt(data[0]) << endl;
	props_write << "upper.death.age = " << repast::strToInt(data[1]) << endl;
	props_write << "lower.infertility.age = " << repast::strToInt(data[2]) << endl;
	props_write << "upper.infertility.age = " << repast::strToInt(data[3]) << endl;
	props_write << "lower.fission.prob = " << repast::strToDouble(data[4]) << endl;
	props_write << "upper.fission.prob = " << repast::strToDouble(data[5]) << endl;
	props_write << "mean.maize.harvest = " << repast::strToDouble(data[6]) << endl;
	props_write << "variance.maize.harvest = " << repast::strToDouble(data[7]) << endl;

	props_write << "global.random.seed = 2" << endl;
	props_write << "board.size.x = 120" << endl;
	props_write << "board.size.y = 80" << endl;
	props_write << "start.year = 800" << endl;
	props_write << "end.year = 1350" << endl;
	props_write << "count.of.agents = 14" << endl;
	props_write << "upper.random.age = 30" << endl;
	props_write << "lower.random.age = 10" << endl;
	props_write << "death.age.change.year = 850" << endl;
	props_write << "fertile.age = 16" << endl;
	props_write << "minimum.maize = 2" << endl;
	props_write << "upper.init.maize = 1600" << endl;
	props_write << "lower.init.maize = 1500" << endl;
	props_write << "harvest.adj.level = 1" << endl;
	props_write << "soil.quality = 1" << endl;
	props_write << "household.field.distance = 10" << endl;
	props_write << "household.water.distance = 10" << endl;

	props_write.close();
	
	return 0;
}