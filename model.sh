
#run model from R

# rm props/model.props

# run input to props script
mpirun bin/props_create.exe

mpirun bin/main.exe props/config.props props/model.props

mpirun bin/output_calibration.exe
