source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- array(c(10,20,30,40,50,60,70,80,90,100,110,120), dim=c(2,3,2))
bar <- foo[1,3,2]
bar
