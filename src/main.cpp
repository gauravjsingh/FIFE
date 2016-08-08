#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <chrono>
#include <cstdlib>

#include <msgpack.hpp>

#include "pke/pke.h"
#include "oneqfe/ss.h"
#include "oneqfe/singleton.h"
#include "bounded/gvw.h"
#include "bounded/stateful.h"
#include "circuit/circuit.h"

/* This takes as input a text file with a list of options, and then outputs the results of using the specified type of functional encryption scheme to the specified type of circuit, giving the running times for each of Setup, KeyGen, Encrypt, and Decrypt, along with sizes for the MasterPublicKey, MasterSecretKey, SecretKey, and Ciphertext.
 */

// Generate a random instance of the given circuit type
void randomInnerProductModPCircuit(Circuit **circuit, int mod, int length) {
  std::vector<int> circ(length);

  for (size_t i = 0; i < circ.size(); i++) {
    circ[i] = rand() % mod;
  }

  *circuit = new InnerProductModPCircuit(mod, circ);
}

void randomParityCircuit(Circuit **circuit, int length) {
  std::vector<int> circ(length);

  for (size_t i = 0; i < circ.size(); i++) {
    circ[i] = rand() % 2;
  }

  *circuit = new ParityCircuit(circ);
}

void randomHammingCircuit(Circuit **circuit, int length) {
  std::vector<int> circ(length);

  for (size_t i = 0; i < circ.size(); i++) {
    circ[i] = rand() % 2;
  }

  *circuit = new HammingCircuit(circ);
}

void randomLevenshteinCircuit(Circuit **circuit, int inputLen, int circuitLen, int alphabetBits) {
  std::vector<int> circ(circuitLen);

  for (size_t i = 0; i < circ.size(); i++) {
    circ[i] = rand() % (1 << alphabetBits);
  }

  *circuit = new LevenshteinCircuit(circ, inputLen, alphabetBits);
}

// give the space required to write an element to file, for keys and ciphertexts
template <class Writable>
uint64_t fileSize(Writable w, std::string fileName) {
  writeToFile(w, fileName);
  std::ifstream file(fileName, std::ios::in|std::ios::binary|std::ios::ate);
  std::streampos size;

  size = file.tellg();
  file.close();

  return (uint64_t) size;
}

// Take an input configuration, and set up the appropriate circuit description
void handleCircOptions(CircuitDescription** desc, std::map<std::string, std::string> config) {
  if (config["circuit_type"] == "inner_product_mod_p") {
    int mod = std::stoi(config["circuit_modulus"]);
    int numbers = std::stoi(config["circuit_input_length"]);
    *desc = new InnerProductModPCircuitDescription(mod, numbers);
  } else if (config["circuit_type"] == "parity") {
    int numbers = std::stoi(config["circuit_input_length"]);
    *desc = new ParityCircuitDescription(numbers);
  } else if (config["circuit_type"] == "hamming") {
    int numbers = std::stoi(config["circuit_input_length"]);
    *desc = new HammingCircuitDescription(numbers);
  } else if (config["circuit_type"] == "levenshtein") {
    int inputLen = std::stoi(config["circuit_input_length"]);
    int circuitLen = std::stoi(config["circuit_circuit_length"]);
    int bits = std::stoi(config["levenshtein_alphabet_bits"]);
    *desc = new LevenshteinCircuitDescription(inputLen, circuitLen, bits);
  } else {
    throw std::runtime_error("Unsupported circuit type for One Query FE.");
  }
}

