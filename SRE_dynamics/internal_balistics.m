function [f] = internal_balistics(~, y, DATA)
%INTERNAL_BALISTICS function determining the internal balistics of the
% rocket engine. 
% The elements of the state space vector are: chamber pressure, grain
% regression, m_propellant_exited

% Pc = max(y(1), 1e4);                                                        % Pa
Pc = y(1);
x  = y(2);                                                                  % m

%_______________________________________________________________ grain data
if x < DATA.xfin
    Ab = DATA.Ab(x);
    rb = DATA.a * Pc.^DATA.n;
    Vc = DATA.Vc(x, DATA.L);
else
    Ab = 0;
    rb = 0;
    Vc = DATA.Vc(DATA.xfin, DATA.L);
end

%__________________________________________________________ propellant data
Pcbar = Pc / 1e5;
gamma = DATA.g(Pcbar);
if Pcbar > max(DATA.Pbar) * 2
    warning('Pressure is over double the maximum tabulated pressure')
    fprintf('Pc [bar] at the moment: %f bar\n', Pcbar)
    fprintf('Maximum tabulated pressure: %f bar\n', max(DATA.Pbar))
    warning('Warning! There is also the possibility that this is a rejected step by the ode() function')
end

PR = min(101325 / Pc, 0.999);
choked = 1/PR > ( (gamma + 1)/2 )^(gamma/(gamma-1));

if choked
    cstar = DATA.cs(Pcbar);
    mdot  = Pc * DATA.At / cstar;
else
    R = DATA.R(Pcbar);
    T = DATA.Tc(Pcbar);
    
    mdot = DATA.At * Pc * sqrt(gamma/(R*T)) * ...
           PR^(1/gamma) * ...
           sqrt(2/(gamma-1) * (1 - PR^((gamma-1)/gamma)));
end



mdot_gen = DATA.rhop * rb * Ab;
Rt = DATA.R(Pcbar) * DATA.Tc(Pcbar);

f = zeros(3, 1);


f(1) = Rt / Vc * (mdot_gen - mdot);
f(2) = rb;
f(3) = -mdot;
end

