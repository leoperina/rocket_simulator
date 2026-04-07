clear
close all
clc

eps_ = load("Ae_At.txt");


P_MPa = 0.1:1:20;

% P, T, g, gs, R, ue
output_matrix = zeros(length(P_MPa), 7);

T_start = 288;

% per lavorare su questo codice, basta modificare i dati CEA, il resto
% rimane invariato (anche la P_max in P_MPa volendo, ma 20 MPa sono un po'
% altini eh)

for I = 1:length(P_MPa)
    P = P_MPa(I);
    P_bar = P * 10;
    x = CEA('reac', ...
        'oxid','NH4CLO4(I)','wt%',68,'t(k)',T_start, ...
        'fuel','AL(cr)','wt%',18,'t(k)',T_start, ...
        'fuel','HTPB','C',7.075,'H',10.65,'O',0.223,'N',0.063,'wt%',14,'h,kj/mol',-58,'t(k)',T_start, ...
        'prob','case','CASO PROVA','hp','p,bar',P_bar, ...
        'output','short','thermo','transport','si','end');
    
    x_rocket = CEA('problem','rocket','frozen','nfz',1,... 
      'p,bar',P_bar,'sup,ae/at',eps_,...
    'react','fuel','AL(cr)',...
        'wt',18,'t,k',T_start,'oxid','NH4CLO4(I)', 'wt',68,'t,k',T_start,...
        'fuel','HTPB', 'wt',14,'t,k',T_start,'h,kj/mol',-58,...
        'C', 7.075, 'H', 10.65, 'O', 0.223, 'N', 0.063, 'output','end');
    out = zeros(1, 5);
    out(1) = x.output.temperature;
    out(2) = x.output.gamma;           % gamma non congelata
    out(3) = -x.output.dlvpt * out(2); % gamma congelata
    out(4) = 8314 / x.output.mw;
    out(5) = x_rocket.output.froz.cstar(1);
    out(6) = x.output.density;
    output_matrix(I, :) = [P, out];
end

filename = 'totrash.txt';


%% plot di debug
clc
close all
clearvars -except output_matrix filename

P  = output_matrix(:, 1);
T  = output_matrix(:, 2);
g  = output_matrix(:, 3);
gs = output_matrix(:, 4);
R  = output_matrix(:, 5);
cs = output_matrix(:, 6);
rho= output_matrix(:, 7);

rho_th = P*1e6./(R.*T);


figure('Color',[1,1,1])
subplot(3, 2, 1)
plot(P, T)
title('Chamber T')
subplot(3, 2, 2)
plot(P, g)
title('\gamma')
subplot(3, 2, 3)
plot(P, gs)
title('\gamma_s')
subplot(3, 2, 4)
plot(P, R)
title('R')
subplot(3, 2, 5)
plot(P, cs)
title('c^*')

subplot(3, 2, 6)
plot(P, rho)
hold on
plot(P, rho_th)
title('\rho')

figure('Color',[1, 1, 1])
loglog(P, abs(rho - rho_th))
grid minor
%% salvataggio dati
clearvars -except output_matrix filename
writematrix(output_matrix, filename)

