This is a highly modified fork of [jt's voxel ray caster](https://www.shadertoy.com/view/7dK3D3).

This code simulates light interference between any number of [gaussian beams](https://en.wikipedia.org/wiki/Gaussian_beam), their electric field is defined by the equation: $${\mathbf {E} (r,z)}=E_{0}{\hat {\mathbf {x} }}{\frac {w_{0}}{w(z)}}\exp \left({\frac {-r^{2}}{w(z)^{2}}}\right)\exp \left(-i\left(kz+k{\frac {r^{2}}{2R(z)}}-\psi (z)\right)\right)$$
And the final intensity by the following equation: $$I(r,z)={|E(r,z)|^{2} \over 2\eta }$$

It is designed to be able to add multiple 'optical objects' such as polarizers or retarders, since it treats the phase of the wave as a complex value.

https://github.com/Pharadas/RayCastingLightSimulation/assets/60682906/4b609ae8-0ece-4158-bb85-1a92796b9b99
