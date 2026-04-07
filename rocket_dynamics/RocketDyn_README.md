# READ ME
This file contains all the useful informations about the code in the rocket dynamics model.

The structure historically divides into 3 parts:
1) validation model
2) validation model with variable mass and rho changes
3) simulink implementation

## validation model
This is historically the first validation model.
It contains the dynamical model where a magical force pushes the rocket with a fixed mass.
Compared to the more sophisticated model, it only has 6 states in the state vector, because mass is supposed constant.

## validation model with variable mass and rho
This model is more sophisticated, it accounts for mass variations and variation in air density.
This model is still not the most sophisticated because it does not account for variations in the c_d and c_l in vicinity of Mach 1.
Also, it does not even try to model aeroelasticity in any form.

## Simulink implementation
This is still under construction, it is a translation into Simulink of the model of point 2)


