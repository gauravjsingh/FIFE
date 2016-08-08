#include <vector>
#include <iostream>

#include "pke/pke.h"
#include "oneqfe/ss.h"
#include "bounded/gvw.h"

#include "gtest/gtest.h"

class FileTest : public testing::Test {
 protected:
  void EXPECT_EQ_BLOCKS(block a, block b) {
    uint64_t *vala = (uint64_t *) &a;
    uint64_t *valb = (uint64_t *) &b;

    EXPECT_EQ(vala[0], valb[0]);
    EXPECT_EQ(vala[1], valb[1]);
  }
};



TEST_F(FileTest, AESKey) {
  AES aes;
  AES::KeyPair p = aes.Setup(AES_DEFAULT_KEYLENGTH);

  AES::PublicKey pk;

  writeToFile(p.pk, "test/tmp/tmp-aes-key");
  readFromFile(pk, "test/tmp/tmp-aes-key");

  std::string msg = "test message";

  const unsigned char *bytes = (const unsigned char*) msg.c_str();

  AES::PlainText msg_pt(bytes, bytes + msg.size());

  EXPECT_EQ(msg_pt, aes.Decrypt(p.sk, aes.Encrypt(pk, msg_pt)));
}

TEST_F(FileTest, RSAKeys) {
  RSA rsa;
  RSA::KeyPair p = rsa.Setup(3072);

  RSA::PublicKey pk;
  RSA::SecretKey sk;

  writeToFile(p.pk, "test/tmp/tmp-rsa-pk");
  writeToFile(p.sk, "test/tmp/tmp-rsa-sk");
  readFromFile(pk, "test/tmp/tmp-rsa-pk");
  readFromFile(sk, "test/tmp/tmp-rsa-sk");

  std::string msg = "test message";

  const unsigned char *bytes = (const unsigned char*) msg.c_str();

  RSA::PlainText msg_pt(bytes, bytes + msg.size());

  RSA::PlainText out1 = rsa.Decrypt(p.sk, rsa.Encrypt(pk, msg_pt));
  RSA::PlainText out2 = rsa.Decrypt(sk, rsa.Encrypt(p.pk, msg_pt));
  out1.resize(msg_pt.size());
  out2.resize(msg_pt.size());

  EXPECT_EQ(msg_pt, out1);
  EXPECT_EQ(msg_pt, out2);
}

TEST_F(FileTest, SSKeys) {
  Circuit *circuit = new InnerProductModPCircuit(101, {11, 2, 45, 13});
  std::vector<int> x = {100, 97, 3, 17};
  SS_AES fe(new InnerProductModPCircuitDescription(101, 4));
  SS_AES::KeyPair p = fe.Setup(AES_DEFAULT_KEYLENGTH);
  SS_AES::SecretKey sk = fe.KeyGen(p.sk, circuit);
  SS_AES::CipherText ct = fe.Encrypt(p.pk, x);
  std::vector<int> pt = fe.Decrypt(sk, ct);

  SS_AES::MasterPublicKey mpkRead;
  SS_AES::MasterSecretKey mskRead;
  SS_AES::KeyPair pRead;
  SS_AES::SecretKey skRead;
  SS_AES::CipherText ctRead;

  writeToFile(p.pk, "test/tmp/tmp-oneqfe-mpk");
  readFromFile(mpkRead, "test/tmp/tmp-oneqfe-mpk");
  writeToFile(p.sk, "test/tmp/tmp-oneqfe-msk");
  readFromFile(mskRead, "test/tmp/tmp-oneqfe-msk");

  SS_AES::SecretKey sk2 = fe.KeyGen(mskRead, circuit);
  SS_AES::CipherText ct2 = fe.Encrypt(mpkRead, x);
  std::vector<int> pt2 = fe.Decrypt(sk2, ct2);

  EXPECT_EQ(34, pt[0]);
  EXPECT_EQ(pt, pt2);
}

TEST_F(FileTest, SSSecretKey) {
  Circuit *circuit = new InnerProductModPCircuit(101, {11, 2, 45, 13});
  std::vector<int> x = {100, 97, 3, 17};
  SS_AES fe(new InnerProductModPCircuitDescription(101, 4));
  SS_AES::KeyPair p = fe.Setup(AES_DEFAULT_KEYLENGTH);
  SS_AES::SecretKey sk = fe.KeyGen(p.sk, circuit);
  SS_AES::CipherText ct = fe.Encrypt(p.pk, x);
  std::vector<int> pt = fe.Decrypt(sk, ct);

  SS_AES::SecretKey skRead;

  writeToFile(sk, "test/tmp/tmp-oneqfe-sk");
  readFromFile(skRead, "test/tmp/tmp-oneqfe-sk");

  std::vector<int> pt2 = fe.Decrypt(skRead, ct);

  EXPECT_EQ(34, pt[0]);
  EXPECT_EQ(pt, pt2);
}

TEST_F(FileTest, SSCipherText) {
  Circuit *circuit = new InnerProductModPCircuit(101, {11, 2, 45, 13});
  std::vector<int> x = {100, 97, 3, 17};
  SS_AES fe(new InnerProductModPCircuitDescription(101, 4));
  SS_AES::KeyPair p = fe.Setup(AES_DEFAULT_KEYLENGTH);
  SS_AES::SecretKey sk = fe.KeyGen(p.sk, circuit);
  SS_AES::CipherText ct = fe.Encrypt(p.pk, x);
  std::vector<int> pt = fe.Decrypt(sk, ct);

  SS_AES::CipherText ctRead;

  writeToFile(ct, "test/tmp/tmp-oneqfe-ct");
  readFromFile(ctRead, "test/tmp/tmp-oneqfe-ct");

  std::vector<int> pt2 = fe.Decrypt(sk, ctRead);

  EXPECT_EQ(34, pt[0]);
  EXPECT_EQ(pt, pt2);
}

