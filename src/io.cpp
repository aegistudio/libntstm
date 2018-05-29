/**
 * @file io.cpp
 * @author Haoran Luo
 * @brief Implementation for I/O classes.
 *
 * See the I/O headers for specification details.
 */

// The library headers.
#include "ntstm/io.hpp"
#include "ntstm/fdio.hpp"
#include "ntstm/bufio.hpp"

// The C standard headers.
#include <cstdint>
#include <cerrno>
#include <cstring>

// Where the read() and write() function is declared.
#ifdef _WIN32
// There's no UNISTD.H under windows, but the IO.H includes the declaration of read() 
// and write() functions. The ssize_t should also be redeclared here.
#include <io.h>
#define ssize_t int
#else
// Other platforms should have UNISTD.H declared.
#include <unistd.h>
#endif

// Constructor of the file stream.
NtIoFileStream::NtIoFileStream(int fd): fd(fd) {}

// Destructor of the file stream.
NtIoFileStream::~NtIoFileStream() {}

// Perform error checking for read and write function in file stream.
static void NtIoFileStreamErrnoCheck() throw (NtIoException) {
	switch(errno) {
		// The file is not permitted to read.
		case EPERM:
			throw NtIoException { NtIoErrorCode::eioPermission };
		break;

		// The file has closed.
		case EIO:
		case EPIPE:
			throw NtIoException { NtIoErrorCode::eioStreamClosed };
		break;

		// The file is in non-blocking mode (Which is not supported).
		case EAGAIN:
		case EWOULDBLOCK:
			throw NtIoException { NtIoErrorCode::eioNonBlocking };
		break;

		// The interrupt has occured.
		case EINTR:
			throw NtIoException { NtIoErrorCode::eioInterrupted };
		break;

		// The cases for invalid handles is regarded as default case.
		case EBADF:
		case EINVAL:
		case EISDIR:
		default:
			throw NtIoException { NtIoErrorCode::eioInvalidHandle };
		break;
	}
}

// Reading method of the file stream.
void NtIoFileStream::read(void* buffer, size_t size) throw (NtIoException) {
	int8_t* byteBuffer = (int8_t*)buffer;
	size_t readSize = 0;

	// Read until all data is retrieved or an error occurs.
	while(readSize < size) {
		// Attempt to invoke the read() function first.
		ssize_t newSize = ::read(fd, &byteBuffer[readSize], size - readSize);

		// The normal status of reading a stream.
		// Just forward the read size to next location.
		if(newSize > 0) readSize += ((size_t)newSize);
			
		// Meaning that the end of stream reaches.
		else if(newSize == 0) throw NtIoException { NtIoErrorCode::eioStreamClosed };

		// Meaning that an error occurs with error code set. 
		// Suitable exception should be intepreted from the errno and thrown in this case.
		else NtIoFileStreamErrnoCheck();
	}
}

// Writing method of the file stream.
void NtIoFileStream::write(const void* buffer, size_t size) throw (NtIoException) {
	const int8_t* byteBuffer = (const int8_t*)buffer;
	size_t writeSize = 0;

	// Write until all data is written or an error occurs.
	while(writeSize < size) {
		// Attempt to invoke the read() function first.
		ssize_t newSize = ::write(fd, &byteBuffer[writeSize], size - writeSize);

		// The normal status of writing a stream.
		// Just forward the write size to next location.
		if(newSize > 0) writeSize += ((size_t)newSize);

		// Meaning that an error occurs with error code set.
		// Suitable exception should be intepreted from the errno and thrown in this case.
		else NtIoFileStreamErrnoCheck();
	}
}

// Constructor of the input buffer.
NtIoInputBuffer::NtIoInputBuffer(const void* buffer, size_t size): buffer(buffer), size(size) {}

// Destructor of the input buffer.
NtIoInputBuffer::~NtIoInputBuffer() {}

// Reading method of the input buffer.
void NtIoInputBuffer::read(void* requestBuffer, size_t requestSize) throw (NtIoException) {
	// The data requested has exceeds the remained size.
	if(requestSize > size) throw NtIoException { NtIoErrorCode::eioStreamClosed };

	// Just copy the data to the request buffer.
	memcpy(requestBuffer, buffer, requestSize);

	// Forward the buffer pointer and reduce the remained size.
	buffer = &(((int8_t*)buffer)[requestSize]);
	size -= requestSize;
}

// Constructor of the output buffer.
NtIoOutputBuffer::NtIoOutputBuffer(size_t memoryStepping): memoryStepping(memoryStepping) {}

// Destructor of the output buffer.
NtIoOutputBuffer::~NtIoOutputBuffer() {}

// Writing method of the output buffer.
void NtIoOutputBuffer::write(const void* requestBuffer, size_t requestSize) throw (NtIoException) {
	if(requestSize == 0) return;

	// Make sure there's enough memory to hold newly come data.
	size_t oldSize = buffer.size();
	size_t newSize = oldSize + requestSize;
	size_t maskStepSize = stepSize() - 1;
	size_t newContainerSize = ((newSize + maskStepSize) | maskStepSize) ^ (maskStepSize);
	if(buffer.capacity() < newContainerSize) {
		// Make sure the memory space could be allocated and reserved.
		try {
			buffer.reserve(newContainerSize);
		}
		catch(std::length_error le) {
			throw NtIoException { NtIoErrorCode::eioAllocation };
		}
		catch(std::bad_alloc bal) {
			throw NtIoException { NtIoErrorCode::eioAllocation };
		}
	}

	// Copy new data to the tail of the buffer.
	buffer.resize(newSize);
	memcpy(&buffer[oldSize], requestBuffer, requestSize);
}