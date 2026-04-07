clear
close all
clc

%%%%%%%%%%%%%%%%%%%%%%%%%%%% LAUNCHER DYNAMICS %%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Se non specificato le unità sono tutte in SI
% NB!!! Anche se mdot è presente, in realtà la massa è mantenuta costante.
% 
%   MSL     [struct]        contiene le dimensioni caratteristiche del
%                           lanciatore.
%   AMB     [struct]        struct containing data about air at launch

%___ variables ____________________________________________________________

AMB.g0     = 9.8065;                                                          % m/s2
AMB.gamma  = 1.4;                                                            
AMB.T0     = 288.15;                                                          % K
AMB.R      = 8314 / 29;                                                       % J/KgK
AMB.M      = sqrt(AMB.gamma*AMB.R*AMB.T0);
AMB.lambda = -0.0065;
AMB.rho    = @(h) ...
    1.225*(1 + AMB.lambda*h/AMB.T0).^(-AMB.g0/(AMB.R*AMB.lambda));

figure('Color',[1, 1, 1])
plot(1:5000, AMB.rho(1:5000))


MSL.l     = 1;                                                               % rocket length
MSL.diam  = 0.12;                                                            % body diameter
MSL.lc    = 0.45;                                                            % control arm
MSL.la    = 0.20;                                                            % aerodinamic arm
MSL.ms    = 1;                                                               % empty weight
MSL.mp    = 0.5;                                                             % propellent weight (at startup)
MSL.m     = MSL.mp + MSL.ms;                                                 % starting mass
MSL.J     = MSL.m*MSL.l^2/12;                                                % moment of inertia
MSL.mdot  = -MSL.mp / 5;                                                     % mass flow out of the nozzle (negative since it's exiting the rocket)
MSL.tb    = MSL.mp / abs(MSL.mdot);                                          % burning time [s] (after this time, thrust goes to zero)
MSL.Isp   = 300;                                                             % engine Isp, in [s]
MSL.T     = MSL.Isp*abs(MSL.mdot)*AMB.g0;                                    % engine thrust, assumed constant [N]
MSL.clt   = 3.7;                                                             % cl_derivative [cl/rad]
MSL.clts  = -2.4;                                                            % cl_der after stalling [cl/rad]
MSL.thlim = deg2rad([-45, -20, 20, 45]);                                     % theta limits
MSL.thc   = 0.1;                                                             % viscous damping
MSL.cl    = @(th) ...
    (th > MSL.thlim(2) & th <= MSL.thlim(3)) .* MSL.clt.*th + ...
    (th > MSL.thlim(3) & th <= MSL.thlim(4)) .* ...
    (MSL.clts.*(th - MSL.thlim(3)) + MSL.clt.*MSL.thlim(3)) + ...
    (th >= MSL.thlim(1) & th < MSL.thlim(2)) .* ...
    (MSL.clts.*(th - MSL.thlim(2)) + MSL.clt.*MSL.thlim(2));
MSL.cd0   = 0.01;
MSL.k     = 0.01;
MSL.cd    = @(th) MSL.cd0 + MSL.k*MSL.cl(th).^2;




%% simulazione
clc
SIM.y0 = [0; 0; 0; 0.1; 0; 0; MSL.mp];
SIM.Fc = @(t) 0*t + 10.*(t > 1 & t < 4);

SIM.opts = odeset(RelTol=1e-4,AbsTol=1e-8);
[t, y] = ode89(@(t, y) validation_model_variablemassNrho...
    (t, y, MSL, AMB.g0, AMB.rho, SIM.Fc) , ...
    [0, 50], SIM.y0, SIM.opts);

SIM.x    = y(:, 1);
SIM.vx   = y(:, 2);
SIM.y    = y(:, 3);
SIM.vy   = y(:, 4);
SIM.th   = y(:, 5);
SIM.t    = t;
SIM.beta = atan2(y(:, 4), y(:, 2)); 
SIM.U    = sqrt(SIM.vx.^2 + SIM.vy.^2);
SIM.mp   = y(:, 7);

SIM.ax   = [0; diff(SIM.vx)];
SIM.ay   = [0; diff(SIM.vy)];


clearvars t y;


if max(abs(SIM.th)) > min(abs([MSL.thlim(1), MSL.thlim(4)]))
    error('Missile theta went out of bounds. Results are garbage')
elseif max(abs(SIM.th)) > min(abs([MSL.thlim(2), MSL.thlim(3)]))
    warning('Missile stalled during the simulation!')
end


%% plots e validazioni
close all

%============================================================cd e cl models

% figure('Color',[1,1,1])
% th = linspace(MSL.thlim(1), MSL.thlim(end), 1000);
% subplot(1, 2, 1)
% yyaxis right
% plot(rad2deg(th), MSL.cl(th))
% xline(0)
% yline(0)
% yyaxis left
% plot(rad2deg(th), MSL.cd(th))
% grid minor
% subplot(1, 2, 2)
% plot(MSL.cd(th), MSL.cl(th))
% title('Polar')
% grid on


%=============================================================velocity plot

figure('Color',[1, 1, 1])
yyaxis left
plot(SIM.t, SIM.U / AMB.M, ...
    'LineWidth', 1, 'Color','k', 'DisplayName', 'Mach speed')
ylabel('Mach number')
yyaxis right
plot(SIM.t, SIM.U, ...
    'LineWidth', 1, 'Color','k', 'Visible','on', 'HandleVisibility','off')
hold on
xline(MSL.tb, 'DisplayName', 'MECO')
ylabel('Speed [m/s]')
grid minor
title('Rocket speed (air condition at '+string(AMB.T0 - 273.15)+'C°)')
legend
ax = gca;
ax.YAxis(1).Color = 'k';
ax.YAxis(2).Color = 'k';


%==========================================================position in time

figure('Color',[1, 1, 1])
scatter(SIM.x, SIM.y, [], SIM.t)
colorbar
axis equal
grid on

figure('Color',[1, 1, 1])
plot(SIM.t, rad2deg(SIM.th), 'DisplayName', '\theta', 'LineWidth',1)
hold on
plot(SIM.t, rad2deg(SIM.beta), 'DisplayName', '\beta', 'LineWidth',1)
xline(MSL.tb, 'DisplayName', 'MECO')
ylabel('Angle - [deg]')
xlabel('Time - [s]')
yyaxis right
plot(SIM.t, SIM.Fc(SIM.t), 'DisplayName', 'Control Force', 'LineWidth',1)
ylabel('Force - [N]')
legend
grid minor


%=================================================propeller mass and thrust

% figure('Color',[1, 1, 1])
% plot(SIM.t, MSL.Isp * [0;-diff(SIM.mp)]*AMB.g0, 'LineWidth',1)
% xlabel('Time [s]')
% ylabel('Thrust [N]')
% grid minor

figure('Color',[1, 1, 1])
plot(SIM.t, SIM.mp, 'LineWidth',1, 'Color','k')
xlabel('Time [s]')
ylabel('Mass of propellant in the rocket [Kg]')


%======================================================rocket accelerations

figure('Color',[1, 1, 1])
plot(SIM.t, SIM.ax, 'LineWidth', 1, 'DisplayName','Horizontal accelerations')
hold on
plot(SIM.t, SIM.ay, 'LineWidth', 1, 'DisplayName','Vertical accelerations')
grid on
legend()


clearvars ax
