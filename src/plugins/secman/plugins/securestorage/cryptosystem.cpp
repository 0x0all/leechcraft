/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cryptosystem.h"
#include <memory>
#include <QByteArray>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <util/sll/util.h>
#include "ciphertextformat.h"

namespace LC
{
namespace SecMan
{
namespace SecureStorage
{
	const char* WrongHMACException::what () const throw ()
	{
		return "WrongHMACException";
	}

	CryptoSystem::CryptoSystem (const QString& password)
	{
		Key_ = CreateKey (password);
	}

	CryptoSystem::~CryptoSystem ()
	{
		Key_.fill (0, Key_.length ());
	}

	namespace
	{
		void Check (int retCode)
		{
			if (retCode)
				return;

			const char *file = nullptr;
			int line = 0;
			const char *data = nullptr;
			int flags = ERR_TXT_STRING;
			ERR_get_error_line_data (&file, &line, &data, &flags);
			throw std::runtime_error { QString { "encryption failed: %1 at %2:%3 (%4)" }
					.arg (data)
					.arg (file)
					.arg (line)
					.arg (retCode)
					.toStdString () };
		}

		template<typename T, typename D>
		auto mkUnique(T *pointee, D dtor)
		{
			return std::unique_ptr<T, D> { pointee, dtor };
		}

		auto mkHmacCtx ()
		{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			return mkUnique (HMAC_CTX_new (), &HMAC_CTX_free);
#else
			auto ptr = new HMAC_CTX ();
			HMAC_CTX_init (ptr);
			return mkUnique (ptr,
					[] (auto ptr)
					{
						HMAC_CTX_cleanup (ptr);
						delete ptr;
					});
#endif
		}
	}

	QByteArray CryptoSystem::Encrypt (const QByteArray& data) const
	{
		QByteArray result;
		result.resize (CipherTextFormatUtils::BufferLengthFor (data.length ()));
		CipherTextFormat cipherText (result.data (), data.length ());

		// fill IV in cipherText & random block
		Check (RAND_bytes (cipherText.Iv (), IVLength));
		unsigned char randomData [RndLength];
		Check (RAND_bytes (randomData, RndLength));

		// init cipher
		auto cipherCtx = mkUnique (EVP_CIPHER_CTX_new (), &EVP_CIPHER_CTX_free);
		EVP_CIPHER_CTX_init (cipherCtx.get ());
		Check (EVP_EncryptInit (cipherCtx.get (), EVP_aes_256_ofb (),
				reinterpret_cast<const unsigned char*> (Key_.data ()),
				cipherText.Iv ()));

		// encrypt
		int outLength = 0;
		unsigned char* outPtr = cipherText.Data ();
		Check (EVP_EncryptUpdate (cipherCtx.get (), outPtr, &outLength,
				reinterpret_cast<const unsigned char*> (data.data ()),
				data.length ()));
		outPtr += outLength;
		Check (EVP_EncryptUpdate (cipherCtx.get (), outPtr, &outLength, randomData, RndLength));
		outPtr += outLength;
		// output last block & cleanup
		Check (EVP_EncryptFinal (cipherCtx.get (), outPtr, &outLength));

		// compute hmac
		{
			auto hmacCtx = mkHmacCtx ();
			Check (HMAC_Init_ex (hmacCtx.get (), Key_.data (), Key_.length (), EVP_sha256 (), 0));
			Check (HMAC_Update (hmacCtx.get (), reinterpret_cast<const unsigned char*> (data.data ()), data.length ()));
			Check (HMAC_Update (hmacCtx.get (), randomData, RndLength));
			unsigned hmacLength = 0;
			Check (HMAC_Final (hmacCtx.get (), cipherText.Hmac (), &hmacLength));
		}

		return result;
	}

	QByteArray CryptoSystem::Decrypt (const QByteArray& cipherText) const
	{
		if (CipherTextFormatUtils::DataLengthFor (cipherText.length ()) < 0)
			throw WrongHMACException ();

		QByteArray data;
		data.resize (CipherTextFormatUtils::DecryptBufferLengthFor (cipherText.length ()));
		CipherTextFormat cipherTextFormat (const_cast<char*> (cipherText.data ()),
				CipherTextFormatUtils::DataLengthFor (cipherText.length ()));

		// init cipher
		auto cipherCtx = mkUnique (EVP_CIPHER_CTX_new (), &EVP_CIPHER_CTX_free);
		EVP_CIPHER_CTX_init (cipherCtx.get ());
		EVP_DecryptInit (cipherCtx.get (), EVP_aes_256_ofb (),
				reinterpret_cast<const unsigned char*> (Key_.data ()),
				cipherTextFormat.Iv ());

		// decrypt
		int outLength = 0;
		unsigned char* outPtr = reinterpret_cast<unsigned char*> (data.data ());
		EVP_DecryptUpdate (cipherCtx.get (), outPtr, &outLength,
				cipherTextFormat.Data (), cipherTextFormat.GetDataLength ());
		outPtr += outLength;
		EVP_DecryptUpdate (cipherCtx.get (), outPtr, &outLength,
				cipherTextFormat.Rnd (), RndLength);
		outPtr += outLength;
		// output last block & cleanup
		EVP_DecryptFinal (cipherCtx.get (), outPtr, &outLength);

		// compute hmac
		unsigned char hmac [HMACLength];
		{
			auto hmacCtx = mkHmacCtx ();
			HMAC_Init_ex (hmacCtx.get (), Key_.data (), Key_.length (),
					EVP_sha256 (), 0);
			HMAC_Update (hmacCtx.get (), reinterpret_cast<unsigned char*> (data.data ()), data.length ());
			unsigned int hmacLength = 0;
			HMAC_Final (hmacCtx.get (), hmac, &hmacLength);
		}

		// check hmac
		const bool hmacsDifferent = memcmp (hmac, cipherTextFormat.Hmac (), HMACLength);
		if (hmacsDifferent)
			throw WrongHMACException ();

		// remove random block
		data.truncate (cipherTextFormat.GetDataLength ());

		return data;
	}

	QByteArray CryptoSystem::Hash (const QByteArray& data) const
	{
		unsigned char hash [HashLength];
		SHA256 (reinterpret_cast<const unsigned char*> (data.data ()), data.size (), hash);
		return QByteArray (reinterpret_cast<char*> (hash), HashLength);
	}

	QByteArray CryptoSystem::CreateKey (const QString& password) const
	{
		QByteArray res = Hash (password.toUtf8 ());
		res.resize (KeyLength);
		return res;
	}
}
}
}
