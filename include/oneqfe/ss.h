#ifndef SS_H
#define SS_H

#include <vector>
#include <string>
#include <iostream>
#include <msgpack.hpp>

#include "circuit/circuit.h"
#include "pke/pke.h"
#include "oneqfe/esWrapper.h"
#include "oneqfe/singleton.h"

#include "libgarble/garble.h"
#include "libgarble/garbled_info.h"

/* A template using a functional encryption scheme to implement the
 * Sehai-Seyalioglu functional encryption scheme.
 */

template <class ES>
class SS {
private:
  CircuitDescription *circuitDescription;

public:
  struct MasterSecretKey {
    //size circuit_size
    std::vector<std::pair<typename ES::MasterSecretKey, typename ES::MasterSecretKey> > sks;

    MasterSecretKey() {};
    MasterSecretKey(int size): sks(size) {};

    MSGPACK_DEFINE(sks);
  };

  struct MasterPublicKey {
    //size circuit_size
    std::vector<std::pair<typename ES::MasterPublicKey, typename ES::MasterPublicKey> > pks;

    MasterPublicKey() {};
    MasterPublicKey(int size): pks(size) {};

    MSGPACK_DEFINE(pks);
  };

  struct KeyPair {
    MasterSecretKey sk;
    MasterPublicKey pk;
  };

  struct SecretKey {
    std::vector<int> bits; // size circuit_size
    std::vector<typename ES::SecretKey> sks; // size circuit_size

    MSGPACK_DEFINE(bits, sks);
  };

  struct CipherText {
    GarbledInfo garbled_info;
    std::vector<block> labels; //size input_size
    std::vector<std::pair<typename ES::CipherText, typename ES::CipherText> > inputs; //size circuit_size
 
    template <typename Packer> inline void msgpack_pack(Packer& pk) const;

    inline void msgpack_unpack(msgpack::object const& o);
  };

  SS(CircuitDescription *description);

  KeyPair Setup(int length);

  SecretKey KeyGen(MasterSecretKey msk, Circuit *circuit);

  CipherText Encrypt(MasterPublicKey mpk, std::vector<int> msg);

  std::vector<int> Decrypt(SecretKey sk, CipherText ct);
};

template<class ES> template <typename Packer>
void SS<ES>::CipherText::msgpack_pack(Packer& pk) const {
  pk.pack_array(3);

  garbled_info.msgpack_pack(pk);

  pk.pack_bin(labels.size() * sizeof(block));
  pk.pack_bin_body((const char *) labels.data(), labels.size() * sizeof(block));

  pk.pack_array(inputs.size() * 2);
  for (size_t i = 0; i < inputs.size(); i ++) {
    inputs[i].first.msgpack_pack(pk);
    inputs[i].second.msgpack_pack(pk);
  }
};

template<class ES>
void SS<ES>::CipherText::msgpack_unpack(msgpack::object const& o) {
  if (o.type != msgpack::type::ARRAY) { throw msgpack::type_error(); }
  const size_t size = o.via.array.size;
  assert(size == 3);

  garbled_info.msgpack_unpack(o.via.array.ptr[0]);

  labels.resize(o.via.array.ptr[1].via.bin.size / sizeof(block));
  memcpy(labels.data(), o.via.array.ptr[1].via.bin.ptr, o.via.array.ptr[1].via.bin.size);
  
  inputs.resize(o.via.array.ptr[2].via.array.size / 2);
  for (size_t i = 0; i < inputs.size(); i++) {
    inputs[i].first.msgpack_unpack(o.via.array.ptr[2].via.array.ptr[2 * i]);
    inputs[i].second.msgpack_unpack(o.via.array.ptr[2].via.array.ptr[2 * i + 1]);
  }
};

typedef SS<AESWrapper> SS_AES;
typedef SS<RSAWrapper> SS_RSA;
typedef SS<SingletonAES> SS_SingletonAES;
typedef SS<SingletonRSA> SS_SingletonRSA;

#endif
