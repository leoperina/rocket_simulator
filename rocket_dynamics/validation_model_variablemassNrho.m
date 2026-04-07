function [f] = validation_model_variablemassNrho(t, y, MSL, g, rhofun, Fc)
%VALIDATION_MODEL Modello delle EDO di moto atto a validare il modello


% xp   = y(1);
vx   = y(2);
yp   = y(3);
vy   = y(4);
th   = y(5);
thd  = y(6);
mp   = y(7);
beta = atan2(vy,vx);
U2   = vx^2 + vy^2;
cl   = MSL.cl(th);
cd   = MSL.cd(th);

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

