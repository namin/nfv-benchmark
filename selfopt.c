#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rte_ethdev.h"

#include "benchmark.h"
#include "dataplane.h"
#include "jit.h"
#include "log.h"
#include "packets.h"
#include "pipeline.h"
#include "opt.h"

#include "rte_cycles.h"
#include "rte_prefetch.h"

#define F 5
#define I 3
#define FA {0, 0, 0, 0, 0}
#define IA {FA,FA,FA}
int sk[] = FA;
int st[] = FA;
int sk_per[I][F] = IA;
int st_per[I][F] = IA;

int sample_input()
{
  return rand() % I;
}

int missing(int* sk)
{
  for (int i=0; i<F; i++) {
    if (sk[i]==0) return i;
  }
  return -1;
}

double over(double* sr, int* sk, int* st)
{
  double total = 0.0;
  for (int i=0; i<F; i++) {
    if (st[i] != 0) {
      double r = sk[i]/(1.0*st[i]);
      sr[i] = r;
      total += r;
    }
  }
  return total;
}

double drand(double high)
{
    return (rand() * high)/RAND_MAX;
}

int sample_fun(int* sk, int* st)
{
  int k = missing(sk);
  if (k != -1) return k;
  double sr[] = FA;
  double total = over(sr, sk, st);
  double r = drand(total);
  double c = 0.0;
  for (int i=0; i<F; i++) {
    if (r < c+sr[i]) return i;
    c += sr[i];
  }
  return -1; // shouldn't happen
}

int print_summary_per()
{
  printf("\nselfopt: SUMMARY\n");
  for (int arg=0; arg<I; arg++) {
    printf("selfopt: arg=%d ", arg);
    double sr[] = FA;
    double total = over(sr, sk_per[arg], st_per[arg]);
    if (total > 0.0) {
      for (int i=0; i<F; i++) {
        sr[i] /= total;
      }
    }
    for (int i=0; i<F; i++) {
      printf("%.3f ", sr[i]);
    }
    printf("\n");
  }
  return 0;
}

int print_summary_and_cleanup()
{
  print_summary_per();
  for (int fun=0; fun<F; fun++) {
    sk[fun] = 0;
    st[fun] = 0;
  }
  for (int arg=0; arg<I; arg++) {
    for (int fun=0; fun<F; fun++) {
      sk_per[arg][fun] = 0;
    }
  }
  return 0;
}
  
void packets_pool_uniform(struct packet_pool_t *pool)
{
  packet_t *pkt_first = NULL;
  for (packet_t *pkt = packets_pool_first(pool); 
       pkt < pool->end; pkt = (packet_t*)(pkt->data + pool->size)) {
    if (pkt_first == NULL) {
      pkt_first = pkt;
    } else {
      memcpy(pkt->data, pkt_first->data, pkt->size);
    }
  }
}

int pipeline_adaptive(int m, int n, char const *name)
{
  printf("selfopt: for %s\n", name);
  uint32_t packet_count = 1<<10;
  struct packet_pool_t *pool_random = packets_pool_create(packet_count, PACKET_SIZE);
  struct packet_pool_t *pool_zipf = packets_pool_create(packet_count, PACKET_SIZE);
  struct packet_pool_t *pool_uniform = packets_pool_create(packet_count, PACKET_SIZE);
  packets_pool_zipfian(pool_zipf, 0, packet_count - 1, 26, 8, 0.5);
  packets_pool_uniform(pool_uniform);
  struct packet_pool_t* pools[I] = {pool_random, pool_zipf, pool_uniform};

  struct jit_t jit = {0};
  jit_test_load(&jit, name);

  for (int i=0; i<n; i++) {
    int arg = sample_input();
    int* sk_per_arg = sk_per[arg];
    int* st_per_arg = st_per[arg];
    int k;
    if (i < m ||
        missing(sk_per_arg)!=-1 ||
        missing(st_per_arg)!=-1) {
      k = sample_fun(sk, st);
    } else {
      k = sample_fun(sk_per_arg, st_per_arg);
    }

    struct packet_pool_t* pool = pools[arg];
    uint32_t repeat = 40;
    asm volatile ("mfence" ::: "memory");
    uint64_t cycles = rte_get_tsc_cycles();
    (*jit.entry.test)(pool, repeat, k);
    asm volatile ("mfence" ::: "memory");
    int r = (int)(100*(float)(rte_get_tsc_cycles() - cycles)/(float)(packet_count * repeat));
   
    sk[k] += 1;
    st[k] += r;
    sk_per_arg[k] += 1;
    st_per_arg[k] += r;
    /*
    if (i % 100 == 1) {
      printf("%d ", i);
      print_summary_per();
    }
    */
  }

  jit_test_unload(&jit);
  packets_pool_delete(&pool_random);
  packets_pool_delete(&pool_zipf);
  packets_pool_delete(&pool_uniform);
      
  return 0;
}

int datapath_init(int argc, char **argv, struct dataplane_port_t **port) {
    *port = 0;
    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Failed to initialize the EAL.");

    const char port_name[] = PORT_NAME;
    log_info_fmt("Num available dpdk ports (i.e., number of usable ethernet devices): %d", rte_eth_dev_count());

    struct dataplane_port_t *pport = 0;
    ret = port_configure(port_name, &pport);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Failed to configure port %s", port_name);
    log_info_fmt("Port [%s] configured successfully.", port_name);
    *port = pport;
    return ret;
}

void datapath_teardown(struct dataplane_port_t *port) {
    port_release(port);
    rte_eal_cleanup();
}

int main(int argc, char **argv)
{
  struct dataplane_port_t *port = 0;
  int ret = datapath_init(argc, argv, &port);
  argc -= ret;
  argv += ret;

  if (!port) 
    return 0;

  int m = 1000;
  int n = 3000;
  pipeline_adaptive(m, n, "checksum-checksum");
  print_summary_and_cleanup();
  pipeline_adaptive(m, n, "checksum-drop");
  print_summary_and_cleanup();
  pipeline_adaptive(m, n, "checksum-rfile");
  print_summary_and_cleanup();
  pipeline_adaptive(m, n, "checksum-routing");
  print_summary_and_cleanup();
  pipeline_adaptive(m, n, "mea_checksum-rfile");
  print_summary_and_cleanup();
  pipeline_adaptive(m, n, "mea_rfile-checksum");
  print_summary_and_cleanup();
  pipeline_adaptive(m, n, "measurement-drop");
  print_summary_and_cleanup();
  pipeline_adaptive(m, n, "rfile_checksum-mea");
  print_summary_and_cleanup();
  pipeline_adaptive(m, n, "rfile-drop");
  print_summary_and_cleanup();

  datapath_teardown(port);
  return 0;
}