// Takes a functional encryption scheme, and tests it to get running times and
// key and ciphertext sizes
template <class FE>
void handleFEOptions(FE fe, std::map<std::string, std::string> config) {
  std::ofstream results;
  results.open(config["results_file_name"]);

  auto t1 = std::chrono::high_resolution_clock::now();
  typename FE::KeyPair p = fe.Setup(std::stoi(config["base_security_parameter"]));
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> ms = t2 - t1;
  results << "Setup took: " << ms.count() << " ms" << std::endl;
  results << "Master Secret Key size: " << fileSize(p.sk, config["master_secret_key_file_name"]) << std::endl;
  results << "Master Public Key size: " << fileSize(p.pk, config["master_public_key_file_name"]) << std::endl;

  Circuit *circuit;

  if (config["circuit_type"] == "inner_product_mod_p") {
    randomInnerProductModPCircuit(&circuit, std::stoi(config["circuit_modulus"]), std::stoi(config["circuit_input_length"]));
  } else if (config["circuit_type"] == "parity") {
    randomParityCircuit(&circuit, std::stoi(config["circuit_input_length"]));
  } else if (config["circuit_type"] == "hamming") {
    randomHammingCircuit(&circuit, std::stoi(config["circuit_input_length"]));
  } else if (config["circuit_type"] == "levenshtein") {
    randomLevenshteinCircuit(&circuit, std::stoi(config["circuit_input_length"]), std::stoi(config["circuit_circuit_length"]), std::stoi(config["levenshtein_alphabet_bits"]));
  } else {
    throw std::runtime_error("Unrecognized circuit type.");
  }

  t1 = std::chrono::high_resolution_clock::now();
  typename FE::SecretKey sk = fe.KeyGen(p.sk, circuit);
  t2 = std::chrono::high_resolution_clock::now();
  ms = t2 - t1;
  results << "KeyGen took: " << ms.count() << " ms" << std::endl;
  results << "Functional Key size: " << fileSize(sk, config["functional_key_file_name"]) << std::endl;

  std::vector<int> msg;

  if (config["circuit_type"] == "inner_product_mod_p") {
    msg.resize(((InnerProductModPCircuit *) circuit)->circuit.size());
    for (size_t i = 0; i < msg.size(); i++) {
      msg[i] = rand() % ((InnerProductModPCircuit *) circuit)->mod;
    }
  } else if (config["circuit_type"] == "parity") {
    msg.resize(((ParityCircuit *) circuit)->circuit.size());
    for (size_t i = 0; i < msg.size(); i++) {
      msg[i] = rand() % 2;
    }
  } else if (config["circuit_type"] == "hamming") {
    msg.resize(((HammingCircuit *) circuit)->circuit.size());
    for (size_t i = 0; i < msg.size(); i++) {
      msg[i] = rand() % 2;
    }
  } else if (config["circuit_type"] == "levenshtein") {
    msg.resize(((LevenshteinCircuit *) circuit)->inputLen);
    for (size_t i = 0; i < msg.size(); i++) {
      msg[i] = rand() % (1 << ((LevenshteinCircuit *) circuit)->alphabetBits);
    }
  } else {
    throw std::runtime_error("Unrecognized circuit type.");
  }

  t1 = std::chrono::high_resolution_clock::now();
  typename FE::CipherText ct = fe.Encrypt(p.pk, msg);
  t2 = std::chrono::high_resolution_clock::now();
  ms = t2 - t1;
  results << "Encryption took: " << ms.count() << " ms" << std::endl;
  results << "CipherText size: " << fileSize(ct, config["cipher_text_file_name"]) << std::endl;

  t1 = std::chrono::high_resolution_clock::now();
  fe.Decrypt(sk, ct);
  t2 = std::chrono::high_resolution_clock::now();
  ms = t2 - t1;
  results << "Decryption took: " << ms.count() << " ms" << std::endl;

  results.close();
}

// Takes a public or private key encryption scheme, and tests it to get 
// running times and key and ciphertext sizes
template <class ES>
void handleESOptions(ES es, std::map<std::string, std::string> config) {
  if (config.count("setup") > 0) {
    typename ES::KeyPair p = es.Setup(std::stoi(config["base_security_parameter"]));
    writeToFile(p.sk, config["secret_key_file_name"]);
    writeToFile(p.pk, config["public_key_file_name"]);
  }

  if (config.count("encrypt") > 0) {
    typename ES::PublicKey pk;
    readFromFile(pk, config["public_key_file_name"]);
    typename ES::PlainText msg;
    readFromFile(msg, config["plain_text_file_name"]);

    typename ES::CipherText ct = es.Encrypt(pk, msg);
    writeToFile(ct, config["cipher_text_file_name"]);
  }

  if (config.count("decrypt") > 0) {
    typename ES::SecretKey sk;
    readFromFile(sk, config["secret_key_file_name"]);
    typename ES::CipherText ct;
    readFromFile(ct, config["cipher_text_file_name"]);

    typename ES::PlainText pt = es.Decrypt(sk, ct);
    writeToFile(pt, config["plain_text_file_name"]);
  }
}

