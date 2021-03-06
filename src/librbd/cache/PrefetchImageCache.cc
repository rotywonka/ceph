// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "PrefetchImageCache.h"
#include "include/buffer.h"
#include "common/dout.h"
#include "librbd/ImageCtx.h"
#include <algorithm>

#define dout_subsys ceph_subsys_rbd
#undef dout_prefix
#define dout_prefix *_dout << "librbd::PrefetchImageCache: " << this << " " \
                           <<  __func__ << ": "

namespace librbd {
namespace cache {

template <typename I>
PrefetchImageCache<I>::PrefetchImageCache(ImageCtx &image_ctx)
  : m_image_ctx(image_ctx), m_image_writeback(image_ctx) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 10) << "lru_queue=" << lru_queue << ", cache_entries=" << cache_entries << dendl;
}


/*  what PrefetchImageCache<I>::aio_read should do!! :
 *  get called (by ImageReadRequest::send_image_cache_request())
 *  get the image exents, and loop through, for each extent: split?? into 
 *    either each extent that comes in is split into its own sub list of offsets of CACHE_CHUNK_SIZE chunks, then these are all checked from the cache, or
 *    maybe, the whole thing gets split into a new image_extents that's just all the incoming extents split into chunks in one big list, then that gets checked?
 *    kinda doesn't matter
 *  then we dispatch the async eviction list updater
 *  but in the end, we have a lot of chunks, and for each one we check the cache - first lock it, then
 *  we use the count(key) method and see if that offset is in the cache, 
 *    if so we get that value which is a bufferlist*
 *    then we copy that back into the read bufferlist
 *  if it's not in the cache, 
 *    do that read from m_image_writeback, that should put it into the read bufferlist (???) 
 *    then can we also get it and put it in the cache??? somehow this needs to happen
 *    
 *  
 *  
 *  
 *  
*/  

template <typename I>
void PrefetchImageCache<I>::aio_read(Extents &&image_extents, bufferlist *bl,
                                        int fadvise_flags, Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << "image_extents=" << image_extents << ", "
                 << "on_finish=" << on_finish << dendl;

	
	std::vector<Extents> unique_list_of_extents;
	std::set<uint64_t> set_tracker;

	//get the extents, then call the splitting/chunking function from @Leo's code
	//<<<<<<< HEAD
	std::vector<Extents> temp;
	for(auto &it : image_extents){
		temp.push_back(extent_to_chunks(it));
	}

	//loops through the row
	for ( const auto &row : temp)
    {
			//temp list of extent
			Extents fogRow;
			//loops through the column
			for ( const auto &s : row )
        {
					//inserts into a set and checks to see if the element is already inserted
					auto ret = set_tracker.insert(s.first);
					//if inserted, insert into the vector of list
					if (ret.second==true)
            fogRow.push_back(s);

        }
			//inserts the fogRow temp vector into the vector of vector of extents.
			unique_list_of_extents.push_back(fogRow);
    }
				
	
	//begin read from cache
  
	ImageCacheEntries tempHash; 
	//checks to see if cache is empty
	//if it is, read chunks,
	//copying the hash table of cache to a temporary hash table.
	//else read from cluster
	if(!(cache_entries->empty())){
                                
		tempHash = *cache_entries;

	//else read from cluster

	}	else{


  // writeback's aio_read method used for reading from cluster
		m_image_writeback.aio_read(std::move(image_extents), bl, fadvise_flags, on_finish);                //do we assume that it's already in the (read) bufferlist 

		//  PrefetchImageCache<I>::extent_to_chunks(image_extents[0]);
	//begin read from cache
  
  // writeback's aio_read method used for reading from cluster
	//	std::unordered_map<uint64_t, ceph::bufferlist *>::iterator it = cache_entries->begin();

	//ImageCacheEntries temp; 
	//checks to see if cache -is**** ISN'T empty ---- but this still is completely wrong and also it segfaults immediately
	//if it is, read chunks,
	//copying the hash table of cache to a temporary hash table.
	//else read from cluster
//	if(!(cache_entries->empty())){
	//	temp = *cache_entries;
	//else read from cluster
//	}	else{
  // writeback's aio_read method used for reading from cluster
		m_image_writeback.aio_read(std::move(image_extents), bl, fadvise_flags, on_finish);                //do we assume that it's already in the (read) bufferlist 
//	}

	//call chunking/splitting function again from @Leo's code
	
//	}
//>>>>>>> 49909ce003100b37fae85d33e55341e46736f4ac


	}
	//call chunking/splitting function again from @Leo's code after reading from cluster

}

