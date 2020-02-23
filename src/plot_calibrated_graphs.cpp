#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <cmath>
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include <fstream>
#include <sstream>

using namespace std;

int main(void){
	system("rm calibration/results.csv");

	int numMins = 5;
	int numMinCount = 0;

	std::vector<double> calibration_data;
	std::string value;
	std::vector<int> target_data;

	std::ofstream results_write;
	std::string fileName = "calibration/results.csv";
	results_write.open(fileName, std::ios_base::app);

	// read target file
    std::ifstream file;
    std::string calibration_file = "calibration/calibration_output.csv";
    file.open(calibration_file.c_str());
    if (!file.is_open())
    	std::cout << "ERROR: File Open\n";

    std::string line;
    while (getline(file, line)){
	    std::stringstream ss(line);
	    while(getline(ss, value,'\n') && file.good()){
	        calibration_data.push_back(repast::strToDouble(value));
	    }
	}

	// read target file
	std::ifstream file_3;
	std::string target_file = "target_data.csv";
	file_3.open(target_file.c_str());
	if (!file_3.is_open())
		std::cout << "ERROR: File Open\n";

	while (getline(file_3, line)){
	    std::stringstream ss(line);
	    while(getline(ss, value,',') && file_3.good()){
	        target_data.push_back(repast::strToInt(value));
	    }
	}

	std::vector< std::vector<int> > resultsData;

	while (numMinCount < numMins){
		int line_num = 0;
		std::vector<double> calibration_data_2;

		int minElementIndex = std::min_element(calibration_data.begin(),calibration_data.end()) - calibration_data.begin();
		double minElement = *std::min_element(calibration_data.begin(), calibration_data.end());

		// read target file
		std::ifstream file_2;
		std::string calibration_file_2 = "calibration/calibration_output.csv";
		file_2.open(calibration_file_2.c_str());
		if (!file_2.is_open())
			std::cout << "ERROR: File Open\n";

		while (getline(file_2, line)){
		    std::stringstream ss(line);
		    if (line_num==minElementIndex){
		    	while(getline(ss, value,',') && file_2.good()){
			        calibration_data_2.push_back(repast::strToDouble(value));
			    }
		    }
		    line_num++;
		}
		std::ofstream props_write;
		std::string fileName = "props/model.props";
		props_write.open(fileName);
		std::vector<std::string> data;
		data.clear();

		// write to props file
		props_write << "lower.death.age = " << calibration_data_2[2] << endl;
		props_write << "upper.death.age = " << calibration_data_2[3] << endl;
		props_write << "lower.infertility.age = " << calibration_data_2[4] << endl;
		props_write << "upper.infertility.age = " << calibration_data_2[5] << endl;
		props_write << "lower.fission.prob = " << calibration_data_2[6] << endl;
		props_write << "upper.fission.prob = " << calibration_data_2[7] << endl;
		props_write << "mean.maize.harvest = " << calibration_data_2[8] << endl;
		props_write << "variance.maize.harvest = " << calibration_data_2[9] << endl;

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

		system("mpirun bin/main.exe props/config.props props/model.props");
		
		// read target file
		std::ifstream file_3;
		std::string target_file = "number_of_households.csv";
		file_3.open(target_file.c_str());
		if (!file_3.is_open())
			std::cout << "ERROR: File Open\n";

		vector<int> outputData;
		while (getline(file_3, line)){
		    std::stringstream ss(line);
		    while(getline(ss, value,',') && file_3.good()){
		        outputData.push_back(repast::strToInt(value));
		    }
		}

		calibration_data.erase(calibration_data.begin()+minElementIndex);

		numMinCount++;

		resultsData.push_back(outputData);
	}

	for (int j=0; j<550; j++){
		results_write << target_data[j] << ",";
		for (int i=0; i<numMins; i++){
			results_write << resultsData[i][j] << ",";
		}
		results_write << "\n";
	}

	results_write.close();
	return 0;
}