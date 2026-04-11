# READ ME

This project was a test to put together my thesis code, which was on the regression rate of solid propellants, with a 2D dynamics simulator.
Altogether, they generate a Simulink block which simulates the behaviour of a rocket with good approximation.

---

## PROS
* This code utilizes NASA CEA to simulate the shifting thermodynamics used in the system.
With the right corrections it can support virtually any (sensible) solid rocket fuel.
As a matter of facts, in the path<br>
*SRE_dynamics/thesis_code/CEA_DATA/*<br>
...there are some examples of fuel propellants with different aluminum concentrations and operating temperatures.
* With the right setup, this project returns a "black box" Simulink block that inputs the control force on the rocket and outputs all the sensible data about the rocket dynamics.

## CONS
* At this stage, this project is still a fesibility study:
  It does not handle 3D dynamics, there are not (**yet**) aerodynamic corrections for the aerodynamic coefficients in proximity of Mach 1, there is no aeroelasticity, the dynamics are overall a little simple.
* At this stage the Simulink is still in a development stage, i.e. to correctly work it still needs both the libraries to be open
* The validations are still ongoing (nevertheless the results are promising)
  
