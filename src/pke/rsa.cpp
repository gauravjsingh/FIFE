#include <assert.h>
#include <iostream>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/files.h>
#include <crypto++/base64.h>

#include "pke/pke.h"

/* RSA methods.
 */

template<>
RSA::KeyPair RSA::Setup(int length) {
  RSA::SecretKey sk;
  CryptoPP::AutoSeededRandomPool rng;
  sk.sk.GenerateRandomWithKeySize(rng, length);
  
  RSA::PublicKey pk(sk);

  return RSA::KeyPair(sk, pk);
}

template<>
RSA::CipherText RSA::Encrypt(RSA::PublicKey pk, RSA::PlainText msg) {
  CryptoPP::RSAES_OAEP_SHA_Encryptor e(pk.pk);

  size_t ct_size = e.CiphertextLength(msg.size());

  RSA::CipherText ct(ct_size);
  CryptoPP::AutoSeededRandomPool rng;

  CryptoPP::ArraySource ss(msg.data(), msg.size(), true,
      new CryptoPP::PK_EncryptorFilter(rng, e, new CryptoPP::ArraySink(ct.ct.data(), ct.ct.size())));

  return ct;
}

template<>
RSA::PlainText RSA::Decrypt(RSA::SecretKey sk, RSA::CipherText ct) {
  CryptoPP::RSAES_OAEP_SHA_Decryptor d(sk.sk);

  size_t pt_size = d.MaxPlaintextLength(ct.ct.size());

  RSA::PlainText pt(pt_size);
  CryptoPP::AutoSeededRandomPool rng;

  CryptoPP::ArraySource ss(ct.ct.data(), ct.ct.size(), true, 
      new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::ArraySink(pt.data(), pt.size())));

  return pt;
}
