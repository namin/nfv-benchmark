# nfv-benchmark

Adopted to continue project on NFV

## Instructions:
- Setup DPDK in sibling directory ([zip](https://github.com/DPDK/dpdk/archive/v18.02.zip), ([docs](https://dpdk.readthedocs.io/en/v2.2.0/linux_gsg/intro.html)))
  * `make config T=x86_64-native-linuxapp-gcc`
  * `make`
- As root:
  * `sudo ./setup_hugepages.sh`
  * `cat /proc/meminfo | grep Huge` should show some Free pages, e.g.:
```
AnonHugePages:         0 kB
HugePages_Total:    1024
HugePages_Free:     1024
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
```
  * If not:
  * `sudo umount -a -t hugetlbfs`
  * `sudo mount -t hugetlbfs nodev /mnt/huge`
  * can also do `sudo rm -rf /mnt/huge/*` to free once mounted
- `./benchmark.sh`

## Next steps:
- [x] Build and run anything
  - [x] Fix gcc warnings
- [ ] Run benchmarks
- [ ] Write custom benchmarks
- [ ] Initial language design
