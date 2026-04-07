clear
close all
clc

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% filenames %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
FILENAMES.propl_data = "meshNpropellant/propellant/totrash.txt";
% FILENAMES.grain_data = "meshNpropellant/grain_mesh/try_graphs.txt";
FILENAMES.grain_data = "meshNpropellant/grain_mesh/star_data_graphs.txt";
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%__________________________________ transforming data into useful functions
ENG.propl = load(FILENAMES.propl_data);
ENG.grain = load(FILENAMES.grain_data);                                    % already in SI unities


ENG.Ab   = @(x)    interp1(ENG.grain(:, 3), ENG.grain(:, 1), x);
ENG.Vc   = @(x, L) interp1(ENG.grain(:, 3), ENG.grain(:, 2), x).*L;
ENG.xfin = ENG.grain(end, 3);
ENG.DCC  = ENG.grain(1, 4);

ENG.Pbar  = ENG.propl(:, 1);                                              % bar
ENG.Tc  = @(P) interp1(ENG.Pbar, ENG.propl(:, 2), P);                    
ENG.g   = @(P) interp1(ENG.Pbar, ENG.propl(:, 3), P);
ENG.gs  = @(P) interp1(ENG.Pbar, ENG.propl(:, 4), P);
ENG.R   = @(P) interp1(ENG.Pbar, ENG.propl(:, 5), P);
ENG.cs  = @(P) interp1(ENG.Pbar, ENG.propl(:, 6), P);
ENG.rho = @(P) interp1(ENG.Pbar, ENG.propl(:, 7), P);

ENG.L      = 1;                                                              % length of the CC (m)
ENG.Vc_dry = ENG.L * (ENG.DCC^2/4 * pi);
ENG.rhop   = 1750;                                                           % Kg/m3
ENG.At     = 0.05;                                                            % m2
ENG.mp     = ENG.rhop * (ENG.Vc_dry - ENG.Vc(ENG.grain(1, 3), ENG.L));

ENG.a    = 7e-5;                                                           % m/s/Pa
ENG.n    = 0.4;



%% integration testing
clc
y0 = [101325; ENG.grain(1, 3); ENG.mp];

[t, y] = ode45(@(t, y) internal_balistics(t, y, ENG), [0, 5], y0);

figure('Color',[1, 1, 1])
plot(t, y(:, 1) / 1e5)
xlabel('Time - t [s]')
ylabel('Pressure - p [bar]')

figure('Color',[1, 1, 1])
plot(t, y(:, 2))
xlabel('Time - t [s]')
ylabel('Regression - x [m]')

figure('Color',[1, 1, 1])
plot(t, y(:, 3))
xlabel('Time - t [s]')
ylabel('Expelled mass - m [Kg]')
