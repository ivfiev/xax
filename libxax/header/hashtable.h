#ifndef XAX_HASHTABLE_H
#define XAX_HASHTABLE_H

#include "types.h"

#define KV(def) ((kv) {def})

#define FOREACH_KV(hash_tbl, code) \
  do {                                 \
  kv *kvs = hash_keys(hash_tbl);   \
  size_t tbl_len = hash_tbl->len;                                 \
  for (int k_ix = 0; k_ix < tbl_len; k_ix++) { \
    kv key = kvs[k_ix];       \
    kv val = hash_getv(hash_tbl, key);  \
    code \
  } \
  free(kvs);                        \
  } while (0)

hashtable *hash_new(size_t cap, uint64_t (*hash)(kv k, size_t N), int (*cmp)(kv k1, kv k2));

void hash_set(hashtable *ht, kv k, kv v);

int hash_hask(hashtable *ht, kv k);

kv hash_getv(hashtable *ht, kv k);

void hash_del(hashtable *ht, kv k);

kv *hash_keys(hashtable *ht);

void hash_free(hashtable *ht);

uint64_t hash_int(kv k, size_t N);

uint64_t hash_str(kv k, size_t N);

int hash_cmp_str(kv k1, kv k2);

int hash_cmp_int(kv k1, kv k2);

#endif