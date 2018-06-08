#ifndef _FLL_H_
#define _FLL_H_

#include "memory.h"

/*
 * Frequency locked loop between a slave and a master.
 * 
 * Use binary search to ensure there is no packet loss on the slave side.  The
 * algorithm works as follows: 
 *  1) the master starts to send packets at max rate,
 *  2) the slave checks to see if there is packet loss on its side.
 *      If yes, it sends a request to increase the delay between packets (in us)
 *      If not, it sends a request to decrease the interpacket delay.
*/

#define FLL_UPPER 150
#define FLL_LOWER 0
#define FLL_MAGIC 0xbeed5335
#define FLL_THRESHOLD 100
#define FLL_PKT_COUNT_START 1<<16
#define FLL_PKT_COUNT_MAX   1<<24

struct __attribute__((packed)) fll_packet_t {
    uint32_t magic;
    uint32_t delay;
};

struct fll_t {
    uint32_t upper;
    uint32_t lower;
    uint32_t current;
    uint32_t mode;
    uint32_t count;
};

inline int fll_is_fll_pkt(char *data) {
    struct fll_packet_t *pkt = (struct fll_packet_t*)data;
    return pkt->magic == FLL_MAGIC;
}

inline int fll_master(struct fll_t *fll, int loss, char *buffer) {
    if (fll->lower == fll->upper || (loss != 0 && fll->count < FLL_PKT_COUNT_MAX && loss < FLL_THRESHOLD) || (loss == 0 && fll->count >= FLL_PKT_COUNT_MAX)) {
        if (fll->count >= FLL_PKT_COUNT_MAX)
            return 1;
        fll->count <<= 1;
    }

    if (loss > 0) {
        fll->current += 1;
    } else {
        if (fll->current == 0)
            return 1;
        fll->current -= 1;
    }

    struct fll_packet_t *pkt = (struct fll_packet_t *)buffer;
    pkt->magic = FLL_MAGIC;
    pkt->delay = fll->current;
    return 0;
}

inline int fll_follower(struct fll_t *fll, char *data) {
    struct fll_packet_t *pkt = (struct fll_packet_t*)data;
    fll->current = pkt->delay;
}

inline int fll_delay(struct fll_t *fll) {
    rte_delay_us(fll->current);
}

inline struct fll_t* fll_create() {
    struct fll_t *fll = mem_alloc(sizeof(struct fll_t));
    fll->upper = FLL_UPPER;
    fll->lower = FLL_LOWER;
    fll->current = fll->upper;
    fll->mode = 1;
    fll->count = FLL_PKT_COUNT_START;
    return fll;
}

inline void fll_release(struct fll_t *fll) {
    mem_release(fll);
}

#endif // _FLL_H_
