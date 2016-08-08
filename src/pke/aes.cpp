#include <assert.h>
#include <iostream>
#include <fstream>

#include <crypto++/aes.h>
#include <crypto++/files.h>
#include <crypto++/modes.h>
#include <crypto++/osrng.h>

#include "pke/pke.h"

/* AES methods.
 */

template<>
AES::KeyPair AES::Setup(int length) {
  AES::SecretKey sk(0x00, length);
  CryptoPP::AutoSeededRandomPool rng;
  rng.GenerateBlock(sk.key, sk.key.size());
  
  AES::PublicKey pk = sk;

  return AES::KeyPair(sk, pk);
}

template<>
AES::CipherText AES::Encrypt(AES::PublicKey pk, AES::PlainText msg) {
  std::vector<unsigned char> iv(CryptoPP::AES::BLOCKSIZE);
  CryptoPP::AutoSeededRandomPool rng;
  rng.GenerateBlock(iv.data(), CryptoPP::AES::BLOCKSIZE);
  CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption e(pk.key, pk.key.size(), iv.data());

  size_t ct_size = msg.size();

  AES::CipherText ct(ct_size, iv);

  e.ProcessData(ct.ct.data(), msg.data(), msg.size());

  return ct;
}

template<>
AES::PlainText AES::Decrypt(AES::SecretKey sk, AES::CipherText ct) {
  CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption d(sk.key, sk.key.size(), ct.iv.data());

  size_t pt_size = ct.ct.size();

  AES::PlainText pt(pt_size);

  d.ProcessData(pt.data(), ct.ct.data(), ct.ct.size());

  return pt;
}
