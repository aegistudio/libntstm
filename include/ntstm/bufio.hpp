#pragma once

/**
 * @file ntstm/bufio.hpp
 * @author Haoran Luo
 * @brief Buffer's Input/Output classes.
 *
 * The buffer means a piece of memory region that is readable or writable.
 *
 * The implementation for readable and writable memory buffer is separated 
 * regarding the memory allocation requirement.
 */

#include "ntstm/io.hpp"
#include <vector>
#include <cstdint>

/**
 * @brief The input buffer's implementation.
 *
 * The buffer content that could be read should never changes. The class is
 * not thread safe and should never be shared among threads.
 */
class NtIoInputBuffer : public NtIoInputStream {
	/// Indicating the remained size of data in the buffer.
	size_t size;

	/// Pointer to the current input buffer.
	const void* buffer;
public:
	/// Constructor of the input buffer.
	NtIoInputBuffer(const void* buffer, size_t size);

	/// Destructor of the input buffer.
	virtual ~NtIoInputBuffer();

	/// Implementation of the read method.
	virtual void read(void*, size_t) throw (NtIoException) override;
};

/**
 * @brief The output buffer's implementation.
 *
 * The output buffer should changes its size, and the vector's implementation
 * is picked up for convenience. The memory allocation strait could be changed
 * through the internal memory stepping field.
 *
 * The class is not thread safe and should never be shared among threads.
 */
class NtIoOutputBuffer : public NtIoOutputStream {
	/// Indicates how many data is inside the buffer.
	std::vector<int8_t> buffer;
public:
	/// Indicates how fast should the size of buffer incresed. The buffer
	/// should be in rounded size of (1 << (memoryStepping + 1)). Default 
	/// value for memory stepping is 5, meaning that the space reserved is 
	/// initially 64 * N.
	size_t memoryStepping;

	/// Constructor of the output buffer.
	NtIoOutputBuffer(size_t memoryStepping = 5);

	/// Destructor of the output buffer.
	virtual ~NtIoOutputBuffer();

	/// Implementation of the write method.
	virtual void write(const void*, size_t) throw (NtIoException) override;

	/// Retrieve the size of the buffer.
	inline size_t size() { return buffer.size(); }

	/// Retrieve the data of the buffer.
	inline const void* data() { return buffer.data(); }
private:
	/// Retrieve the stepping size of the memory. Guaranteed to be 2^N.
	inline size_t stepSize() { return 1 << (memoryStepping + 1); }
};