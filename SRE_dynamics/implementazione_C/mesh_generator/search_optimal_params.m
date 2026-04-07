clear
close all
clc

T = readmatrix("optimal_Nh.txt");

Nv = T(:, 1);
hv = T(:, 2);

figure('Color',[1, 1, 1])
subplot(1, 2, 1)
plot(Nv, hv, 'O:', 'LineWidth',1.5)
hold on

% fit log-lineare
p = polyfit(log(Nv), log(hv), 1);   % p = [logB, logA]
B = p(1);
A = exp(p(2));

f = @(x) A.*x.^B;

f(12)
x = linspace(min(Nv), max(Nv), 1000);
plot(x, f(x), 'k--', 'LineWidth',1.2)
% plot(x, polyval(pC, x), 'k--', 'LineWidth',1.2)
legend('dati', 'fit esponenziale')
xlabel('Numero di punte, N [ - ]')
ylabel('Frazione, h_% [ - ]')
title('Relazione in piano cartesiano')
grid on



subplot(1, 2, 2)
loglog(Nv, hv,  'O:', 'LineWidth',1.5)
grid minor
xlabel('Numero di punte, N [ - ]')
ylabel('Frazione, h_% [ - ]')
title('Relazione in piano logaritmico')