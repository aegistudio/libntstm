#pragma once

/**
 * @file ntstm/io.hpp
 * @author Haoran Luo
 * @brief Input/Output Interfaces.
 */

/**
 * @brief Predefined error codes that would be thrown by the LibNtStm.
 *
 * The user could add his own I/O error code after NtIoErrorCode::eioMax
 * when he finds some logical I/O error (not percepted by the library).
 */
enum NtIoErrorCode {
	/// The handle has no mapped resource in kernel space. The EBADF, 
	/// EINVAL and EISDIR will cause such error.
	eioInvalidHandle = 0,
	
	/// The handle has no permission of reading or writing. Opening file 
	/// with improper permission or the EPERM will cause such error.
	eioPermission,
	
	/// The stream is closed while reading or writing. Reading returns 0,
	/// buffer limit reached, and the EIO and EPIPE will cause such error.
	eioStreamClosed,
	
	/// The stream is non-blocking (which is not supported by the stream
	/// representation). The EAGAIN or EWOULDBLOCK will cause such error.
	eioNonBlocking,
	
	/// Signal interrupts reading or writing process. Meaning that proper 
	/// signal handler may not be installed. The EINTR will cause such error.
	eioInterrupted,
	
	/// No more buffer space could be allocated. Container allocation 
	/// and EFAULT will cause such error.
	eioAllocation,
	
	/// The input data is malformed while reading. This error is raised 
	/// by userspace code and has no correspond error code.
	eioMalformed,
	
	/// The max predefined I/O error codes in LibNtStm.
	/// The user can add his own error code after this code, however neither 
	/// user nor the library should throw this error.
	eioMax,
};

/**
 * @brief Exception thrown when encountering I/O error.
 *
 * Exception that is thrown when underlying handle or buffer has encountered
 * read/write error, or the underlying data stream has malformed.
 *
 * For efficiency consideration, the exception does not hold a precise 
 * description of the error, but instead an error code.
 *
 * No subclassing of this exception object.
 */
struct NtIoException final {
	/// The error code corresponds to the I/O error.
	unsigned int ioErrorCode;
};

/**
 * @brief Abstraction for stream inputing.
 */
class NtIoInputStream {
public:
	/// So that the destructor of the input stream subclasses will be invoked.
	virtual ~NtIoInputStream() {}
	
	// Read data FULLY (which is different from the conventional read()
	// defined in the POSIX API) from the stream.
	virtual void read(void* buffer, size_t size) throw (NtIoException) = 0;
	
	/// Wrapped read fully method. This method is applicable to primitives and value 
	/// objects. Complicated or variant objects should be represented as NtIoSerializable
	/// and use NtIoSerializable::read for reading.
	template<typename T>
	inline void read(T& t) throw (NtIoException) { read(&t, sizeof(T)); }
};

/**
 * @brief Abstraction for stream outputing.
 */
class NtIoOutputStream {
public:
	/// So that the destructor of the output stream subclasses will be invoked.
	virtual ~NtIoOutputStream() {}

	/// Write data FULLY to the stream.
	virtual void write(const void* buffer, size_t size) throw (NtIoException) = 0;
	
	/// Wrapped write fully method. Just analogous to the read() counter part.
	template<typename T>
	inline void write(const T& t) throw (NtIoException) { write(&t, sizeof(T)); }
};

/**
 * @brief Abstraction for serializable object.
 *
 * Serializable objects defines method that could deflate them to the output stream and
 * inflate them from the input stream.
 */
class NtIoSerializable {
public:
	/// So that the destructor of the serializable subclasses will be invoked.
	virtual ~NtIoSerializable() {}

	/// Inflate internal state from the input stream. The prior internal state should 
	/// be discarded and internal memory allocation should be deallocated (even if 
	/// an NtIoException is thrown).
	virtual void read(NtIoInputStream& inputStream) throw (NtIoException) = 0;
	
	/// Deflate internal state to the output stream. The internal state remains unchanged.
	virtual void write(NtIoOutputStream& outputStream) const throw (NtIoException) = 0;
};