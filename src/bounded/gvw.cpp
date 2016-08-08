#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>

#include <NTL/lzz_pX.h>

#include "oneqfe/ss.h"
#include "bounded/gvw.h"

template<class OneQS>
GVW<OneQS>::GVW(int keys, int depth, int kappa, int modulus, bool useDelta, CircuitDescription *description) {
  key_limit = keys;
  this->depth = depth;
  secret_shares = key_limit * key_limit * kappa;
  total_shares = depth * depth * key_limit * key_limit * secret_shares;
  delta_size = kappa;
  delta_pool_size = delta_size * key_limit * key_limit;
  this->useDelta = useDelta;

  assert(description->type == CIRCUIT_TYPE_INNER_PRODUCT_MOD_P);
  if (useDelta) {
    description = new InnerProductModPDeltaCircuitDescription(description->getMod(), description->circuit_size/description->getModBits(), delta_pool_size);
  }
  oneqfe = new OneQS(description);
  assert(modulus > total_shares);
  NTL::zz_p::init(modulus);
}

template<class OneQS>
GVW<OneQS>::GVW(int keys, int depth, int secret_shares, int total_shares, int delta_size, int delta_pool_size, int modulus, bool useDelta, CircuitDescription *description) {
  key_limit = keys;
  this->depth = depth;
  this->total_shares = total_shares;
  this->secret_shares = secret_shares;
  this->delta_size = delta_size;
  this->delta_pool_size = delta_pool_size;
  this->useDelta = useDelta;

  assert(description->type == CIRCUIT_TYPE_INNER_PRODUCT_MOD_P);
  if (useDelta) {
    description = new InnerProductModPDeltaCircuitDescription(description->getMod(), description->circuit_size/description->getModBits(), delta_pool_size);
  }
  oneqfe = new OneQS(description);
  assert(modulus > total_shares);
  NTL::zz_p::init(modulus);
}

template<class OneQS>
typename GVW<OneQS>::KeyPair GVW<OneQS>::Setup(int length) {
  typename GVW<OneQS>::KeyPair p;
  p.sk = GVW<OneQS>::MasterSecretKey(total_shares);
  p.pk = GVW<OneQS>::MasterPublicKey(total_shares);

  for (int i = 0; i < total_shares; i++) {
    typename OneQS::KeyPair p1 = oneqfe->Setup(length);
    p.sk.sks[i] = p1.sk;
    p.pk.pks[i] = p1.pk;
  }

  return p;
}

template<class OneQS>
typename GVW<OneQS>::SecretKey GVW<OneQS>::KeyGen(typename GVW<OneQS>::MasterSecretKey msk, Circuit *circuit) {
  typename GVW<OneQS>::SecretKey sk;

  // Pick random instances for the secret_shares
  sk.Gamma.resize(total_shares);
  for (size_t i = 0; i < sk.Gamma.size(); i++) {
    sk.Gamma[i] = i;
  }
  std::random_shuffle(sk.Gamma.begin(), sk.Gamma.end());
  sk.Gamma.resize(secret_shares * depth + 1);

  if (useDelta) {
    // Pick random values for Delta.
    sk.Delta.resize(delta_pool_size);
    for (size_t i = 0; i < sk.Delta.size(); i++) {
      sk.Delta[i] = i;
    }
    std::random_shuffle(sk.Delta.begin(), sk.Delta.end());
    sk.Delta.resize(delta_size);

    circuit = new InnerProductModPDeltaCircuit(*((InnerProductModPCircuit *) circuit), delta_pool_size, sk.Delta);
  }

  sk.sks.resize(secret_shares * depth + 1);
  for (size_t i = 0; i < sk.sks.size(); i++) {
    sk.sks[i] = oneqfe->KeyGen(msk.sks[sk.Gamma[i]], circuit);
  }
  return sk;
}

template<class OneQS>
typename GVW<OneQS>::CipherText GVW<OneQS>::Encrypt(typename GVW<OneQS>::MasterPublicKey mpk, std::vector<int> msg) {
  std::vector<NTL::zz_pX> msg_polys(msg.size());
  std::vector<NTL::zz_pX> zeta_polys(delta_pool_size);

  // Randomize polynomials
  for (size_t i = 0; i < msg.size(); i++) {
    NTL::random(msg_polys[i], (long) secret_shares);
    NTL::SetCoeff(msg_polys[i], 0, msg[i]);
  }

  // Randomize zeta polynomials if using
  if (useDelta) {
    for (int i = 0; i < delta_pool_size; i++) {
      NTL::random(zeta_polys[i], (long) (secret_shares * depth));
      NTL::SetCoeff(zeta_polys[i], 0, 0);
    }
  }

  typename GVW<OneQS>::CipherText ct(total_shares);

  for (int i = 0; i < total_shares; i++) {
    std::vector<int> poly_points(msg.size());

    // evaluate the polynomials 
    for (size_t j = 0; j < msg.size(); j++) {
      NTL::zz_p poly_value, poly_point;
      NTL::zz_pX poly = msg_polys[j];
      NTL::conv(poly_point, (long) i + 1);
      NTL::eval(poly_value, poly, poly_point);
      poly_points[j] = (int) NTL::rep(poly_value);
    }

    if (useDelta) {
      poly_points.resize(msg.size() + delta_pool_size);

      for (int j = 0; j < delta_pool_size; j++) {
        NTL::zz_p poly_value, poly_point;
        NTL::zz_pX poly = zeta_polys[j];
        NTL::conv(poly_point, (long) i + 1);
        NTL::eval(poly_value, poly, poly_point);
        poly_points[j + msg.size()] = (int) NTL::rep(poly_value);
      }
    }

    // Encrypt the resulting points
    ct.cts[i] = oneqfe->Encrypt(mpk.pks[i], poly_points);
  }

  return ct;
}

template<class OneQS>
std::vector<int> GVW<OneQS>::Decrypt(typename GVW<OneQS>::SecretKey sk, typename GVW<OneQS>::CipherText ct) {
  std::vector<std::vector<int> > poly_outputs(sk.Gamma.size());

  // decrypt points on each polynomial
  for (size_t i = 0; i < sk.Gamma.size(); i++) {
    poly_outputs[i] = oneqfe->Decrypt(sk.sks[i], ct.cts[sk.Gamma[i]]);
  }

  std::vector<int> outputs(poly_outputs[0].size());
  for (size_t i = 0; i < outputs.size(); i++) {
    NTL::zz_pX f;

    // Set up values for polynomial interpolation
    NTL::vec_zz_p poly_in, poly_out;
    poly_in.SetLength(sk.Gamma.size());
    poly_out.SetLength(sk.Gamma.size());
    for (size_t j = 0; j < sk.Gamma.size(); j++) {
      poly_in[j] = sk.Gamma[j] + 1;
      poly_out[j] = (long) poly_outputs[j][i];
    }

    // Interpolate, and recover the desired outputs
    NTL::interpolate(f, poly_in, poly_out);
    NTL::zz_p zeroval, zero;
    NTL::conv(zero, (long) 0);
    NTL::eval(zeroval, f, zero);
    outputs[i] = (int) NTL::rep(zeroval);
  }

  return outputs;
}

template class GVW<SS_AES>;
template class GVW<SS_RSA>;
template class GVW<SS_SingletonAES>;
template class GVW<SS_SingletonRSA>;
