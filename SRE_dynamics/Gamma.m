function [Gamma] = Gamma(gamma)
%GAMMA Vandenkerckhove's function Γ
Gamma = sqrt(gamma) .* (2./(gamma+1)).^((gamma+1)./2./(gamma-1));
end

