function [x, y] = star_port_builder(D, N, h_frac)
    % Dch = D*sqrt(1+(h_frac*N/pi)^2)^-1;
    h = h_frac/2*D;
    thetai = 2*pi/N;
    theta = zeros(N*2, 1);
    peak = zeros(length(theta), 1);
    for I = 1:N
        theta(2*I-1) = thetai*(I-1);
        theta(2*I)   = thetai*(I-1) + thetai/2;
        peak(2*I-1)  = D/2;
        peak(2*I)    = D/2 - h;
    end
    theta = [theta; 0];
    peak  = [peak; D/2];

    x = peak.*cos(theta);
    y = peak.*sin(theta);
end