int main(int argc, char *argv[]) {
  std::string configFile;

  if (argc > 1) {
    configFile = std::string(argv[1]);
  } else {
    configFile = "test/tmp/config";
  }

  std::ifstream conf;
  conf.open(configFile);

  std::map<std::string, std::string> config;

  std::string paramName, paramValue;

  while (conf >> paramName >> paramValue) {
    config.insert(std::make_pair(paramName, paramValue));
  }

  conf.close();

  if (config["encryption_scheme_type"] == "base") {
    if (config["base_encryption_scheme"] == "RSA") {
      RSA rsa;
      
      handleESOptions(rsa, config);
    } else {
      AES aes;

      handleESOptions(aes, config);
    }
  } else if (config["encryption_scheme_type"] == "ss") {
    CircuitDescription *desc;

    handleCircOptions(&desc, config);

    if (config["base_encryption_scheme"] == "singleton_RSA") {
      SS_SingletonRSA fe(desc);

      handleFEOptions(fe, config);
    } else if (config["base_encryption_scheme"] == "singleton_AES") {
      SS_SingletonAES fe(desc);

      handleFEOptions(fe, config);
    } else if (config["base_encryption_scheme"] == "RSA") {
      SS_RSA fe(desc);

      handleFEOptions(fe, config);
    } else {
      SS_AES fe(desc);

      handleFEOptions(fe, config);
    }
  } else if (config["encryption_scheme_type"] == "stateful") {
    CircuitDescription *desc;
    handleCircOptions(&desc, config);

    int keys = std::stoi(config["bounded_collusion_function_limit"]);

    if (config["base_encryption_scheme"] == "singleton_RSA") {
      StatefulFE_SingletonRSA fe(keys, desc);

      handleFEOptions(fe, config);
    } else if (config["base_encryption_scheme"] == "singleton_AES") {
      StatefulFE_SingletonAES fe(keys, desc);

      handleFEOptions(fe, config);
    } else if (config["base_encryption_scheme"] == "RSA") {
      StatefulFE_RSA fe(keys, desc);

      handleFEOptions(fe, config);
    } else {
      StatefulFE_AES fe(keys, desc);

      handleFEOptions(fe, config);
    }
  } else {
    int keys = std::stoi(config["bounded_collusion_function_limit"]);
    int depth = std::stoi(config["bounded_collusion_circuit_depth"]);
    int secret_shares = std::stoi(config["gvw_secret_shares"]);
    int total_shares = std::stoi(config["gvw_total_shares"]);
    int delta_size = std::stoi(config["gvw_delta_size"]);
    int delta_pool_size = std::stoi(config["gvw_delta_pool_size"]);
    bool useDelta = (config["gvw_use_delta"] == "yes");
    int modulus = std::stoi(config["circuit_modulus"]);
    CircuitDescription *desc;

    if (config["circuit_type"] == "inner_product_mod_p") {
      int mod = std::stoi(config["circuit_modulus"]);
      int numbers = std::stoi(config["circuit_input_length"]);
      desc = new InnerProductModPCircuitDescription(mod, numbers);
    } else {
      throw std::runtime_error("Unsupported circuit type for Bounded Collusion FE.");
    }

    if (config["base_encryption_scheme"] == "singleton_RSA") {
      GVW_SS_SingletonRSA fe(keys, depth, secret_shares, total_shares, delta_size, delta_pool_size, modulus, useDelta, desc);

      handleFEOptions(fe, config);
    } else if (config["base_encryption_scheme"] == "singleton_AES") {
      GVW_SS_SingletonAES fe(keys, depth, secret_shares, total_shares, delta_size, delta_pool_size, modulus, useDelta, desc);

      handleFEOptions(fe, config);
    } else if (config["base_encryption_scheme"] == "RSA") {
      GVW_SS_RSA fe(keys, depth, secret_shares, total_shares, delta_size, delta_pool_size, modulus, useDelta, desc);

      handleFEOptions(fe, config);
    } else {
      GVW_SS_AES fe(keys, depth, secret_shares, total_shares, delta_size, delta_pool_size, modulus, useDelta, desc);

      handleFEOptions(fe, config);
    }
  }
}
