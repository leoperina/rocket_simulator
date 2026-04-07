function [f] = internal_balistics(~, y, ENG, ENGfnc)
%INTERNAL_BALISTICS function determining the internal balistics of the
% rocket engine. 
% The elements of the state space vector are: chamber pressure, grain
% regression, m_propellant_exited

% Pc = max(y(1), 1e4);                                                        % Pa
Pc = y(1);
x  = y(2);                                                                  % m

%_______________________________________________________________ grain data
if x < ENG.xfin
    Ab = ENGfnc.Ab(x);
    rb = ENG.a * Pc.^ENG.n;
    Vc = ENGfnc.Vc(x, ENG.L);
else
    Ab = 0;
    rb = 0;
    Vc = ENGfnc.Vc(ENG.xfin, ENG.L);
end

%__________________________________________________________ propellant data
Pcbar = Pc / 1e5;
gamma = ENGfnc.g(Pcbar);
if Pcbar > max(ENG.Pbar) * 2
    warning('Pressure is over double the maximum tabulated pressure')
    fprintf('Pc [bar] at the moment: %f bar\n', Pcbar)
    fprintf('Maximum tabulated pressure: %f bar\n', max(ENG.Pbar))
    warning('Warning! There is also the possibility that this is a rejected step by the ode() function')
end

PR = min(101325 / Pc, 0.999);
choked = 1/PR > ( (gamma + 1)/2 )^(gamma/(gamma-1));

if choked
    cstar = ENGfnc.cs(Pcbar);
    mdot  = Pc * ENG.At / cstar;
else
    R = ENGfnc.R(Pcbar);
    T = ENGfnc.Tc(Pcbar);
    
    mdot = ENG.At * Pc * sqrt(gamma/(R*T)) * ...
           PR^(1/gamma) * ...
           sqrt(2/(gamma-1) * (1 - PR^((gamma-1)/gamma)));
end



mdot_gen = ENG.rhop * rb * Ab;
Rt = ENGfnc.R(Pcbar) * ENGfnc.Tc(Pcbar);

f = zeros(3, 1);


f(1) = Rt / Vc * (mdot_gen - mdot);
f(2) = rb;
f(3) = -mdot;
end

