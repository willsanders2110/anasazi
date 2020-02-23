include ./env

.PHONY: create_folders
create_folders:
	mkdir -p objects
	mkdir -p bin

.PHONY: clean_compiled_files
clean_compiled_files:
	rm -f ./objects/*.o
	rm -f ./bin/*.exe
	
.PHONY: clean
clean:
	rm -rf objects
	rm -rf bin

.PHONY: compile
compile: clean_compiled_files
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/main.cpp -o ./objects/main.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/LongHouseValley.cpp -o ./objects/LongHouseValley.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Household.cpp -o ./objects/Household.o

	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/props_create.cpp -o ./objects/props_create.o

	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/output_calibration.cpp -o ./objects/output_calibration.o

	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/new_plot_calibrated_graphs.cpp -o ./objects/new_plot_calibrated_graphs.o

	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/main.exe ./objects/main.o ./objects/LongHouseValley.o ./objects/Household.o $(REPAST_HPC_LIB) $(BOOST_LIBS)
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/props_create.exe ./objects/props_create.o $(REPAST_HPC_LIB) $(BOOST_LIBS)
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/output_calibration.exe ./objects/output_calibration.o $(REPAST_HPC_LIB) $(BOOST_LIBS)
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/new_plot_calibrated_graphs.exe ./objects/new_plot_calibrated_graphs.o $(REPAST_HPC_LIB) $(BOOST_LIBS)
	
.PHONY: all
all: clean create_folders compile