TEST_F(FileTest, GVWKeys) {
  Circuit *circuit = new InnerProductModPCircuit(101, {11, 2, 45, 13});
  std::vector<int> x = {100, 97, 3, 17};
  GVW<SS_AES> fe(2, 2, 1, 101, false, new InnerProductModPCircuitDescription(101, 4));
  GVW<SS_AES>::KeyPair p = fe.Setup(AES_DEFAULT_KEYLENGTH);
  GVW<SS_AES>::SecretKey sk = fe.KeyGen(p.sk, circuit);
  GVW<SS_AES>::CipherText ct = fe.Encrypt(p.pk, x);
  std::vector<int> pt = fe.Decrypt(sk, ct);

  GVW<SS_AES>::MasterPublicKey mpkRead;
  GVW<SS_AES>::MasterSecretKey mskRead;
  GVW<SS_AES>::KeyPair pRead;
  GVW<SS_AES>::SecretKey skRead;
  GVW<SS_AES>::CipherText ctRead;

  writeToFile(p.pk, "test/tmp/tmp-bdfe-mpk");
  readFromFile(mpkRead, "test/tmp/tmp-bdfe-mpk");
  writeToFile(p.sk, "test/tmp/tmp-bdfe-msk");
  readFromFile(mskRead, "test/tmp/tmp-bdfe-msk");

  GVW<SS_AES>::SecretKey sk2 = fe.KeyGen(mskRead, circuit);
  GVW<SS_AES>::CipherText ct2 = fe.Encrypt(mpkRead, x);
  std::vector<int> pt2 = fe.Decrypt(sk2, ct2);

  EXPECT_EQ(34, pt[0]);
  EXPECT_EQ(pt, pt2);
}

TEST_F(FileTest, GVWKeysDelta) {
  Circuit *circuit = new InnerProductModPCircuit(101, {11, 2, 45, 13});
  std::vector<int> x = {100, 97, 3, 17};
  GVW<SS_AES> fe(2, 2, 1, 101, true, new InnerProductModPCircuitDescription(101, 4));
  GVW<SS_AES>::KeyPair p = fe.Setup(AES_DEFAULT_KEYLENGTH);
  GVW<SS_AES>::SecretKey sk = fe.KeyGen(p.sk, circuit);
  GVW<SS_AES>::CipherText ct = fe.Encrypt(p.pk, x);
  std::vector<int> pt = fe.Decrypt(sk, ct);

  GVW<SS_AES>::MasterPublicKey mpkRead;
  GVW<SS_AES>::MasterSecretKey mskRead;
  GVW<SS_AES>::KeyPair pRead;
  GVW<SS_AES>::SecretKey skRead;
  GVW<SS_AES>::CipherText ctRead;

  writeToFile(p.pk, "test/tmp/tmp-bdfe-mpk");
  readFromFile(mpkRead, "test/tmp/tmp-bdfe-mpk");
  writeToFile(p.sk, "test/tmp/tmp-bdfe-msk");
  readFromFile(mskRead, "test/tmp/tmp-bdfe-msk");

  GVW<SS_AES>::SecretKey sk2 = fe.KeyGen(mskRead, circuit);
  GVW<SS_AES>::CipherText ct2 = fe.Encrypt(mpkRead, x);
  std::vector<int> pt2 = fe.Decrypt(sk2, ct2);

  EXPECT_EQ(34, pt[0]);
  EXPECT_EQ(pt, pt2);
}

TEST_F(FileTest, GVWSecretKey) {
  Circuit *circuit = new InnerProductModPCircuit(101, {11, 2, 45, 13});
  std::vector<int> x = {100, 97, 3, 17};
  GVW<SS_AES> fe(2, 2, 1, 101, false, new InnerProductModPCircuitDescription(101, 4));
  GVW<SS_AES>::KeyPair p = fe.Setup(AES_DEFAULT_KEYLENGTH);
  GVW<SS_AES>::SecretKey sk = fe.KeyGen(p.sk, circuit);
  GVW<SS_AES>::CipherText ct = fe.Encrypt(p.pk, x);
  std::vector<int> pt = fe.Decrypt(sk, ct);

  GVW<SS_AES>::SecretKey skRead;

  writeToFile(sk, "test/tmp/tmp-bdfe-sk");
  readFromFile(skRead, "test/tmp/tmp-bdfe-sk");

  std::vector<int> pt2 = fe.Decrypt(skRead, ct);

  EXPECT_EQ(34, pt[0]);
  EXPECT_EQ(pt, pt2);
}

TEST_F(FileTest, GVWCipherText) {
  Circuit *circuit = new InnerProductModPCircuit(101, {11, 2, 45, 13});
  std::vector<int> x = {100, 97, 3, 17};
  GVW<SS_AES> fe(2, 2, 1, 101, false, new InnerProductModPCircuitDescription(101, 4));
  GVW<SS_AES>::KeyPair p = fe.Setup(AES_DEFAULT_KEYLENGTH);
  GVW<SS_AES>::SecretKey sk = fe.KeyGen(p.sk, circuit);
  GVW<SS_AES>::CipherText ct = fe.Encrypt(p.pk, x);
  std::vector<int> pt = fe.Decrypt(sk, ct);

  GVW<SS_AES>::CipherText ctRead;

  writeToFile(ct, "test/tmp/tmp-bdfe-ct");
  readFromFile(ctRead, "test/tmp/tmp-bdfe-ct");

  std::vector<int> pt2 = fe.Decrypt(sk, ctRead);

  EXPECT_EQ(34, pt[0]);
  EXPECT_EQ(pt, pt2);
}
