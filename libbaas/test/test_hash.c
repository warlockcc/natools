#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "baas/hashtbl.h"
#include "baas/vector.h"

int intcmp(const int *a, const int *b) {
  return *a > *b ? 1 : (*a < *b ? -1 : 0);
}

int totalsum;
void acumulate(int *x) {
  totalsum += *x;
}


hashtbl_t * generate_test_ht(int allow_dups) {
  size_t n = 0, i, j;
  int sum = 0, *e = NULL;
  char *rk;

  hashtbl_t *h = hashtbl_init(free, (cmp_func_t)intcmp);
  hashtbl_allow_dups(h, allow_dups);
  vector_t *v = vector_init(free, NULL);
  hash_elem_t *f;

  size_t q = random() % 17000;
  for (i = 0; i < q; i++) {
    switch (random() % 6) {
      case 0:
      case 1:
      case 2: {
        e = (int*)zmalloc(sizeof(int)); *e = random() % 124789;
        rk = (char*)zmalloc(32);
        sprintf(rk, "%d", *e);
        size_t prev_size = hashtbl_size(h);
        hashtbl_insert(h, rk, e);
        if (allow_dups || hashtbl_size(h) == prev_size + 1)
          v = vector_append(v, rk); /* keep track of keys */
        else
          free(rk);
        n++; sum += *e;
      }
        break;
      case 3:
        if (!hashtbl_size(h)) continue;
        rk = (char*)vector_get(v, random() % vector_size(v));
        e = (int*)hashtbl_get(h, rk);
        assert(atoi(rk) == *e);
        break;
      case 4:
        if (!hashtbl_size(h)) continue;
        j = random() % vector_size(v);
        rk = (char*)vector_get(v, j);
        e = (int*)hashtbl_get(h, rk);
        n--; sum -= *e;
        hashtbl_delete(h, rk);
        v = vector_remove(v, j);
        break;
      case 5:
        if (!hashtbl_size(h)) continue;
        j = random() % vector_size(v);
        rk = (char*)vector_get(v, j);
        int k = atoi(rk);
        f = hashtbl_find(h, &k);
        n--; sum -= *(int*)hash_elem_data(f);
        hashtbl_remove(h, f);
        v = vector_remove(v, j);
        break;
    }
  }


  char **keys;
  size_t num_keys = hashtbl_keys(h, &keys);
  if (allow_dups)
    assert(num_keys == hashtbl_size(h));
  for (j = 0; j < num_keys; j++) {
    e = (int*)hashtbl_get(h, keys[j]);
    assert(atoi(keys[j]) == *e);
    free(keys[j]);
  }
  free(keys);

  if (allow_dups) {
    assert(n == hashtbl_size(h));
    totalsum = 0;
    hashtbl_foreach(h, (void(*)(void*))acumulate);
    assert(sum == totalsum);
  } else {
    while (hashtbl_size(h)) {
      rk = (char*)vector_get(v, 0);
      hashtbl_delete(h, rk);
      assert(hashtbl_find(h, rk) == NULL);
      v = vector_remove(v, 0);
    }
  }

  vector_destroy(v);
  return h;
}


#if 0
void check_hash_distribution(hashtbl_t *h) {
  int i;
  int *bsz = (int*)zmalloc(sizeof(int) * hashtbl_numbkt(h));

  int min = 0, max = 0, empty = 0;

  for (i = 0; (size_t)i < hashtbl_numbkt(h); i++) {
    if (h->buckets[i]) {
      bsz[i] = vector_size(h->buckets[i]);
      if (bsz[i] < min || i == 0) min = bsz[i];
      if (bsz[i] > max || i == 0) max = bsz[i];
    } else
      empty++;
  }
#ifdef _DEBUG_
  fprintf(stderr,
      " bucket stats: n %lu, empty %d, min %d, max %d, total %lu, avg %f\n",
      h->bktnum, empty, min, max, hashtbl_size(h),
      (float)hashtbl_size(h) / (float)(h->bktnum - empty));
#endif

  free(bsz);
}
#endif

void check_hash_function(hash_func_t hf, size_t maxkeylen) {
  size_t buckets[4096];
  const size_t nbkt = sizeof(buckets) / sizeof(size_t);
  size_t i, j;

  for (i = 0; i < nbkt; i++)
    buckets[i] = 0;

  for (i = 0; i < nbkt * 100; i++) {
    // generate a random key
    size_t keylen = 2 + random() % maxkeylen;
    char *key = (char*)zmalloc(keylen+1);
    for (j = 0; j < keylen; j++)
      key[j] = 1 + random() % 254;
    key[keylen] = '\0';
    // hash into bucket
    buckets[hf(key) % nbkt]++;
    free(key);
  }

  size_t min = 0, max = 0, empty = 0;
  for (i = 0; i < nbkt; i++) {
    if (buckets[i] < min || i == 0) min = buckets[i];
    if (buckets[i] > max || i == 0) max = buckets[i];
    if (buckets[i] == 0) empty++;
  }

#ifdef _DEBUG_
  fprintf(stderr,
      " bucket stats: n %lu, empty %lu, min %lu, max %lu, total %lu, avg %f\n",
      nbkt, empty, min, max, 100 * nbkt,
      (float)(100 * nbkt) / (float)(nbkt - empty));
#endif

  assert(max - min < 10 * 100*nbkt / (nbkt - empty));
  assert(empty == 0 || 100*nbkt < 10 * nbkt);
}


#ifdef _DEBUG_
void rehash_test() {
  hashtbl_t *h = hashtbl_init(NULL, NULL);
  hashtbl_allow_dups(h, 1);
  size_t i, j;
  for (i = 0; i < 1000000; i++) {
    // generate a random key
    size_t keylen = 3 + random() % 16;
    unsigned char *key = zmalloc(keylen+1);
    for (j = 0; j < keylen; j++)
      key[j] = 1 + random() % 254;
    key[keylen] = '\0';
    hashtbl_insert(h, (char*)key, NULL);
    free(key);
    if (i%1000)
      fprintf(stderr, "\rinserting ... %lu%%\t\t\t", 100*i/1000000);
  }
  hashtbl_destroy(h);
}
#endif

#define ITERATIONS 60
int main(void) {
  int i;
  for (i = 1; i <= ITERATIONS; i++) {
    fprintf(stderr, "\rtesting ... %d%%", 100*i/ITERATIONS);
    hashtbl_t *h = generate_test_ht(i % 2);
#ifdef _DEBUG_
    check_hash_distribution(h);
#endif
    switch (i % 20) {
      case 0: check_hash_function(djb_hash, i); break;
      case 1: check_hash_function(sbox_hash, i); break;
      default: break;
    }
    hashtbl_destroy(h);
  }
#ifdef _DEBUG_
  rehash_test();
#endif
  fprintf(stderr, "\n");
  return 0;
}

/* vim: set sw=2 sts=2 : */
