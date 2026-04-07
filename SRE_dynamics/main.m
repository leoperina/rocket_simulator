clear
close all
clc

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% filenames %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
FILENAMES.propl_data = "meshNpropellant/propellant/totrash.txt";
FILENAMES.grain_data = "meshNpropellant/grain_mesh/try_graphs.txt";
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%__________________________________ transforming data into useful functions
DATA.propl = load(FILENAMES.propl_data);
DATA.grain = load(FILENAMES.grain_data);

DATA.Ab = @(x)    interp1(DATA.grain(:, 3), DATA.grain(:, 1), x);
DATA.Vc = @(x, L) interp1(DATA.grain(:, 3), DATA.grain(:, 2), x).*L;

DATA.Pbar  = DATA.propl(:, 1);                                              % bar
DATA.Tc  = @(P) interp1(DATA.Pbar, DATA.propl(:, 2), P, 'spline');                    
DATA.g   = @(P) interp1(DATA.Pbar, DATA.propl(:, 3), P, 'spline');
DATA.gs  = @(P) interp1(DATA.Pbar, DATA.propl(:, 4), P, 'spline');
DATA.R   = @(P) interp1(DATA.Pbar, DATA.propl(:, 5), P, 'spline');
DATA.cs  = @(P) interp1(DATA.Pbar, DATA.propl(:, 6), P, 'spline');
DATA.rho = @(P) interp1(DATA.Pbar, DATA.propl(:, 7), P, 'spline');

