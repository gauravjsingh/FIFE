#ifndef PKE_TYPES
#define PKE_TYPES

#include <crypto++/rsa.h>
#include <crypto++/aes.h>
#include <string>
#include <msgpack.hpp>

#include "file/writable.h"

//Default size for AES keys (128 bits)
#ifndef AES_DEFAULT_KEYLENGTH
#define AES_DEFAULT_KEYLENGTH CryptoPP::AES::DEFAULT_KEYLENGTH
#endif

/* This file defines the types used by public and secret key encryption schemes. * Also included are methods for reading and writing keys for these schemes.
 */

// Types for RSA, a PKE scheme.
class RSATypes {
 public:
  struct SecretKey {
    CryptoPP::RSA::PrivateKey sk;

    template <typename Packer> inline void msgpack_pack(Packer& pk) const;

    inline void msgpack_unpack(msgpack::object const& o);
  };

  struct PublicKey {
    CryptoPP::RSA::PublicKey pk;

    PublicKey() {};
    PublicKey(SecretKey sk): pk(sk.sk) {};

    template <typename Packer> inline void msgpack_pack(Packer& pk) const;

    inline void msgpack_unpack(msgpack::object const& o);
  };

  struct CipherText {
    std::vector<unsigned char> ct;

    CipherText() {};
    CipherText(size_t size): ct(size) {};

    MSGPACK_DEFINE(ct);
  };
};

// Types for AES, a SKE scheme.
class AESTypes {
public:
  struct Key {
    CryptoPP::SecByteBlock key;

    Key() {};
    Key(unsigned char *byte, int length): key(byte, length) {};

    template <typename Packer> inline void msgpack_pack(Packer& pk) const;

    inline void msgpack_unpack(msgpack::object const& o);
  };

  typedef Key SecretKey;

  typedef Key PublicKey;

  struct CipherText {
    std::vector<unsigned char> ct;
    std::vector<unsigned char> iv;

    CipherText() {};
    CipherText(size_t ct_size, std::vector<unsigned char> iv): ct(ct_size), iv(iv) {};

    MSGPACK_DEFINE(ct, iv);
  };
};

// Packing integers, used to pack RSA keys.
template <typename Packer> inline void packInteger(Packer& pk, CryptoPP::Integer x) {
  std::vector<unsigned char> output(x.MinEncodedSize());
  x.Encode(output.data(), output.size());
  pk.pack_bin(output.size());
  pk.pack_bin_body((const char *) output.data(), output.size());
};

// Unpakcing integers, used to unpack RSA keys.
inline CryptoPP::Integer unpackInteger(msgpack::object const& o) {
  if (o.type != msgpack::type::BIN) { throw msgpack::type_error(); }
  CryptoPP::Integer x;
  x.Decode((unsigned char *) o.via.bin.ptr, o.via.bin.size);
  return x;
};

// Packing RSA Secret Keys.
template <typename Packer> void RSATypes::SecretKey::msgpack_pack(Packer& pk) const {
  pk.pack_array(8);

  packInteger(pk, sk.GetModulus());
  packInteger(pk, sk.GetPublicExponent());
  packInteger(pk, sk.GetPrivateExponent());
  packInteger(pk, sk.GetPrime1());
  packInteger(pk, sk.GetPrime2());
  packInteger(pk, sk.GetModPrime1PrivateExponent());
  packInteger(pk, sk.GetModPrime2PrivateExponent());
  packInteger(pk, sk.GetMultiplicativeInverseOfPrime2ModPrime1());
};

// Unpacking RSA Secret Keys.
void RSATypes::SecretKey::msgpack_unpack(msgpack::object const& o) {
  if (o.type != msgpack::type::ARRAY) { throw msgpack::type_error(); }
  const size_t size = o.via.array.size;
  assert(size == 8);
  CryptoPP::Integer n, e, d, p, q, dp, dq, u;
  n = unpackInteger(o.via.array.ptr[0]);
  e = unpackInteger(o.via.array.ptr[1]);
  d = unpackInteger(o.via.array.ptr[2]);
  p = unpackInteger(o.via.array.ptr[3]);
  q = unpackInteger(o.via.array.ptr[4]);
  dp = unpackInteger(o.via.array.ptr[5]);
  dq = unpackInteger(o.via.array.ptr[6]);
  u = unpackInteger(o.via.array.ptr[7]);
  sk.Initialize(n, e, d, p, q, dp, dq, u);
};

// Packing RSA Public Keys.
template <typename Packer> void RSATypes::PublicKey::msgpack_pack(Packer& pk) const {
  pk.pack_array(2);

  packInteger(pk, this->pk.GetModulus());
  packInteger(pk, this->pk.GetPublicExponent());
};

// Unpacking RSA Public Keys.
void RSATypes::PublicKey::msgpack_unpack(msgpack::object const& o) {
  if (o.type != msgpack::type::ARRAY) { throw msgpack::type_error(); }
  const size_t size = o.via.array.size;
  assert(size == 2);
  CryptoPP::Integer n, e;
  n = unpackInteger(o.via.array.ptr[0]);
  e = unpackInteger(o.via.array.ptr[1]);
  this->pk.Initialize(n, e);
};

// Packing AES Keys.
template <typename Packer> void AESTypes::Key::msgpack_pack(Packer& pk) const {
  pk.pack_bin(key.size());
  pk.pack_bin_body((const char *) key.data(), key.size());
};

// Unpacking AES Keys.
void AESTypes::Key::msgpack_unpack(msgpack::object const& o) {
  if (o.type != msgpack::type::BIN) { throw msgpack::type_error(); }
  key = CryptoPP::SecByteBlock((unsigned char *) o.via.bin.ptr, o.via.bin.size);
};

#endif
