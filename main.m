clear
close all
clc


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% ROCKET DYNAMICS

AMB.g0     = 9.8065;                                                          % m/s2
AMB.gamma  = 1.4;                                                            
AMB.T0     = 288.15;                                                          % K
AMB.R      = 8314 / 29;                                                       % J/KgK
AMB.M      = sqrt(AMB.gamma*AMB.R*AMB.T0);
AMB.lambda = -0.0065;
AMBfnc.rho    = @(h) ...
    1.225*(1 + AMB.lambda*h/AMB.T0).^(-AMB.g0/(AMB.R*AMB.lambda));
AMB.dT     = -0.0065;
AMB.P0     = 101325;


% this will be rendered useless in Simulink
MSL.Isp   = 300;                                                             % engine Isp, in [s]
MSL.mp    = 0.5;                                                             % propellent weight (at startup)
MSL.mdot  = -MSL.mp / 5;                                                     % mass flow out of the nozzle (negative since it's exiting the rocket)
MSL.tb    = MSL.mp / abs(MSL.mdot);                                          % burning time [s] (after this time, thrust goes to zero)
MSL.T     = MSL.Isp*abs(MSL.mdot)*AMB.g0;                                    % engine thrust, assumed constant [N]


MSL.ms    = 500;                                                             % empty weight [Kg]
MSL.m     = MSL.mp + MSL.ms;                                                 % starting mass
MSL.l     = 1;                                                               % rocket length
MSL.diam  = 0.12;                                                            % body diameter
MSL.lc    = 0.45;                                                            % control arm
MSL.la    = 0.20;                                                            % aerodinamic arm
MSL.J     = MSL.m*MSL.l^2/12;                                                % moment of inertia
MSL.clt   = 3.7;                                                             % cl_derivative [cl/rad]
MSL.clts  = -2.4;                                                            % cl_der after stalling [cl/rad]
MSL.thlim = deg2rad([-45, -20, 20, 45]);                                     % angle of attack limits. Over those values the rocket is in a stall and the simulation stops having physical sense
MSL.thc   = 0.1;                                                             % viscous damping
MSLfnc.cl    = @(th) ...
    (th > MSL.thlim(2) & th <= MSL.thlim(3)) .* MSL.clt.*th + ...
    (th > MSL.thlim(3) & th <= MSL.thlim(4)) .* ...
    (MSL.clts.*(th - MSL.thlim(3)) + MSL.clt.*MSL.thlim(3)) + ...
    (th >= MSL.thlim(1) & th < MSL.thlim(2)) .* ...
    (MSL.clts.*(th - MSL.thlim(2)) + MSL.clt.*MSL.thlim(2));
MSL.cd0   = 0.01;
MSL.k     = 0.01;
MSLfnc.cd    = @(th) MSL.cd0 + MSL.k*MSLfnc.cl(th).^2;
MSL.y0_sim = [0; 0; 0; 0.1; 0; 0];

%__________________________________________________ Simulink bus generation
Simulink.Bus.createObject(MSL);
MSLbus = slBus1;
clear slBus1
Simulink.Bus.createObject(AMB);
AMBbus = slBus1;
clear slBus1

clearvars AMBfnc MSLfnc;

%__________________________________________________________________________


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% ENGINE MODEL

FILENAMES.propl_data = "SRE_dynamics/meshNpropellant/propellant/totrash.txt";
FILENAMES.grain_data = "SRE_dynamics/meshNpropellant/grain_mesh/star_data_graphs.txt";

ENG.propl = load(FILENAMES.propl_data);
ENG.grain = load(FILENAMES.grain_data);                                    % already in SI unities


ENGfnc.Ab   = @(x)    interp1(ENG.grain(:, 3), ENG.grain(:, 1), x);
ENGfnc.Vc   = @(x, L) interp1(ENG.grain(:, 3), ENG.grain(:, 2), x).*L;
ENG.xfin = ENG.grain(end, 3);
ENG.DCC  = ENG.grain(1, 4);

ENG.Pbar  = ENG.propl(:, 1);                                              % bar
ENGfnc.Tc  = @(P) interp1(ENG.Pbar, ENG.propl(:, 2), P);                    
ENGfnc.g   = @(P) interp1(ENG.Pbar, ENG.propl(:, 3), P);
ENGfnc.gs  = @(P) interp1(ENG.Pbar, ENG.propl(:, 4), P);
ENGfnc.R   = @(P) interp1(ENG.Pbar, ENG.propl(:, 5), P);
ENGfnc.cs  = @(P) interp1(ENG.Pbar, ENG.propl(:, 6), P);
ENGfnc.rho = @(P) interp1(ENG.Pbar, ENG.propl(:, 7), P);

ENG.L      = 1;                                                              % length of the CC (m)
ENG.Vc_dry = ENG.L * (ENG.DCC^2/4 * pi);
ENG.rhop   = 1750;                                                           % Kg/m3
ENG.At     = 0.05;                                                            % m2
ENG.mp     = ENG.rhop * (ENG.Vc_dry - ENGfnc.Vc(ENG.grain(1, 3), ENG.L));

ENG.a    = 7e-5;                                                           % m/s/Pa
ENG.n    = 0.4;
ENG.y0 = [101325; ENG.grain(1, 3); ENG.mp];


%__________________________________________________ Simulink bus generation
Simulink.Bus.createObject(ENG);
ENGbus = slBus1;
clear slBus1


clearvars ENGfnc;

%__________________________________________________________________________


disp('data loading complete, ready to pass in Simulink')
disp('Filenames')
disp(FILENAMES)
clear FILENAMES;