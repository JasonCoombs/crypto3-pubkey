//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_PUBKEY_XMSS_WOTS_PUBLICKEY_HPP
#define CRYPTO3_PUBKEY_XMSS_WOTS_PUBLICKEY_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <nil/crypto3/pubkey/pk_keys.hpp>
#include <nil/crypto3/utilities/types.hpp>

#include <nil/crypto3/pubkey/xmss/xmss_wots_parameters.hpp>
#include <nil/crypto3/pubkey/xmss/xmss_address.hpp>
#include <nil/crypto3/pubkey/xmss/xmss_hash.hpp>

namespace nil {
    namespace crypto3 {

        typedef std::vector<secure_vector<uint8_t>> wots_keysig_t;

        /**
         * A Winternitz One Time Signature public key for use with Extended Hash-Based
         * Signatures.
         **/
        class XMSS_WOTS_PublicKey : virtual public Public_Key {
        public:
            class TreeSignature final {
            public:
                TreeSignature() = default;

                TreeSignature(const wots_keysig_t &ots_sig, const wots_keysig_t &auth_path) :
                    m_ots_sig(ots_sig), m_auth_path(auth_path) {
                }

                TreeSignature(wots_keysig_t &&ots_sig, wots_keysig_t &&auth_path) :
                    m_ots_sig(std::move(ots_sig)), m_auth_path(std::move(auth_path)) {
                }

                const wots_keysig_t &ots_signature() const {
                    return m_ots_sig;
                }

                wots_keysig_t &ots_signature() {
                    return m_ots_sig;
                }

                const wots_keysig_t &authentication_path() const {
                    return m_auth_path;
                }

                wots_keysig_t &authentication_path() {
                    return m_auth_path;
                }

            private:
                wots_keysig_t m_ots_sig;
                wots_keysig_t m_auth_path;
            };

            /**
             * Creates a XMSS_WOTS_PublicKey for the signature method identified by
             * oid. The public seed for this key will be initialized with a
             * uniformly random n-byte value, where "n" is the element size of the
             * selected signature method.
             *
             * @param oid Identifier for the selected signature method.
             **/
            XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid) :
                m_wots_params(oid), m_hash(m_wots_params.hash_function_name()) {
            }

            /**
             * Creates a XMSS_WOTS_PublicKey for the signature method identified by
             * oid. The public seed for this key will be initialized with a
             * uniformly random n-byte value, where "n" is the element size of the
             * selected signature method.
             *
             * @param oid Identifier for the selected signature method.
             * @param rng A random number generate used to generate the public seed.
             **/
            XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid, RandomNumberGenerator &rng) :
                m_wots_params(oid), m_hash(m_wots_params.hash_function_name()),
                m_public_seed(rng.random_vec(m_wots_params.element_size())) {
            }

