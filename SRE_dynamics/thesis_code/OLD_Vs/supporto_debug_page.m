clear
% close all
clc

n_file = 1;
FOLDER = "MESH_TEST";

figure('Color',[1, 1, 1])
hold on
maxX = -1;
maxY = -1;
plot_count = 0;
for I = 1:n_file
    filename = FOLDER+'/graph_data'+string(I)+'.txt';
    [p, s, N, h_fr, D, step] = loadGraph(filename, 0.02);
    if max(s) > maxX
        maxX = max(s);
    end
    if max(p) > maxY
        maxY = max(p);
    end
    if I
        plot_count = plot_count +1;
        plot(s, p, '*-')
        name = 'N = '+string(N) + ", h_% = "+string(h_fr) + ", D = "+string(D) + ", I = "+string(I);
        leg_name(plot_count) = name;
        xlabel('Step [s]')
        ylabel('Burning perimeter - [m]')
    end
end
xlim([0, maxX])
ylim([0, maxY])
legend(leg_name)



%% 
clear
% close all
clc

FOLDER = 'MESH_TEST';
T = readtable(FOLDER+"\mesh_data1.txt");

x = T.Var1;
y = T.Var2;
% print del case esterno
D = 3;
r = D/2;
theta = linspace(0, 2*pi, 1000);

figure('Color',[1, 1, 1])
plot(y, x, '.')
% hold on
% plot(r*ones(1000, 1).*cos(theta), r*ones(1000, 1).*sin(theta), 'r.')
axis equal



%%
function [p, s, N, h_fr, D, step] = loadGraph(filename, r_b) 
    T = readmatrix(filename);
    N    = T(1);
    h_fr = T(2);
    step = T(3);
    D    = T(4);
    p  = T(5:end);
    len  = length(p);
    dt = step/r_b;
    s = 0:dt:(len-1)*dt;
end
