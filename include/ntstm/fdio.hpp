#pragma once

/**
 * @file ntstm/fdio.hpp
 * @author Haoran Luo
 * @brief Blocking file's Input/Output classes.
 *
 * This file defines the input and output stream implementation under which 
 * a blocking file descriptor is wrapped and controlled. The file should be 
 * opened prior to the creation of the stream, and the flushing and closing 
 * of the file should be managed outside the stream.
 *
 * When using the stream, it is assumed that the stream is blocking and valid
 * (for reading or writing). Any mismatch to the assumption will cause 
 * NtIoException to be raised. No rollback is supported.
 */

#include "ntstm/io.hpp"

/**
 * @brief The file descriptor implementation of the input and output stream.
 *
 * The file descriptor is assumed to be blocking and valid when the object is 
 * alive, and the creation and deletion of the file descriptor should never 
 * be managed by this file stream.
 */
class NtIoFileStream : public NtIoInputStream, public NtIoOutputStream {
	/// The file descriptor's handle.
	int fd;
public:
	/// The constructor of the file stream object.
	NtIoFileStream(int fd);
	
	/// The destructor of the file stream object.
	virtual ~NtIoFileStream();

	/// Implementation of reading from the file descriptor.
	virtual void read(void*, size_t) throw (NtIoException) override;
	
	/// Implementation of writing to the file descriptor.
	virtual void write(const void*, size_t) throw (NtIoException) override;
};