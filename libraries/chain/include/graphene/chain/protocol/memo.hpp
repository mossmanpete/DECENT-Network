/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

   /**
    *  @brief defines the keys used to derive the shared secret
    *
    *  Because account authorities and keys can change at any time, each memo must
    *  capture the specific keys used to derive the shared secret.  In order to read
    *  the cipher message you will need one of the two private keys.
    *
    *  If @ref from == @ref to and @ref from == 0 then no encryption is used, the memo is public.
    *  If @ref from == @ref to and @ref from != 0 then invalid memo data
    *
    */
   struct memo_data
   {
      typedef std::vector<std::string::value_type> message_type;

      public_key_type from;
      public_key_type to;
      /**
       * 64 bit nonce format:
       * [  8 bits | 56 bits   ]
       * [ entropy | timestamp ]
       * Timestamp is number of microseconds since the epoch
       * Entropy is a byte taken from the hash of a new private key
       *
       * This format is not mandated or verified; it is chosen to ensure uniqueness of key-IV pairs only. This should
       * be unique with high probability as long as the generating host has a high-resolution clock OR a strong source
       * of entropy for generating private keys.
       */
      uint64_t nonce = 0;
      /**
       * This field contains the AES encrypted packed @ref memo_message
       */
      message_type message;

      memo_data() = default;

      /**
       * @brief Construct encrypted message
       * @param msg the message to encrypt
       * @param priv the private key of sender
       * @param pub the public key of receiver
       * @param nonce the salt number to use for message encryption (will be generated if zero)
       */
      memo_data(const std::string& msg, const private_key_type& priv, const public_key_type& pub, uint64_t nonce = 0);

      /**
       * @brief Decrypt message
       * @param priv the private key of sender/receiver
       * @param pub the public key of receiver/sender
       * @return decrypted message
       */
      std::string get_message(const private_key_type& priv, const public_key_type& pub) const;

      /**
       * @brief Encrypt message
       * @param message the message to encrypt
       * @param priv the private key of sender
       * @param pub the public key of receiver
       * @param nonce the salt number to use for message encryption
       * @return encrypted message
       */
      static message_type encrypt_message(const std::string &message,
                                          const private_key_type &priv,
                                          const public_key_type &pub,
                                          uint64_t nonce);

      /**
       * @brief Decrypt message
       * @param message the message to decrypt
       * @param priv the private key of sender/receiver
       * @param pub the public key of receiver/sender
       * @param nonce the salt number used for message encryption
       * @return decrypted message
       */
      static std::string decrypt_message(const message_type &message,
                                          const private_key_type &priv,
                                          const public_key_type &pub,
                                          uint64_t nonce);

      /**
       * @brief Generate salt number for encryption
       * @return salt number
       */
      static uint64_t generate_nonce();
   };

   /**
    * @brief defines a message and checksum to enable validation of successful decryption
    *
    * When encrypting/decrypting a checksum is required to determine whether or not
    * decryption was successful.
    */
   struct memo_message
   {
      memo_message(){}
      memo_message( uint32_t checksum, const std::string& text )
      :checksum(checksum),text(text){}

      uint32_t    checksum = 0;
      std::string text;

      string serialize() const;
      static memo_message deserialize(const string& serial);
   };

} } // namespace graphene::chain

FC_REFLECT( graphene::chain::memo_message, (checksum)(text) )
FC_REFLECT( graphene::chain::memo_data, (from)(to)(nonce)(message) )
