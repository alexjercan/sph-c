# Smoothed Particle Hydrodynamics in C

My attempt at implementing
[SPH](https://en.wikipedia.org/wiki/Smoothed-particle_hydrodynamics) in C by
following this
[paper](https://web.archive.org/web/20160910114523id_/http://www.astro.lu.se:80/~david/teaching/SPH/notes/annurev.aa.30.090192.pdf).

For now it is pretty much a work in progress.

## Quickstart

```console
mkdir -p build
cd build
cmake ..
cd ..
cmake --build ./build
./build/water_simulation
```
