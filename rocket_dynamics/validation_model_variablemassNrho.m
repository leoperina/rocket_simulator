function [f] = validation_model_variablemassNrho(t, y, MSL, MSLfnc, g, rhofun, Fc)
%VALIDATION_MODEL_VARIABLEMASSNRHO it is a validation model, later
%translated in Simulink, to validate that everything works as it should.
% It is formatted to be compatible with ODE integrators and it takes as
% input:
%       t           time vector
%       y           state vector [x, vx, y (altitude), vy, theta, theta_dot,
%                                   propeller_mass]
%       MSL         struct containing some useful properties
%       g           constant, it's the gravity constants....
%       rhofun      function handle returning air density as a fun of
%                   altitude
%       Fc          function handle containing the control force as a
%                   function of time
% the output:
%       f           derivative of the state vector f = dy/dt
% 
% format:
%       [f] = validation_model_variablemassNrho(t, y, MSL, g, rhofun, Fc)


% xp   = y(1);
vx   = y(2);
yp   = y(3);
vy   = y(4);
th   = y(5);
thd  = y(6);
mp   = y(7);
beta = atan2(vy,vx);
U2   = vx^2 + vy^2;
cl   = MSLfnc.cl(th);
cd   = MSLfnc.cd(th);

rho = rhofun(yp);

m = MSL.ms + mp;


f = zeros(7, 1);
if mp > 0
    f(7) = MSL.mdot;
    T = MSL.T;
else
    f(7) = 0;
    T = 0;
end

f(1) = vx;
f(3) = vy;
f(5) = thd;

Fa   = 0.5 * rho * U2 * MSL.l * MSL.diam;

f(2) = ( -Fc(t)*sin( beta + th ) + T*cos( beta + th ) + ...
    Fa * (-cl*sin(beta) - cd*cos(beta))) * 1/m;
f(4) = (  Fc(t)*cos( beta + th ) + T*sin( beta + th ) + ...
    Fa * (cl*cos(beta)  - cd*sin(beta))) * 1/m - g;
f(6) = (-Fc(t)*MSL.lc - Fa*(cl*cos(th) + cd*sin(th))*MSL.la - MSL.thc*thd) * 1/MSL.J;


end

