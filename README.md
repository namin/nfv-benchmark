# nfv-benchmark

Adopted to continue project on NFV

## Instructions:
- Setup DPDK in sibling directory ([zip](https://github.com/DPDK/dpdk/archive/v18.02.zip), ([docs](https://dpdk.readthedocs.io/en/v2.2.0/linux_gsg/intro.html)))
  * `make config T=x86_64-native-linuxapp-gcc`
  * `make`
- `make`
- As root:
  * `sudo su`
  * `setup_hugepages.sh`
  * `exit`
- `benchmark.sh`

## Next steps:
- [x] Build and run anything
  - [x] Fix gcc warnings
- [ ] Run benchmarks
- [ ] Write custom benchmarks
- [ ] Initial language design
