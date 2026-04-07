clear
close all
clc

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% filenames %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
FILENAMES.propl_data = "meshNpropellant/propellant/totrash.txt";
% FILENAMES.grain_data = "meshNpropellant/grain_mesh/try_graphs.txt";
FILENAMES.grain_data = "meshNpropellant/grain_mesh/star_data_graphs.txt";
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%__________________________________ transforming data into useful functions
DATA.propl = load(FILENAMES.propl_data);
DATA.grain = load(FILENAMES.grain_data);                                    % already in SI unities


DATA.Ab   = @(x)    interp1(DATA.grain(:, 3), DATA.grain(:, 1), x);
DATA.Vc   = @(x, L) interp1(DATA.grain(:, 3), DATA.grain(:, 2), x).*L;
DATA.xfin = DATA.grain(end, 3);
DATA.DCC  = DATA.grain(1, 4);

DATA.Pbar  = DATA.propl(:, 1);                                              % bar
DATA.Tc  = @(P) interp1(DATA.Pbar, DATA.propl(:, 2), P);                    
DATA.g   = @(P) interp1(DATA.Pbar, DATA.propl(:, 3), P);
DATA.gs  = @(P) interp1(DATA.Pbar, DATA.propl(:, 4), P);
DATA.R   = @(P) interp1(DATA.Pbar, DATA.propl(:, 5), P);
DATA.cs  = @(P) interp1(DATA.Pbar, DATA.propl(:, 6), P);
DATA.rho = @(P) interp1(DATA.Pbar, DATA.propl(:, 7), P);

DATA.L      = 1;                                                              % length of the CC (m)
DATA.Vc_dry = DATA.L * (DATA.DCC^2/4 * pi);
DATA.rhop   = 1750;                                                           % Kg/m3
DATA.At     = 0.05;                                                            % m2
DATA.mp     = DATA.rhop * (DATA.Vc_dry - DATA.Vc(DATA.grain(1, 3), DATA.L));

DATA.a    = 7e-5;                                                           % m/s/Pa
DATA.n    = 0.4;



%% integration testing
clc
y0 = [101325; DATA.grain(1, 3); DATA.mp];

[t, y] = ode45(@(t, y) internal_balistics(t, y, DATA), [0, 5], y0);

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
