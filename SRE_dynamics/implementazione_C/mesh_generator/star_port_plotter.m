clear
close all
clc

D  = 1.9025;
N  = 20;
hf = 0.2124;
Dm = 3.2;

% th = linspace(0, 2*pi, 100);
% xb = Dm/2 * cos(th)';
% yb = Dm/2 * sin(th)';
% 
% [x, y] = star_port_builder(D, N, hf);
% x = [x; xb];
% y = [y; yb];

T = readmatrix("cat.txt");
x = T(:, 1);
y = T(:, 2);
th = linspace(0, 2*pi, 100);
xb = Dm/2 * cos(th)';
yb = Dm/2 * sin(th)';
x = [x; x(1); xb];
y = [y; y(1); yb];

fig = polyshape(x, y);
% bst = polyshape(xb, yb);

figure('Color',[1, 1, 1])
plot(fig, 'FaceColor',[0, 0.1, 0.7])
hold on
% plot(bst, 'FaceColor','r')
axis equal
axis off
xlim([-2.5, 2.5])
