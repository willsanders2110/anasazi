#Run Calibration Process

# library (EasyABC)
# 
# file.remove("calibration/calibration_output.csv")
# 
# lower_death_age = c("unif",25,35)
# upper_death_age = c("unif",30,40)
# lower_infertility_age = c("unif",16,30)
# upper_infertility_age = c("unif",25,35)
# lower_fission_prob = c("unif",0,0.1)
# upper_fission_prob = c("unif",0.05,0.25)
# mean_maize_harvest = c("unif",0,1)
# variance_maize_harvest = c("unif",0,1)
# anasazi_prior=list(lower_death_age, upper_death_age, lower_infertility_age, upper_infertility_age, lower_fission_prob, upper_fission_prob, mean_maize_harvest, variance_maize_harvest)
# 
# target_data <- read.csv(file='target_data.csv')
# tolerance=0.2
# 
# # runs the calibration
# ABC_sim<-ABC_rejection(model=binary_model("bash model.sh") , prior=anasazi_prior, prior_test="(X2>X1) && (X4>X3) && (X6>X5)", nb_simul=1000, summary_stat_target=c(target_data), tol=tolerance, use_seed = FALSE, seed_count = 0, n_cluster=1, verbose=FALSE, progress_bar = FALSE)
# #ABC_sim<-ABC_rejection(model=binary_model("bash model.sh") , prior=anasazi_prior, nb_simul=2, prior_test=NULL, summary_stat_target=NULL, tol=NULL, use_seed = FALSE, seed_count = 0, n_cluster=1, verbose=FALSE, progress_bar = FALSE)
# 
# system("mpirun bin/new_plot_calibrated_graphs.exe")


library (ggplot2)

# note to create graphs in R you need to have named variables - therefore this will not be an automatic process: need to go into the cvs file and add the name of the variable to the first line
# creates graphs
year.data <- read.csv(file='year_data.csv')
target.data <- read.csv(file='target_data.csv')
colnames(target.data) <- c("t_households")
# this line will need to change once you have the calibrated output file
calibration.data <- read.csv(file='number_of_households.csv')
colnames(calibration.data) <- c("c_households")
# merges the year with each data set
df1 <- data.frame(year.data, target.data)
df2 <- data.frame(year.data, calibration.data)

# opens png
png("images/calibration_plot.png")
# creates plot of target data and then overlays the calibration data
ggplot() + geom_line(data = df1, aes(x = year, y = t_households)) + geom_line(data= df2, aes(x = year, y=c_households, colour='red'))
# closes png
dev.off()



# create histogram of posterior distributions
posterior.data <- read.csv(file='calibration//calibration_output.csv')
colnames(posterior.data) <- c("RSME", "MAE", "lower_death_age", "upper_death_age", "lower_infertility", "upper_infertility", "lower_fission_prob", "upper_fission_prob", "mean_maize_harvest", "variance_maize_harvest")

png("images//lower_death_age.png")
ggplot() + geom_histogram(data= posterior.data, aes(x = lower_death_age), bins = 5)
dev.off()

png("images//upper_death_age.png")
ggplot() + geom_histogram(data= posterior.data, aes(x = upper_death_age), bins = 5)
dev.off()

png("images//lower_infertility.png")
ggplot() + geom_histogram(data= posterior.data, aes(x = lower_infertility), bins = 5)
dev.off()

png("images//upper_infertility.png")
ggplot() + geom_histogram(data= posterior.data, aes(x = upper_infertility), bins = 5)
dev.off()

png("images//lower_fission_prob.png")
ggplot() + geom_histogram(data= posterior.data, aes(x = lower_fission_prob), bins = 5)
dev.off()

png("images//upper_fission_prob.png")
ggplot() + geom_histogram(data= posterior.data, aes(x = upper_fission_prob), bins = 5)
dev.off()

png("images//mean_maize_harvest.png")
ggplot() + geom_histogram(data= posterior.data, aes(x = mean_maize_harvest), bins = 5)
dev.off()

png("images//variance_maize_harvest.png")
ggplot() + geom_histogram(data= posterior.data, aes(x = variance_maize_harvest), bins = 5)
dev.off()
