function [f] = validation_model(t, y, MSL, g, rho, Fc)
%VALIDATION_MODEL Modello delle EDO di moto atto a validare il modello


% xp   = y(1);
vx   = y(2);
% yp   = y(3);
vy   = y(4);
th   = y(5);
thd  = y(6);
beta = atan2(vy,vx);
U2   = vx^2 + vy^2;
cl   = MSL.cl(th);
cd   = MSL.cd(th);

if t < MSL.tb
    T = MSL.T;
else
    T = 0;
end

f = zeros(6, 1);

f(1) = vx;
f(3) = vy;
f(5) = thd;

Fa   = 0.5 * rho * U2 * MSL.l * MSL.diam;

f(2) = ( -Fc(t)*sin( beta + th ) + T*cos( beta + th ) + ...
    Fa * (-cl*sin(beta) - cd*cos(beta))) * 1/MSL.m;
f(4) = (  Fc(t)*cos( beta + th ) + T*sin( beta + th ) + ...
    Fa * (cl*cos(beta)  - cd*sin(beta))) * 1/MSL.m - g;
f(6) = (-Fc(t)*MSL.lc - Fa*(cl*cos(th) + cd*sin(th))*MSL.la - MSL.thc*thd) * 1/MSL.J;

end

