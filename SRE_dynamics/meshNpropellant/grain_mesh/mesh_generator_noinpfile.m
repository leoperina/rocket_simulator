clear
close all
clc

filename = "star_data";
% T = readmatrix(filename + ".txt");

D_mx = 0.4;
N    = 10;
D    = 0.25;
% h_fr = 0.3188;
h_fr = 0.45;

step_vec = 0.005*ones(1, 1000);
% step_vec = 0.001 * 10.^(0:0.07:2);

perim = [];
area_ = [];
[x, y] = star_port_builder(D, N, h_fr);
n = length(x);
% plot(x, y)
% axis equal
n_per_lato = 10;
meshx = [];
meshy = [];
% popolo la mesh
for I = 1:n
    meshx = [meshx, linspace(x(I), x(mod(I,n) + 1), n_per_lato)];
    meshy = [meshy, linspace(y(I), y(mod(I,n) + 1), n_per_lato)];
end

meshxy = unique(round([meshx(:), meshy(:)], 6), 'rows', 'stable');
meshx = meshxy(:,1);
meshy = meshxy(:,2);

fig = polyshape(meshx, meshy);
% inizio a spostare
nmax = 50;
fig_array = cell(nmax, 1);
fig_array{1} = fig;
roundEvery = 4; % ogni quante iterazioni fare 'round'
k = 1;
skip = 0;




while ~skip
    if mod(k + roundEvery - 1, roundEvery) == 0
        jt = 'round'; % ogni roundEvery passi usa bordi arrotondati
    else
         jt = 'miter'; % altrimenti veloce
    end 
    fig = polybuffer(fig_array{k}, step_vec(k), 'JointType', jt);
    area_ = [area_; area(fix_figure(fig, D_mx))];
    perim = [perim; calc_perimeter(fig, D_mx)];
    k = k+1;
    fig_array{k} = fig;
    if perim(k-1) == 0
        break;
    end
    disp(k)
end
% sapendo che l'ultimo è a perimetro nullo, l'area sarà proprio
% la sezione del booster, quindi la sovrascrivo
area_(end) = (D_mx/2)^2*pi;


%% === debug per vedere le shapes
figure('Color',[1, 1, 1])
hold on
for I = k:-1:1
    plot(fig_array{I})
end
th = linspace(0, 2*pi, 1000);
plot(D_mx/2 * ones(1000, 1) .* cos(th), D_mx/2 * ones(1000, 1) .* sin(th), 'k.')
axis equal


figure('Color',[1, 1, 1])
yyaxis left
plot(cumsum(step_vec(1:k-1)), perim, 'rO-')
ylabel('Perimetro in sezione, perim [m]')
yyaxis right
plot(cumsum(step_vec(1:k-1)), area_, 'bO-')
ylabel('Area in sezione, A [m^2]')
xlabel('passo di regressione, step [-]')
legend('perimetro', 'area')


writematrix([perim, area_, cumsum(step_vec(1:k-1))', repmat(D_mx, size(perim))], filename+"_graphs.txt")

%%
function fig = fix_figure(fig, D_max)
% Filtra i vertici di fig per mantenere:
% - Tutti i punti dentro il cerchio di raggio r-tol
% - Alcuni punti sul bordo di raggio r +/- tol
% - Rimuove punti troppo fuori raggio
%
% fig.Vertices è Nx2 array di coordinate [x, y]
%
r = D_max / 2;
tol = 1e-6;  % tolleranza bordo

vertices = fig.Vertices;
rhos = sqrt(sum(vertices.^2, 2));

% Indici punti interni (meno tolleranza)
idx_inside = rhos < (r - tol);

% Indici punti sul bordo (entro tolleranza)
idx_border = abs(rhos - r) <= tol;

% Indici punti troppo fuori raggio
idx_outside = rhos > (r + tol);

% Mantieni tutti punti interni
vertices_inside = vertices(idx_inside, :);

% Prendi solo alcuni punti sul bordo per ridurre densità
border_points = vertices(idx_border, :);
if ~isempty(border_points)
    % Calcola angoli per ordinare i punti sul bordo
    angles = atan2(border_points(:,2), border_points(:,1));
    [~, sortIdx] = sort(angles);
    border_points_sorted = border_points(sortIdx, :);
    
    % Prendi uno ogni k punti
    k = max(1, floor(size(border_points_sorted,1) / 20)); % massimo 20 punti sul bordo
    border_points_filtered = border_points_sorted(1:k:end, :);
else
    border_points_filtered = [];
end

% Ricostruisci vertici filtrati
new_vertices = [vertices_inside; border_points_filtered];

% (opzionale) riordina tutti i vertici in senso antiorario
new_vertices = sort_vertices_ccw(new_vertices);

% Aggiorna la struttura fig
fig.Vertices = new_vertices;

end

function sorted_vertices = sort_vertices_ccw(vertices)
% Ordina i punti in senso antiorario rispetto al centro (0,0)

angles = atan2(vertices(:,2), vertices(:,1));
[~, order] = sort(angles);
sorted_vertices = vertices(order, :);

end

function perim = calc_perimeter(fig, D_max)
% calcola il perimetro, ma soltanto se il 
% perimetro è sotto D_max. Serve a trovare il perimetro
% che effettivamente sta bruciando
r = D_max/2;
perim = 0;
n = length(fig.Vertices);
for I = 1:n
    next = mod(I, n) + 1;
    x1 = fig.Vertices(I, 1);
    y1 = fig.Vertices(I, 2);
    rho1 = norm([x1, y1]);
    x2 = fig.Vertices(next, 1);
    y2 = fig.Vertices(next, 2);
    rho2 = norm([x2, y2]);
    epsilon = 1e-4;
    if rho1 < (r - epsilon) && rho2 < (r - epsilon)
        perim = perim + norm([x2 - x1, y2 - y1]);
    end
end
end