template <typename I>
//<<<<<<< HEAD
ImageCache::Extents PrefetchImageCache<I>::extent_to_chunks(std::pair<uint64_t, uint64_t> one_extent) {                                                       
                                                                                                                                                              
  uint64_t size;                                                                                                                                              
                                                                                                                                                              
	Extents::iterator itr;                                                                                                                                      
	Extents::iterator itrD;                                                                                                                                     
  Extents chunkedExtent;                                                                                                                                      
  uint64_t changedOffset;                                                                                                                                     
  uint64_t changedLength;                                                                                                                                     
  uint64_t offset = one_extent.first;                                                                                                                         
  uint64_t length = one_extent.second;                                                                                                                        
  uint64_t remainingLength;                                                                                                                                   
  uint64_t deficit;                                                                                                                                           
                                                                                                                                                              
  if (offset%CACHE_CHUNK_SIZE != 0) {                                                                                                                         
    changedOffset = offset-offset%CACHE_CHUNK_SIZE;             //This changes the current offset                                                             
    chunkedExtent.push_back(std::make_pair(changedOffset,CACHE_CHUNK_SIZE));                                                                                  
  }                                                                                                                                                           
  if ((length%CACHE_CHUNK_SIZE + offset) < CACHE_CHUNK_SIZE) {                    //Checks if the length is                                                   
    uint64_t remains2 = CACHE_CHUNK_SIZE - length%CACHE_CHUNK_SIZE;                                                                                           
    changedLength = length + remains2;                                                                                                                        
  } else if((offset%CACHE_CHUNK_SIZE + length) > CACHE_CHUNK_SIZE){                                                                                           
    uint64_t remains = CACHE_CHUNK_SIZE - length%CACHE_CHUNK_SIZE;                                                                                            
    changedLength = length+remains;                                                                                                                           
  }else{                                                                                                                                                      
    changedOffset = offset;                                                                                                                                   
    changedLength = length;                                                                                                                                   
  }                                                                                                                                                           
  if (changedLength > CACHE_CHUNK_SIZE) {                                                                                                                     
    while (changedLength > CACHE_CHUNK_SIZE) {                                                                                                                
                                                                                                                                                              
      remainingLength = changedLength - CACHE_CHUNK_SIZE;                                                                                                     
      changedOffset += CACHE_CHUNK_SIZE;                                                                                                                      
      changedLength -= CACHE_CHUNK_SIZE; 
			//cout << "printing" << changedOffset << endl;                                                                                                          
      chunkedExtent.push_back(std::make_pair(changedOffset,CACHE_CHUNK_SIZE));                                                                                
    }                                                                                                                                                         
  } else {                                                                                                                                                    
    chunkedExtent.push_back(std::make_pair(changedOffset,CACHE_CHUNK_SIZE));                                                                                  
  }                                                                                                                                                           
  return chunkedExtent;                                                                                                                                       
                                                                                                                                                              
}

  
template <typename I>
void PrefetchImageCache<I>::aio_write(Extents &&image_extents,
                                         bufferlist&& bl,
                                         int fadvise_flags,
                                         Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << "image_extents=" << image_extents << ", "
                 << "on_finish=" << on_finish << dendl;

  m_image_writeback.aio_write(std::move(image_extents), std::move(bl),
                              fadvise_flags, on_finish);
}

template <typename I>
void PrefetchImageCache<I>::aio_discard(uint64_t offset, uint64_t length,
                                           bool skip_partial_discard, Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << "offset=" << offset << ", "
                 << "length=" << length << ", "
                 << "on_finish=" << on_finish << dendl;

  m_image_writeback.aio_discard(offset, length, skip_partial_discard, on_finish);
}

template <typename I>
void PrefetchImageCache<I>::aio_flush(Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << "on_finish=" << on_finish << dendl;

  m_image_writeback.aio_flush(on_finish);
}

template <typename I>
void PrefetchImageCache<I>::aio_writesame(uint64_t offset, uint64_t length,
                                             bufferlist&& bl, int fadvise_flags,
                                             Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << "offset=" << offset << ", "
                 << "length=" << length << ", "
                 << "data_len=" << bl.length() << ", "
                 << "on_finish=" << on_finish << dendl;

  m_image_writeback.aio_writesame(offset, length, std::move(bl), fadvise_flags,
                                  on_finish);
}

template <typename I>
void PrefetchImageCache<I>::aio_compare_and_write(Extents &&image_extents,
                                                     bufferlist&& cmp_bl,
                                                     bufferlist&& bl,
                                                     uint64_t *mismatch_offset,
                                                     int fadvise_flags,
                                                     Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << "image_extents=" << image_extents << ", "
                 << "on_finish=" << on_finish << dendl;

  m_image_writeback.aio_compare_and_write(
    std::move(image_extents), std::move(cmp_bl), std::move(bl), mismatch_offset,
    fadvise_flags, on_finish);
}

template <typename I>
void PrefetchImageCache<I>::init(Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;    //for logging purposes
  ldout(cct, 20) << dendl;

  //begin initializing LRU and hash table.
  lru_queue = new LRUQueue();
  cache_entries = new ImageCacheEntries();
  



  cache_entries->reserve(HASH_SIZE);
  



  on_finish->complete(0);

	// init() called where? which context to use for on_finish?
	// in other implementations, init() is called in OpenRequest, totally unrelated
	// to constructor, which makes sense - image context is 'owned' by program
	// using librbd, so lifecycle is not quite the same as the actual open image connection

	// see librbd/Utils.h line 132 for create_context_callback which seems to be the kind of context that's needed for on_finish
}
	
	
template <typename I>
void PrefetchImageCache<I>::shut_down(Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << dendl;

	//erases the content of the LRU queue
	//since the content are ints, there's no need for deallocation
	//by using the erase-remove idiom
	//lru_queue -> erase(std::remove_if(lru_queue->begin(), lru_queue->end(), true), lru_queue->end());

	//calls the destructor which therefore destroys the object, not just only the reference to the object. 
	lru_queue -> clear();

	
	// erases the content of the hash table

 	// the hash table container, along with the objects in it
	delete cache_entries;
						
	}
	
	
template <typename I>
void PrefetchImageCache<I>::invalidate(Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << dendl;

  // dump cache contents (don't have anything)
  on_finish->complete(0);
}

template <typename I>
void PrefetchImageCache<I>::flush(Context *on_finish) {
  CephContext *cct = m_image_ctx.cct;
  ldout(cct, 20) << dendl;

  // internal flush -- nothing to writeback but make sure
  // in-flight IO is flushed
  aio_flush(on_finish);
}

} // namespace cache
} // namespace librbd

template class librbd::cache::PrefetchImageCache<librbd::ImageCtx>;
