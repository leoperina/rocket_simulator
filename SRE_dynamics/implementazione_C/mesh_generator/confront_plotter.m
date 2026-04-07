clear
close all
clc

% filename = 'N' + ret_formatted_num(N) + 'D1_902500h'+ret_formatted_num(hfr) + 'De3_200000_graphs.txt';


Nv  = [5, 10, 15, 20, 50];
hfv = [0.2:0.2:0.8];
% hfv = 0.4;

size_tot = length(Nv) * length(hfv);

legstr = [];

figure('Color',[1, 1, 1])
count = 1;

col   = colormap("turbo");
lC    = length(col);

% Prepariamo la mappa dei colori in base agli indici di Nv
Nv_vals = Nv; % tieni copia dei valori
nNv     = length(Nv_vals);

for iN = 1:nNv
    N = Nv_vals(iN);
    for hfr = hfv
        filename = 'N' + ret_formatted_num(N) + ...
                   'D1_902500h' + ret_formatted_num(hfr) + ...
                   'De3_200000_graphs.txt';
        T = readmatrix(filename);
        subplot(1, 1, 1)
        legstr = [legstr; "N = "+string(N) + ", h_% = "+string(hfr)];

        % scegliamo il colore in base all’indice di N
        midp = round( (iN-1)/(nNv-1) * (lC-1) ) + 1;

        hold on
        plot(T(:, 3), T(:, 1), '-', 'Color', col(midp, :), 'LineWidth',1.5)
    end
end

subplot(1, 1, 1)
grid on
ylabel('Perimetro - A_b / L [m]')
xlabel('step cumulativo, step [m]')

% aggiungiamo colorbar con etichette corrispondenti a N
c = colorbar;
c.Label.String = 'Valore di N';
c.Ticks = linspace(0,1,nNv);      % posizioni normalizzate
c.TickLabels = string(Nv_vals);   % etichette effettive
% legend(legstr)

% subplot(1, 2, 2)
% ylabel('Area - V_c / L [m]')
% xlabel('step cumulativo, step [m]')



%% funz

function snum = ret_formatted_num(num) 
    if num == floor(num)
        snum = string(num);
    else
        snum = strrep(string(num) + '00000', '.', '_');
    end
end
