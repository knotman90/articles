#ifndef _SMALL_ALLOCATOR_H_
#define _SMALL_ALLOCATOR_H_

#include <cstddef> //std:.size_t
#include <cassert> // assert
#include <vector>
#include <limits> // std::numeric_limits

template< std::size_t CHUNK_SIZE, typename = std::enable_if< 0 < CHUNK_SIZE >>
struct FixedSizeAllocator{
	using uchar = unsigned char;
	using mem_pointer = uchar*;

private:
	
//-------- CHUNK -------- 
	class Chunk{
	public:
		uchar m_available_blocks;
		uchar m_head;
		mem_pointer m_pdata;

		inline void reset(const uchar num_blocks,const std::size_t block_size){
			m_head = 0;
			uchar i = 1;
			mem_pointer ptr = m_pdata;
			for(; i < num_blocks ; ++i, ptr+=block_size)
				*ptr=i;
		}
	
		Chunk(const uchar num_blocks,const std::size_t block_size){
			assert(block_size > 0);
			assert(num_blocks > 0);
			m_pdata = new uchar[num_blocks*block_size];
			reset(num_blocks, block_size);			
		}

		Chunk(const Chunk&) = delete;

		void* Allocate(std::size_t block_size){
			if(m_available_blocks <= 0)
				return nullptr;

			mem_pointer ret = m_pdata+m_head*block_size;
			m_head = *m_pdata;
			--m_available_blocks;

			return ret;
		}
		void Deallocate(void* p, std::size_t block_size){
			mem_pointer release = static_cast<mem_pointer>(p);
			//check alignment
			assert(p > m_pdata);
			assert((release-m_pdata) % block_size ==0 );

			*release = m_head;
			//new head is the index of the just released block
			const uchar release_idx = (release-m_pdata) / block_size;
			m_head = release_idx;
			++m_available_blocks;
		}

		~Chunk(){
			delete[] m_pdata;
		}
	};
	//-------- CHUNK -------- 

	std::vector<Chunk> m_chunks;
	uchar m_num_blocks;
	uchar m_block_size;
	Chunk* m_lastAlloc;
	Chunk* m_lastDealloc;

public:

	FixedSizeAllocator(const std::size_t block_size)
	: m_block_size(block_size), m_lastDealloc(nullptr), m_lastAlloc(nullptr){
		assert(block_size > 0);
		constexpr uchar UCHAR_MAX_VALUE = std::numeric_limits<uchar>::max();
		m_num_blocks = CHUNK_SIZE / block_size;
		if (m_num_blocks > UCHAR_MAX_VALUE) m_num_blocks = UCHAR_MAX_VALUE;

	}
};






#endif //_SMALL_ALLOCATOR_H_