            /**
             * Creates a XMSS_WOTS_PrivateKey for the signature method identified by
             * oid, with a precomputed public seed.
             *
             * @param oid Identifier for the selected signature method.
             * @param public_seed A precomputed public seed of n-bytes length.
             **/
            XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid, secure_vector<uint8_t> public_seed) :
                m_wots_params(oid), m_hash(m_wots_params.hash_function_name()), m_public_seed(public_seed) {
            }

            /**
             * Creates a XMSS_WOTS_PublicKey for the signature method identified by
             * oid. The public seed will be initialized with a precomputed seed and
             * and precomputed key data which should be derived from a
             * XMSS_WOTS_PrivateKey.
             *
             * @param oid Ident:s/ifier for the selected signature methods.
             * @param public_seed A precomputed public seed of n-bytes length.
             * @param key Precomputed raw key data of the XMSS_WOTS_PublicKey.
             **/
            XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid, secure_vector<uint8_t> &&public_seed,
                                wots_keysig_t &&key) :
                m_wots_params(oid),
                m_hash(m_wots_params.hash_function_name()), m_key(std::move(key)),
                m_public_seed(std::move(public_seed)) {
            }

            /**
             * Creates a XMSS_WOTS_PublicKey for the signature method identified by
             * oid. The public seed will be initialized with a precomputed seed and
             * and precomputed key data which should be derived from a
             * XMSS_WOTS_PrivateKey.
             *
             * @param oid Identifier for the selected signature methods.
             * @param public_seed A precomputed public seed of n-bytes length.
             * @param key Precomputed raw key data of the XMSS_WOTS_PublicKey.
             **/
            XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid, const secure_vector<uint8_t> &public_seed,
                                const wots_keysig_t &key) :
                m_wots_params(oid),
                m_hash(m_wots_params.hash_function_name()), m_key(key), m_public_seed(public_seed) {
            }

            /**
             * Creates a XMSS_WOTS_PublicKey form a message and signature using
             * Algorithm 6 WOTS_pkFromSig defined in the XMSS standard. This
             * overload is used to verify a message using a public key.
             *
             * @param oid WOTSP algorithm identifier.
             * @param msg A message.
             * @param sig A WOTS signature for msg.
             * @param adrs An XMSS_Address.
             * @param public_seed The public public_seed.
             **/
            XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid, const secure_vector<uint8_t> &msg,
                                const wots_keysig_t &sig, XMSS_Address &adrs,
                                const secure_vector<uint8_t> &public_seed) :
                m_wots_params(oid),
                m_hash(m_wots_params.hash_function_name()), m_key(pub_key_from_signature(msg, sig, adrs, public_seed)),
                m_public_seed(public_seed) {
            }

            /**
             * Retrieves the i-th element out of the length len chain of
             * n-byte elements contained in the public key.
             *
             * @param i index of the element.
             * @returns n-byte element addressed by i.
             **/
            const secure_vector<uint8_t> &operator[](size_t i) const {
                return m_key[i];
            }

            secure_vector<uint8_t> &operator[](size_t i) {
                return m_key[i];
            }

            /**
             * Convert the key into the raw key data. The key becomes a length
             * len vector of n-byte elements.
             **/
            operator const wots_keysig_t &() const {
                return m_key;
            }

            /**
             * Convert the key into the raw key data. The key becomes a length
             * len vector of n-byte elements.
             **/
            operator wots_keysig_t &() {
                return m_key;
            }

            const secure_vector<uint8_t> &public_seed() const {
                return m_public_seed;
            }

            secure_vector<uint8_t> &public_seed() {
                return m_public_seed;
            }

            void set_public_seed(const secure_vector<uint8_t> &public_seed) {
                m_public_seed = public_seed;
            }

            void set_public_seed(secure_vector<uint8_t> &&public_seed) {
                m_public_seed = std::move(public_seed);
            }

            const wots_keysig_t &key_data() const {
                return m_key;
            }

            wots_keysig_t &key_data() {
                return m_key;
            }

            void set_key_data(const wots_keysig_t &key_data) {
                m_key = key_data;
            }

            void set_key_data(wots_keysig_t &&key_data) {
                m_key = std::move(key_data);
            }

            const XMSS_WOTS_Parameters &wots_parameters() const {
                return m_wots_params;
            }

            std::string algo_name() const override {
                return m_wots_params.name();
            }

            algorithm_identifier algorithm_identifier() const override {
                throw Not_Implemented("No algorithm_identifier available for XMSS-WOTS.");
            }

            bool check_key(RandomNumberGenerator &, bool) const override {
                return true;
            }

            size_t estimated_strength() const override {
                return m_wots_params.estimated_strength();
            }

            size_t key_length() const override {
                return m_wots_params.estimated_strength();
            }

            std::vector<uint8_t> public_key_bits() const override {
                throw Not_Implemented("No key format defined for XMSS-WOTS");
            }

            bool operator==(const XMSS_WOTS_PublicKey &key) {
                return m_key == key.m_key;
            }

            bool operator!=(const XMSS_WOTS_PublicKey &key) {
                return !(*this == key);
            }

        protected:
            /**
             * Algorithm 2: Chaining Function.
             *
             * Takes an n-byte input string and transforms it into a the function
             * result iterating the cryptographic hash function "F" steps times on
             * the input x using the outputs of the PRNG "G".
             *
             * This overload is used in multithreaded scenarios, where it is
             * required to provide seperate instances of XMSS_Hash to each
             * thread.
             *
             * @param[out] x An n-byte input string, that will be transformed into
             *               the chaining function result.
             * @param start_idx The start index.
             * @param steps A number of steps.
             * @param adrs An OTS Hash Address.
             * @param public_seed A public seed.
             * @param hash Instance of XMSS_Hash, that may only by the thead
             *        executing chain.
             **/
            void chain(secure_vector<uint8_t> &x, size_t start_idx, size_t steps, XMSS_Address &adrs,
                       const secure_vector<uint8_t> &public_seed, XMSS_Hash &hash);

            /**
             * Algorithm 2: Chaining Function.
             *
             * Takes an n-byte input string and transforms it into a the function
             * result iterating the cryptographic hash function "F" steps times on
             * the input x using the outputs of the PRNG "G".
             *
             * @param[out] x An n-byte input string, that will be transformed into
             *               the chaining function result.
             * @param start_idx The start index.
             * @param steps A number of steps.
             * @param adrs An OTS Hash Address.
             * @param public_seed A public seed.
             **/
            inline void chain(secure_vector<uint8_t> &x, size_t start_idx, size_t steps, XMSS_Address &adrs,
                              const secure_vector<uint8_t> &public_seed) {
                chain(x, start_idx, steps, adrs, public_seed, m_hash);
            }

            XMSS_WOTS_Parameters m_wots_params;
            XMSS_Hash m_hash;
            wots_keysig_t m_key;
            secure_vector<uint8_t> m_public_seed;

        private:
            /**
             * Algorithm 6: "WOTS_pkFromSig"
             * Computes a Winternitz One Time Signature+ public key from a message and
             * its signature.
             *
             * @param msg A message.
             * @param sig The signature for msg.
             * @param adrs An address.
             * @param public_seed A public_seed.
             *
             * @return Temporary WOTS+ public key.
             **/
            wots_keysig_t pub_key_from_signature(const secure_vector<uint8_t> &msg, const wots_keysig_t &sig,
                                                 XMSS_Address &adrs, const secure_vector<uint8_t> &public_seed);
        };

        void XMSS_WOTS_PublicKey::chain(secure_vector<uint8_t> &result, size_t start_idx, size_t steps,
                                        XMSS_Address &adrs, const secure_vector<uint8_t> &seed, XMSS_Hash &hash) {
            for (size_t i = start_idx; i < (start_idx + steps) && i < m_wots_params.wots_parameter(); i++) {
                adrs.set_hash_address(i);

                // Calculate tmp XOR bitmask
                adrs.set_key_mask_mode(XMSS_Address::Key_Mask::Mask_Mode);
                xor_buf(result, hash.prf(seed, adrs.bytes()), result.size());

                // Calculate key
                adrs.set_key_mask_mode(XMSS_Address::Key_Mask::Key_Mode);

                // Calculate f(key, tmp XOR bitmask)
                hash.f(result, hash.prf(seed, adrs.bytes()), result);
            }
        }

        wots_keysig_t XMSS_WOTS_PublicKey::pub_key_from_signature(const secure_vector<uint8_t> &msg,
                                                                  const wots_keysig_t &sig, XMSS_Address &adrs,
                                                                  const secure_vector<uint8_t> &seed) {
            secure_vector<uint8_t> msg_digest {m_wots_params.base_w(msg, m_wots_params.len_1())};

            m_wots_params.append_checksum(msg_digest);
            wots_keysig_t result(sig);

            for (size_t i = 0; i < m_wots_params.len(); i++) {
                adrs.set_chain_address(i);
                chain(result[i], msg_digest[i], m_wots_params.wots_parameter() - 1 - msg_digest[i], adrs, seed);
            }
            return result;
        }
    }    // namespace crypto3
}    // namespace nil

#endif
