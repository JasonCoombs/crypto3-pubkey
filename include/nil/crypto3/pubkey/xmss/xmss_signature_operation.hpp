//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_PUBKEY_XMSS_SIGNATURE_OPERATION_HPP
#define CRYPTO3_PUBKEY_XMSS_SIGNATURE_OPERATION_HPP

#include <cstddef>
#include <string>

#include <nil/crypto3/pubkey/pk_operations.hpp>

#include <nil/crypto3/pubkey/xmss/xmss_parameters.hpp>
#include <nil/crypto3/pubkey/xmss/xmss_privatekey.hpp>
#include <nil/crypto3/pubkey/xmss/xmss_address.hpp>
#include <nil/crypto3/pubkey/xmss/xmss_common_ops.hpp>
#include <nil/crypto3/pubkey/xmss/xmss_signature.hpp>
#include <nil/crypto3/pubkey/xmss/xmss_wots_publickey.hpp>

namespace nil {
    namespace crypto3 {

        /**
         * Signature generation operation for Extended Hash-Based Signatures (XMSS) as
         * defined in:
         *
         * [1] XMSS: Extended Hash-Based Signatures,
         *     draft-itrf-cfrg-xmss-hash-based-signatures-06
         *     Release: July 2016.
         *     https://datatracker.ietf.org/doc/
         *     draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
         **/
        class XMSS_Signature_Operation final : public virtual pk_operations::signature, public XMSS_Common_Ops {
        public:
            XMSS_Signature_Operation(const XMSS_PrivateKey &private_key);

            /**
             * Creates an XMSS signature for the message provided through call to
             * update().
             *
             * @return serialized XMSS signature.
             **/
            secure_vector<uint8_t> sign(RandomNumberGenerator &) override;

            void update(const uint8_t msg[], size_t msg_len) override;

        private:
            /**
             * Algorithm 11: "treeSig"
             * Generate a WOTS+ signature on a message with corresponding auth path.
             *
             * @param msg A message.
             * @param xmss_priv_key A XMSS private key.
             * @param adrs A XMSS Address.
             **/
            XMSS_WOTS_PublicKey::TreeSignature generate_tree_signature(const secure_vector<uint8_t> &msg,
                                                                       XMSS_PrivateKey &xmss_priv_key,
                                                                       XMSS_Address &adrs);

            /**
             * Algorithm 12: "XMSS_sign"
             * Generate an XMSS signature and update the XMSS secret key
             *
             * @param msg A message to sign of arbitrary length.
             * @param [out] xmss_priv_key A XMSS private key. The private key will be
             *              updated during the signing process.
             *
             * @return The signature of msg signed using xmss_priv_key.
             **/
            XMSS_Signature sign(const secure_vector<uint8_t> &msg, XMSS_PrivateKey &xmss_priv_key);

            wots_keysig_t build_auth_path(XMSS_PrivateKey &priv_key, XMSS_Address &adrs);

            void initialize();

            XMSS_PrivateKey m_priv_key;
            secure_vector<uint8_t> m_randomness;
            size_t m_leaf_idx;
            bool m_is_initialized;
        };

        XMSS_Signature_Operation::XMSS_Signature_Operation(const XMSS_PrivateKey &private_key) :
            XMSS_Common_Ops(private_key.xmss_oid()), m_priv_key(private_key), m_randomness(0), m_leaf_idx(0),
            m_is_initialized(false) {
        }

        XMSS_WOTS_PublicKey::TreeSignature
            XMSS_Signature_Operation::generate_tree_signature(const secure_vector<uint8_t> &msg,
                                                              XMSS_PrivateKey &xmss_priv_key,
                                                              XMSS_Address &adrs) {

            wots_keysig_t auth_path = build_auth_path(xmss_priv_key, adrs);
            adrs.set_type(XMSS_Address::Type::OTS_Hash_Address);
            adrs.set_ots_address(m_leaf_idx);

            wots_keysig_t sig_ots = xmss_priv_key.wots_private_key().sign(msg, adrs);
            return XMSS_WOTS_PublicKey::TreeSignature(sig_ots, auth_path);
        }

        XMSS_Signature XMSS_Signature_Operation::sign(const secure_vector<uint8_t> &msg_hash,
                                                      XMSS_PrivateKey &xmss_priv_key) {
            XMSS_Address adrs;
            XMSS_Signature sig(m_leaf_idx, m_randomness, generate_tree_signature(msg_hash, xmss_priv_key, adrs));
            return sig;
        }

        wots_keysig_t XMSS_Signature_Operation::build_auth_path(XMSS_PrivateKey &priv_key, XMSS_Address &adrs) {
            wots_keysig_t auth_path(m_xmss_params.tree_height());
            adrs.set_type(XMSS_Address::Type::Hash_Tree_Address);

            for (size_t j = 0; j < m_xmss_params.tree_height(); j++) {
                size_t k = (m_leaf_idx / (1 << j)) ^ 0x01;
                auth_path[j] = priv_key.tree_hash(k * (1 << j), j, adrs);
            }

            return auth_path;
        }

        void XMSS_Signature_Operation::update(const uint8_t msg[], size_t msg_len) {
            initialize();
            m_hash.h_msg_update(msg, msg_len);
        }

        secure_vector<uint8_t> XMSS_Signature_Operation::sign(RandomNumberGenerator &) {
            initialize();
            secure_vector<uint8_t> signature(sign(m_hash.h_msg_final(), m_priv_key).bytes());
            m_is_initialized = false;
            return signature;
        }

        void XMSS_Signature_Operation::initialize() {
            // return if we already initialized and reserved a leaf index for signing.
            if (m_is_initialized) {
                return;
            }

            secure_vector<uint8_t> index_bytes;
            // reserve leaf index so it can not be reused in by another signature
            // operation using the same private key.
            m_leaf_idx = m_priv_key.reserve_unused_leaf_index();

            // write prefix for message hashing into buffer.
            XMSS_Tools::concat(index_bytes, m_leaf_idx, 32);
            m_randomness = m_hash.prf(m_priv_key.prf(), index_bytes);
            index_bytes.clear();
            XMSS_Tools::concat(index_bytes, m_leaf_idx, m_priv_key.xmss_parameters().element_size());
            m_hash.h_msg_init(m_randomness, m_priv_key.root(), index_bytes);
            m_is_initialized = true;
        }
    }    // namespace crypto3
}    // namespace nil

#endif
