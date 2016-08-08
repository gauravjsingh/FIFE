#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>

#include <NTL/lzz_pX.h>

#include "oneqfe/ss.h"
#include "bounded/stateful.h"

template<class OneQS>
StatefulFE<OneQS>::StatefulFE(int keys, CircuitDescription *description) {
  key_limit = keys;
  oneqfe = new OneQS(description);
  state = 0;
}

template<class OneQS>
typename StatefulFE<OneQS>::KeyPair StatefulFE<OneQS>::Setup(int length) {
  typename StatefulFE<OneQS>::KeyPair p;
  p.sk = StatefulFE<OneQS>::MasterSecretKey(key_limit);
  p.pk = StatefulFE<OneQS>::MasterPublicKey(key_limit);

  for (int i = 0; i < key_limit; i++) {
    typename OneQS::KeyPair p1 = oneqfe->Setup(length);
    p.sk.sks[i] = p1.sk;
    p.pk.pks[i] = p1.pk;
  }

  return p;
}

template<class OneQS>
typename StatefulFE<OneQS>::SecretKey StatefulFE<OneQS>::KeyGen(typename StatefulFE<OneQS>::MasterSecretKey msk, Circuit *circuit) {
  if (state >= key_limit) {
    throw std::runtime_error("Too many keys already issued.");
  }

  typename StatefulFE<OneQS>::SecretKey sk;

  sk.index = state;
  sk.sk = oneqfe->KeyGen(msk.sks[state], circuit);

  state++;
  return sk;
}

template<class OneQS>
typename StatefulFE<OneQS>::CipherText StatefulFE<OneQS>::Encrypt(typename StatefulFE<OneQS>::MasterPublicKey mpk, std::vector<int> msg) {
  typename StatefulFE<OneQS>::CipherText ct(key_limit);

  for (int i = 0; i < key_limit; i++) {
    ct.cts[i] = oneqfe->Encrypt(mpk.pks[i], msg);
  }

  return ct;
}

template<class OneQS>
std::vector<int> StatefulFE<OneQS>::Decrypt(typename StatefulFE<OneQS>::SecretKey sk, typename StatefulFE<OneQS>::CipherText ct) {
  return oneqfe->Decrypt(sk.sk, ct.cts[sk.index]);
}

template class StatefulFE<SS_AES>;
template class StatefulFE<SS_RSA>;
template class StatefulFE<SS_SingletonAES>;
template class StatefulFE<SS_SingletonRSA>;
