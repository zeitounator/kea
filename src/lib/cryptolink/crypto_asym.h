// Copyright (C) 2014, 2015  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <util/buffer.h>

#include <boost/noncopyable.hpp>

#include <cryptolink/cryptolink.h>

#ifndef ISC_CRYPTO_ASYM_H
#define ISC_CRYPTO_ASYM_H

namespace isc {
namespace cryptolink {

/// Forward declaration, pimpl style
class AsymImpl;

/// \brief Asymmetrical cryptography (Asym) support
///
/// This class is used to create and verify Asym signatures. Instances
/// can be created with CryptoLink::createAsym()
///
class Asym : private boost::noncopyable {
private:
    /// \brief Constructor from a key, asym and hash algorithm,
    ///        key kind and key binary format
    ///
    /// \exception UnsupportedAlgorithm if the given algorithm
    ///            is unknown or not supported by the underlying library
    /// \exception Badkey if the given key length is bad
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// \param key            The key to sign/verify with
    /// \param len            The length of the key
    /// \param asym_algorithm The asymmetrical cryptography algorithm
    /// \param hash_algorithm The hash algorithm
    /// \param key_kind       The key kind
    /// \param key_format     The key binary format
    Asym(const void* key, size_t key_len,
         const AsymAlgorithm asym_algorithm,
         const HashAlgorithm hash_algorithm,
         const AsymKeyKind key_kind,
         const AsymFormat key_format);
    ///
    /// \param key            The key to sign/verify with
    /// \param asym_algorithm The asymmetrical cryptography algorithm
    /// \param hash_algorithm The hash algorithm
    /// \param key_kind       The key kind
    /// \param key_format     The key binary format
    Asym(const std::vector<uint8_t> key,
         const AsymAlgorithm asym_algorithm,
         const HashAlgorithm hash_algorithm,
         const AsymKeyKind key_kind,
         const AsymFormat key_format);
    ///
    /// \param filename       The key file name/path
    /// \param password       The PKCS#8 password
    /// \param asym_algorithm The asymmetrical cryptography algorithm
    /// \param hash_algorithm The hash algorithm
    /// \param key_kind       The key kind
    /// \param file_format    The key file format
    Asym(const std::string& filename, const std::string& password,
         const AsymAlgorithm asym_algorithm,
         const HashAlgorithm hash_algorithm,
         const AsymKeyKind key_kind,
         const AsymFormat file_format);

    friend Asym* CryptoLink::createAsym(const void*, size_t,
                                        const AsymAlgorithm,
                                        const HashAlgorithm,
                                        const AsymKeyKind,
                                        const AsymFormat);

    friend Asym* CryptoLink::createAsym(const std::vector<uint8_t>,
                                        const AsymAlgorithm,
                                        const HashAlgorithm,
                                        const AsymKeyKind,
                                        const AsymFormat);

    friend Asym* CryptoLink::createAsym(const std::string&,
                                        const std::string&,
                                        const AsymAlgorithm,
                                        const HashAlgorithm,
                                        const AsymKeyKind,
                                        const AsymFormat);

public:
    /// \brief Destructor
    ~Asym();

    /// \brief Returns the AsymAlgorithm of the object
    AsymAlgorithm getAsymAlgorithm() const;

    /// \brief Returns the HashAlgorithm of the object
    HashAlgorithm getHashAlgorithm() const;

    /// \brief Returns the AsymKeyKind of the object
    AsymKeyKind getAsymKeyKind() const;

    /// \brief Returns the key size in bits
    ///
    size_t getKeySize() const;

    /// \brief Returns the output size of the signature
    ///
    /// \param sig_format The signature binary format
    size_t getSignatureLength(const AsymFormat sig_format) const;

    /// \brief Add data to digest
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// \param data The data to add
    /// \param len  The size of the data
    void update(const void* data, const size_t len);

    /// \brief Calculate the final signature
    ///
    /// The result will be appended to the given outputbuffer
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// \param result     The OutputBuffer to append the result to
    /// \param len        The number of bytes from the result to copy. If this
    ///                   value is smaller than the algorithms output size, the
    ///                   result will be truncated. If this value is larger,
    ///                   only output size bytes will be copied
    /// \param sig_format The signature binary format
    void sign(isc::util::OutputBuffer& result, size_t len,
              const AsymFormat sig_format);

    /// \brief Calculate the final signature
    ///
    /// len bytes of data from the result will be copied to *result
    /// If len is larger than the output size, only output_size bytes
    /// will be copied. If it is smaller, the output will be truncated
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// At least len bytes of data must be available for writing at
    /// result
    void sign(void* result, size_t len, const AsymFormat sig_format);

    /// \brief Calculate the final signatre
    ///
    /// The result will be returned as a std::vector<uint8_t>
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// \param len        The number of bytes from the result to copy. If this
    ///                   value is smaller than the algorithms output size, the
    ///                   result will be truncated. If this value is larger,
    ///                   only output size bytes will be copied
    /// \param sig_format The signature binary format
    /// \return           a vector containing the signature
    std::vector<uint8_t> sign(size_t len, const AsymFormat sig_format);

    /// \brief Verify an existing signature
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// \param sig        The signature to verify
    /// \param len        The length of the signature. If this is smaller
    ///                   than the output length of the algorithm,
    ///                   only len bytes will be checked
    /// \param sig_format The signature binary format
    /// \return           true if the signature is correct, false otherwise
    ///
    /// \note verify() does not destroy its context so it can be
    /// called multiple times with different signatures.
    bool verify(const void* sig, size_t len, const AsymFormat sig_format);

    /// \brief Clear the crypto state and go back to the initial state
    /// (must be called before reusing an Asym object)
    void clear();

    /// \brief Export the key value
    ///
    /// The result will be returned as a std::vector<uint8_t>
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// \param key_kind       The key kind
    /// \param key_format     The key binary format
    std::vector<uint8_t> exportkey(const AsymKeyKind key_kind,
                                   const AsymFormat key_format) const;

    /// \brief Export the key value
    ///
    /// The key value will be returned into the given file,
    /// optionally when PKCS#8 is selected encrypted by the password
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// \param filename       The key file name/path
    /// \param password       The PKCS#8 password
    /// \param key_kind       The key kind    
    /// \param file_format    The key file format
    void exportkey(const std::string& filename,
                   const std::string& password,
                   const AsymKeyKind key_kind,
                   const AsymFormat file_format) const;

    /// \brief Check the validity
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    bool validate() const;
    
    /// \brief Compare two keys
    ///
    /// The key kind parameter allows to select what to compare.
    ///
    /// \exception LibraryError if there was any unexpected exception
    ///            in the underlying library
    ///
    /// \param key_kind       The key kind
    bool compare(const Asym* other, const AsymKeyKind key_kind) const;

private:
    AsymImpl* impl_;
};

/// \brief Delete an Asym object
void deleteAsym(Asym* asym);

} // namespace cryptolink
} // namespace isc
   
#endif // ISC_CRYPTO_ASYM_H