#pragma once

#include "Defines.hpp"

#include <webgpu/webgpu.h>

#include <cstddef>
#include <deque>
#include <memory>

namespace WebGPUlib
{
class UploadBuffer
{
public:
    struct Allocation
    {
        WGPUBuffer buffer;
        uint64_t   offset;
    };

    Allocation allocate( std::size_t sizeInBytes, std::size_t alignment );

    void reset();

protected:
    explicit UploadBuffer( WGPUBufferUsage usage,  std::size_t pageSize = _2MB );

private:
    struct Page
    {
        Page( WGPUBufferUsage usage, std::size_t sizeInBytes );
        ~Page();

        // Check to see if this page has sufficient storage to satisfy the requested allocation.
        bool hasSpace( std::size_t sizeInBytes, std::size_t alignment ) const;

        // Allocate memory in the upload buffer.
        // The size of the allocation must not exceed the size of a single page.
        Allocation allocate( std::size_t sizeInBytes, std::size_t alignment );

        // Reset the page for reuse.
        void reset();

    private:
        WGPUBuffer  buffer   = nullptr;
        std::size_t pageSize = 0;
        std::size_t offset   = 0;
    };

    using PagePool = std::deque<std::shared_ptr<Page>>;

    // Request a page from the page pool, or create a new
    // page if there are no available pages.
    std::shared_ptr<Page> requestPage();

    PagePool pagePool;
    PagePool availablePages;

    std::shared_ptr<Page> currentPage;

    WGPUBufferUsage usage;
    std::size_t pageSize;
};
}  // namespace WebGPUlib