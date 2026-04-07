close all
clear
clc

T           = readmatrix("RUN_TO_SAVE\STIMA_ROCKETCANDY\simulation_data.txt");

perim       = T(:, 1);
t           = T(:, 2);
P           = T(:, 3);
A           = T(:, 4);
step        = T(:, 5);
T_          = T(:, 6) ./ 1e6;
dPdt        = T(:, 7);
dt          = t - [0; t(1:end-1)];
cs          = T(:, 8);
ct          = T(:, 9);


% per filtrare i dati in un intervallo di interesse
inf_thr = 0;
sup_thr = 1000;
P           = P(t > inf_thr & t < sup_thr );
perim       = perim(t > inf_thr & t < sup_thr );
T_          = T_(t > inf_thr & t < sup_thr );
dPdt        = dPdt(t > inf_thr & t < sup_thr );
A           = A(t > inf_thr & t < sup_thr );
step        = step(t > inf_thr & t < sup_thr );
dt          = dt(t > inf_thr & t < sup_thr );
cs          = cs(t > inf_thr & t < sup_thr );
ct          = ct(t > inf_thr & t < sup_thr );
t           = t(t > inf_thr & t < sup_thr );

g0          = 9.8065;
I_s         = ct.*cs./g0;
m_dot       = T_*1e6./(cs.*ct);


% stima della massa iniziale con stime geometriche (on point)
% ri = 3.2/2;
% Ai = ri^2*pi;
% V_eff = (Ai - A(1))*8.3;
% m_i   = V_eff*1690 / 1000;
% disp('Debug: '+string(m_i))


I_tot       = trapz(t, T_);
m_tot       = trapz(t, m_dot);
disp('Impulso totale: ' + string(I_tot) + " MNs")
disp('Massa totale  : ' + string(m_tot/1000) + " tons")

% calcolo le medie con media pesata
% mP = sum(P .*dt) / t(end);
% mT = sum(T_.*dt) / t(end);
% mmdot = sum(m_dot.*dt) / t(end);

t065     = t(t > 0 & t < 65);
mP065    = trapz(t, P)    / (t(end) - t(1));
mT065    = trapz(t, T_)   / (t(end) - t(1));
mmdotL = trapz(t, m_dot)/ (t(end) - t(1));


mP    = trapz(t, P)    / (t(end) - t(1));
mT    = trapz(t, T_)   / (t(end) - t(1));
mmdot = trapz(t, m_dot)/ (t(end) - t(1));

gs = 1;
figure('Color',[gs, gs, gs])
plot(t, P, 'b','LineWidth', 1)
hold on
% yline(mP)
disp('Pressione media: '+string(mP) + " MPa")
disp('Spinta media: ' + string(mT) + " MN")
ylabel('Pressione in camera, p_c [Mpa]')
yyaxis right
plot(t, T_,'r', 'LineWidth', 1)
hold on
% yline(mT, '--')
ylabel('Spinta, T [MN]', 'Interpreter','latex')
xlabel('Tempo, t [s]')
legend('Pressione', 'Thrust')
% title('Andamento di pressione e Thrust')

figure('Color',[1, 1, 1])
subplot(2, 1, 1)
plot(t, A, '--', 'LineWidth',1)
ylabel('Area di sezione [m^2]')
yyaxis right
plot(t, perim, '.-', 'LineWidth',1)
xlabel('Tempo - s')
ylabel('Perimetro - [m]')
title('Sviluppo del volume di camera di combustione per unità di lunghezza [ m^2 ]')
grid on

subplot(2, 1, 2)
plot(t, I_s)
ylabel('Impulso specifico, I_{sp} [s]')
xlabel('Tempo, t [s]')
title('Impulso specifico nel tempo')
grid on


c = (abs(dPdt) > 0.1).* 10;
figure('Color',[gs, gs, gs])
scatter(t, dPdt, [], c)
title('Andamento del dP/dt')
xlabel('Tempo - s')
ylabel('dPdt [MPa / s] ')
grid minor

figure('Color',[gs, gs, gs])
plot(t, m_dot, 'k', 'LineWidth',1)
% title('Portata in uscita dall''ugello')
xlabel('Tempo, t [s]')
ylabel('Portata massica, $\dot{m}$  [Kg / s] ', 'Interpreter','latex')
% grid minor

figure('Color',[gs, gs, gs])
yyaxis left
plot(t, cs)
ylabel('c^* [ m/s ]')
yyaxis right
plot(t, ct)
ylabel('cT [ / ]')
grid minor
legend('c^*', 'c_T')
title('Sviluppo dei coefficienti di spinta')

figure('Color',[gs, gs, gs])
subplot(2, 1, 1)
semilogy(t, step)
grid minor
title('Step size envelope')
ylabel('step size - [m]')
subplot(2, 1, 2)
semilogy(t, dt)
grid minor
title('Step size envelope')
ylabel('time step - [s]')

