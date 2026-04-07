clear

% cose da togliere nella v finale
close all
clc
% ---

% leggo P_bar cercata
P_bar = readmatrix('request.txt'); % di fatto è uno scalare (DEVE ESSERE IN BAR)


out(1) = x.output.temperature;
out(2) = x.output.gamma;           % gamma non congelata
out(3) = -x.output.dlvpt * out(2); % gamma congelata
out(4) = 8314 / x.output.mw;
out(5) = x_rocket.output.froz.sonvel(end);
out(6) = x_rocket.output.froz.cstar(end);
out(7) = x_rocket.output.froz.cf(end);

writematrix(out', "answer.txt");

