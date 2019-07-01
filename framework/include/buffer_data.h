#pragma once

namespace cgb
{
	/** This struct contains information for a generic buffer.
	*/
	struct generic_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/** This struct contains information for a uniform buffer.
	*/
	struct uniform_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/** This struct contains information for a uniform texel buffer.
	*/
	struct uniform_texel_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/** This struct contains information for a storage buffer.
	*/
	struct storage_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/** This struct contains information for a storage texel buffer.
	*/
	struct storage_texel_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/**	This struct contains information for a buffer which is intended to be used as 
	*	vertex buffer, i.e. vertex attributes provided to a shader.
	*/
	struct vertex_buffer_data
	{
		size_t sizeof_one_element() const { return mSizeOfOneElement; }
		size_t num_elements() const { return mNumElements; }
		size_t total_size() const { return sizeof_one_element() * num_elements(); }

		size_t mSizeOfOneElement;
		size_t mNumElements;
	};

	/**	This struct contains information for a buffer which is intended to be used as 
	*	index buffer.
	*/
	struct index_buffer_data
	{
		size_t sizeof_one_element() const { return mSizeOfOneElement; }
		size_t num_elements() const { return mIndexCount; }
		size_t total_size() const { return sizeof_one_element() * num_elements(); }

		// Size of the data type which is used to represent one index element, 
		// this is typically uint16 or uint32
		size_t mSizeOfOneElement;

		// The number of indices in the buffer
		size_t mIndexCount;
	};

}
