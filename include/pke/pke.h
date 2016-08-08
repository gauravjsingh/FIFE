#ifndef PKE_H
#define PKE_H

#include <string>

#include "types.h"
#include "file/writable.h"

/* This defines the methods a PKE or SKE should support. For an SKE, we use
 * PublicKey and SecretKey to mean the same key, for simplicity of code.
 * This also instantiates the templates for the supported schemes, and makes
 * helpful typedefs.
 */

template <class Types>
class PKEBase {
public:
  typedef typename Types::SecretKey SecretKey;

  typedef typename Types::PublicKey PublicKey;

  struct KeyPair {
    SecretKey sk;
    PublicKey pk;

    KeyPair(SecretKey sk, PublicKey pk): sk(sk), pk(pk) {}
  };

  typedef typename Types::CipherText CipherText;

  typedef std::vector<unsigned char> PlainText;

  static KeyPair Setup(int length);

  static CipherText Encrypt(PublicKey pk, PlainText msg);

  static PlainText Decrypt(SecretKey sk, CipherText ct);
};

template class PKEBase<AESTypes>;

template class PKEBase<RSATypes>;

typedef PKEBase<AESTypes> AES;

typedef PKEBase<RSATypes> RSA;

#endif
