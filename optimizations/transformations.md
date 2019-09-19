## Batching

```
map = table(8 bytes => 8 bytes)
module measurement(pkt) {
  map[pkt[ip.src, ip.dst, ip.]]++
}
```

Transforms to:

```
map = table(8 bytes => 8 bytes)
module measurement(pkts) {
  for pkt in pkts {
    map[pkt[ip.src, ip.dst, ip.]]++
  }
}
```
## Prefetching instructions

```
map = table(8 bytes => 8 bytes)
module measurement(pkt) {
  map[pkt[ip.src, ip.dst, ip.]]++
}
```

Transforms to:

```
map = table(8 bytes => 8 bytes)
module measurement(pkt) {
  prefetch map[pkt[ip.src, ip.dst, ip.]] in cell
  cell++
}
```

## Replacing a branch with a no-branch counterpart 
```
router = lpm_table(8 bytes => 4 bytes) 
map    = table(8 bytes => 8 bytes)

module route_and_count_or_drop(pkt) {
  if (router[pkt[ip.src, ip.dst]] != DROP) {
    map[pkt[ip.src, ip.dst]]++
  }
}
```

Transforms to:

```
router = lpm_table(8 bytes => 4 bytes) 
map    = table(8 bytes => 8 bytes)

module route_and_count_or_drop(pkt) {
  map[pkt[ip.src, ip.dst]] += (router[pkt[ip.src, ip.dst]] != DROP)
}
```

## SIMD instruction templates for packet processing 


