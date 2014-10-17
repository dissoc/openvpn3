//    OpenVPN -- An application to securely tunnel IP networks
//               over a single port, with support for SSL/TLS-based
//               session authentication and key exchange,
//               packet encryption, packet authentication, and
//               packet compression.
//
//    Copyright (C) 2013-2014 OpenVPN Technologies, Inc.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License Version 3
//    as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program in the COPYING file.
//    If not, see <http://www.gnu.org/licenses/>.

// OpenVPN CBC/HMAC data channel

#ifndef OPENVPN_CRYPTO_CRYPTO_CHM_H
#define OPENVPN_CRYPTO_CRYPTO_CHM_H

#include <openvpn/crypto/encrypt.hpp>
#include <openvpn/crypto/decrypt.hpp>
#include <openvpn/crypto/cryptodc.hpp>

namespace openvpn {

  template <typename CRYPTO_API>
  class CryptoCHM : public CryptoDCBase<CRYPTO_API>
  {
  public:
    typedef CryptoDCBase<CRYPTO_API> Base;

    CryptoCHM(const typename CRYPTO_API::Cipher& cipher_arg,
	      const typename CRYPTO_API::Digest& digest_arg,
	      const Frame::Ptr& frame_arg,
	      const typename PRNG<CRYPTO_API>::Ptr& prng_arg)
      : cipher(cipher_arg),
	digest(digest_arg),
	frame(frame_arg),
	prng(prng_arg)
    {
      encrypt_.frame = frame;
      decrypt_.frame = frame;
      encrypt_.prng = prng;
    }

    virtual void init_encrypt_cipher(const StaticKey& key, const int mode)
    {
      encrypt_.cipher.init(cipher, key, mode);
    }

    virtual void init_encrypt_hmac(const StaticKey& key)
    {
      encrypt_.hmac.init(digest, key);
    }

    virtual void init_encrypt_pid_send(const int form)
    {
      encrypt_.pid_send.init(form);
    }

    virtual void init_decrypt_cipher(const StaticKey& key, const int mode)
    {
      decrypt_.cipher.init(cipher, key, mode);
    }

    virtual void init_decrypt_hmac(const StaticKey& key)
    {
      decrypt_.hmac.init(digest, key);
    }

    virtual void init_decrypt_pid_recv(const int mode, const int form,
				       const int seq_backtrack, const int time_backtrack,
				       const char *name, const int unit,
				       const SessionStats::Ptr& stats_arg)
    {
      decrypt_.pid_recv.init(mode, form, seq_backtrack, time_backtrack, name, unit, stats_arg);
    }

    /* returns true if packet ID is close to wrapping */
    virtual bool encrypt(BufferAllocated& buf, const PacketID::time_t now)
    {
      encrypt_.encrypt(buf, now);
      return encrypt_.pid_send.wrap_warning();
    }

    virtual Error::Type decrypt(BufferAllocated& buf, const PacketID::time_t now)
    {
      return decrypt_.decrypt(buf, now);
    }

    virtual void rekey(const typename Base::RekeyType type)
    {
    }

  private:
    typename CRYPTO_API::Cipher cipher;
    typename CRYPTO_API::Digest digest;
    Frame::Ptr frame;
    const typename PRNG<CRYPTO_API>::Ptr prng;

    Encrypt<CRYPTO_API> encrypt_;
    Decrypt<CRYPTO_API> decrypt_;
  };

  template <typename CRYPTO_API>
  class CryptoContextCHM : public CryptoDCContext<CRYPTO_API>
  {
  public:
    typedef boost::intrusive_ptr<CryptoContextCHM> Ptr;

    CryptoContextCHM(const typename CRYPTO_API::Cipher& cipher_arg,
		     const typename CRYPTO_API::Digest& digest_arg,
		     const Frame::Ptr& frame_arg,
		     const typename PRNG<CRYPTO_API>::Ptr& prng_arg)
      : cipher(cipher_arg),
	digest(digest_arg),
	frame(frame_arg),
	prng(prng_arg)
    {
    }

    virtual typename CryptoDCBase<CRYPTO_API>::Ptr new_obj(const unsigned int key_id)
    {
      return new CryptoCHM<CRYPTO_API>(cipher, digest, frame, prng);
    }

  private:
    typename CRYPTO_API::Cipher cipher;
    typename CRYPTO_API::Digest digest;
    Frame::Ptr frame;
    const typename PRNG<CRYPTO_API>::Ptr prng;
  };
}

#endif
