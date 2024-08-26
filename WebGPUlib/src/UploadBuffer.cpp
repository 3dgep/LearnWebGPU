#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/Helpers.hpp>
#include <WebGPUlib/UploadBuffer.hpp>

using namespace WebGPUlib;

UploadBuffer::Allocation UploadBuffer::allocate( std::size_t sizeInBytes, std::size_t alignment )
{
    if ( sizeInBytes > pageSize )
    {
        throw std::bad_alloc();
    }

    if ( !currentPage || !currentPage->hasSpace( sizeInBytes, alignment ) )
    {
        currentPage = requestPage();
    }

    return currentPage->allocate( sizeInBytes, alignment );
}

void UploadBuffer::reset()
{
    currentPage = nullptr;

    // Reset all available pages.
    availablePages = pagePool;

    for ( auto& page: availablePages )
    {
        page->reset();
    }
}

UploadBuffer::UploadBuffer( WGPUBufferUsage usage, std::size_t pageSize )
: usage{usage}
, pageSize { pageSize }
{}

UploadBuffer::Page::Page( WGPUBufferUsage usage, std::size_t sizeInBytes )
: pageSize( sizeInBytes )
{
    WGPUBufferDescriptor desc {};
    desc.label            = "UploadBuffer::Page";
    desc.usage            = usage | WGPUBufferUsage_CopyDst;
    desc.size             = sizeInBytes;
    desc.mappedAtCreation = false;

    buffer = wgpuDeviceCreateBuffer( Device::get().getWGPUDevice(), &desc );
}

UploadBuffer::Page::~Page()
{
    if ( buffer )
    {
        // wgpuBufferUnmap( buffer ); // This may not be necessary.
        wgpuBufferRelease( buffer );
    }
}

bool UploadBuffer::Page::hasSpace( std::size_t sizeInBytes, std::size_t alignment ) const
{
    std::size_t alignedSize   = AlignUp( sizeInBytes, alignment );
    std::size_t alignedOffset = AlignUp( offset, alignment );

    return alignedOffset + alignedSize < pageSize;
}

UploadBuffer::Allocation UploadBuffer::Page::allocate( std::size_t sizeInBytes, std::size_t alignment )
{
    if ( !hasSpace( sizeInBytes, alignment ) )
    {
        throw std::bad_alloc();
    }

    std::size_t alignedSize = AlignUp( sizeInBytes, alignment );
    offset                  = AlignUp( offset, alignment );

    Allocation allocation;
    allocation.buffer = buffer;
    allocation.offset = offset;

    offset += alignedSize;

    return allocation;
}

void UploadBuffer::Page::reset()
{
    offset = 0;
}

std::shared_ptr<UploadBuffer::Page> UploadBuffer::requestPage()
{
    std::shared_ptr<Page> page;

    if ( availablePages.empty() )
    {
        page = std::make_shared<Page>( usage, pageSize );
        pagePool.push_back( page );
    }
    else
    {
        page = availablePages.front();
        availablePages.pop_front();
    }

    return page;
}