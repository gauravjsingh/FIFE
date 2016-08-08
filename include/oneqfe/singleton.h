#ifndef SINGLETON
#define SINGLETON

#include <string>
#include <msgpack.hpp>

/* This class template turns an existing functional encryption scheme into
 * one using noncommitting encryption, to aid in achieving adaptive security.
 */

template <class ES>
class Singleton {
public:
  struct MasterSecretKey {
    std::pair<typename ES::SecretKey, typename ES::SecretKey> sks;

    MSGPACK_DEFINE(sks);
  };

  struct MasterPublicKey {
    std::pair<typename ES::PublicKey, typename ES::PublicKey> pks;

    MSGPACK_DEFINE(pks);
  };

  struct KeyPair {
    MasterSecretKey sk;
    MasterPublicKey pk;
  };

  struct SecretKey {
    int bit;
    typename ES::SecretKey sk;

    MSGPACK_DEFINE(bit, sk);
 };

  struct CipherText {
    std::pair<typename ES::CipherText, typename ES::CipherText> cts;

    MSGPACK_DEFINE(cts);
  };

  typedef typename ES::PlainText PlainText;

  static KeyPair Setup(int length) {
    KeyPair p;
    typename ES::KeyPair p1 = ES::Setup(length);
    typename ES::KeyPair p2 = ES::Setup(length);

    p.sk.sks.first = p1.sk;
    p.sk.sks.second = p2.sk;
    p.pk.pks.first = p1.pk;
    p.pk.pks.second = p2.pk;

    return p;
  };

  static SecretKey KeyGen(MasterSecretKey msk) {
    SecretKey sk;

    int bit = rand() % 2;
    sk.bit = bit;
    if (bit == 0) {
      sk.sk = msk.sks.first;
    } else {
      sk.sk = msk.sks.second;
    }

    return sk;
  };

  static CipherText Encrypt(MasterPublicKey mpk, PlainText msg) {
    CipherText ct;
    
    ct.cts.first = ES::Encrypt(mpk.pks.first, msg);
    ct.cts.second = ES::Encrypt(mpk.pks.second, msg);

    return ct;
  };

  static PlainText Decrypt(SecretKey sk, CipherText ct) {
    if (sk.bit == 0) {
      return ES::Decrypt(sk.sk, ct.cts.first);
    } else {
      return ES::Decrypt(sk.sk, ct.cts.second);
    }
  };
};

typedef Singleton<AES> SingletonAES;
typedef Singleton<RSA> SingletonRSA;

#endif
