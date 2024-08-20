#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/RenderTarget.hpp>
#include <WebGPUlib/Sampler.hpp>
#include <WebGPUlib/Surface.hpp>
#include <WebGPUlib/Texture.hpp>
#include <WebGPUlib/TextureUnlitPipelineState.hpp>
#include <WebGPUlib/TextureView.hpp>
#include <WebGPUlib/UniformBuffer.hpp>

#include <Timer.hpp>

#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <glm/mat4x4.hpp>

#include <iostream>

using namespace WebGPUlib;

constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;
const char*   WINDOW_TITLE  = "04 - Mesh";

bool isRunning = true;

std::shared_ptr<Mesh>          cubeMesh;
std::shared_ptr<UniformBuffer> mvpBuffer;
std::shared_ptr<Texture>       depthTexture;
std::shared_ptr<TextureView>   depthTextureView;
std::shared_ptr<Texture>       albedoTexture;
std::shared_ptr<Sampler>       linearRepeatSampler;
TextureUnlitPipelineState      textureUnlitPipelineState;

void init()
{
    SDL_Init( SDL_INIT_VIDEO );

    SDL_Window* window = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                           WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE );

    if ( !window )
    {
        std::cerr << "Failed to create window." << std::endl;
        return;
    }

    Device::create( window );

    // Create a uniform buffer large enough to hold a single 4x4 matrix.
    mvpBuffer = Device::get().createUniformBuffer( nullptr, sizeof( glm::mat4 ) );

    // Create the depth texture.
    WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth32Float;

    WGPUTextureDescriptor depthTextureDescriptor = {};
    depthTextureDescriptor.label                 = "Depth Texture";
    depthTextureDescriptor.usage                 = WGPUTextureUsage_RenderAttachment;
    depthTextureDescriptor.dimension             = WGPUTextureDimension_2D;
    depthTextureDescriptor.size                  = { WINDOW_WIDTH, WINDOW_HEIGHT, 1 };
    depthTextureDescriptor.format                = depthTextureFormat;
    depthTextureDescriptor.mipLevelCount         = 1;
    depthTextureDescriptor.sampleCount           = 1;
    depthTextureDescriptor.viewFormatCount       = 1;
    depthTextureDescriptor.viewFormats           = &depthTextureFormat;

    depthTexture = Device::get().createTexture( depthTextureDescriptor );

    // Create the depth texture view.
    WGPUTextureViewDescriptor depthTextureViewDescriptor {};
    depthTextureViewDescriptor.label           = "Depth Texture View";
    depthTextureViewDescriptor.format          = depthTextureFormat;
    depthTextureViewDescriptor.dimension       = WGPUTextureViewDimension_2D;
    depthTextureViewDescriptor.baseMipLevel    = 0;
    depthTextureViewDescriptor.mipLevelCount   = 1;
    depthTextureViewDescriptor.baseArrayLayer  = 0;
    depthTextureViewDescriptor.arrayLayerCount = 1;
    depthTextureViewDescriptor.aspect          = WGPUTextureAspect_DepthOnly;

    depthTextureView = depthTexture->getView( &depthTextureViewDescriptor );

    albedoTexture = Device::get().loadTexture( "assets/textures/webgpu.png" );
    cubeMesh      = Device::get().createCube();

    // Setup the texture sampler.
    WGPUSamplerDescriptor linearRepeatSamplerDesc {};
    linearRepeatSamplerDesc.label         = "Linear Repeat Sampler";
    linearRepeatSamplerDesc.addressModeU  = WGPUAddressMode_Repeat;
    linearRepeatSamplerDesc.addressModeV  = WGPUAddressMode_Repeat;
    linearRepeatSamplerDesc.addressModeW  = WGPUAddressMode_Repeat;
    linearRepeatSamplerDesc.magFilter     = WGPUFilterMode_Linear;
    linearRepeatSamplerDesc.minFilter     = WGPUFilterMode_Linear;
    linearRepeatSamplerDesc.mipmapFilter  = WGPUMipmapFilterMode_Linear;
    linearRepeatSamplerDesc.lodMinClamp   = 0.0f;
    linearRepeatSamplerDesc.lodMaxClamp   = FLT_MAX;
    linearRepeatSamplerDesc.compare       = WGPUCompareFunction_Undefined;
    linearRepeatSamplerDesc.maxAnisotropy = 8;

    linearRepeatSampler = Device::get().createSampler( linearRepeatSamplerDesc );
}

void render()
{
    auto surface = Device::get().getSurface();

    RenderTarget renderTarget;
    renderTarget.attachTexture( AttachmentPoint::Color0, surface->getNextTextureView() );
    renderTarget.attachTexture( AttachmentPoint::DepthStencil, depthTextureView );

    auto queue = Device::get().getQueue();

    auto commandBuffer = queue->createGraphicsCommandBuffer( renderTarget, ClearFlags::Color | ClearFlags::Depth,
                                                             { 0.4f, 0.6f, 0.9f, 1.0f }, 1.0f );

    commandBuffer->setGraphicsPipeline( textureUnlitPipelineState );

    // Bind parameters.
    WGPUBindGroupEntry bindings[3] {};

    bindings[0].binding = 0;
    bindings[0].buffer  = mvpBuffer->getWGPUBuffer();
    bindings[0].offset  = 0;
    bindings[0].size    = mvpBuffer->getSize();

    bindings[1].binding     = 1;
    bindings[1].textureView = albedoTexture->getView()->getWGPUTextureView();

    bindings[2].binding = 2;
    bindings[2].sampler = linearRepeatSampler->getWGPUSampler();

    auto bindGroup = Device::get().createBindGroup( 

    commandBuffer->draw( *cubeMesh );

    queue->submit( *commandBuffer );

    surface->present();

    // Poll the device to make sure work is done.
    Device::get().poll();
}

void update( void* userdata = nullptr )
{
    static Timer    timer;
    static double   totalTime = 0.0;
    static uint64_t frames    = 0;

    timer.tick();
    frames++;

    totalTime += timer.elapsedSeconds();
    if ( totalTime > 1.0 )
    {
        std::cout << "FPS: " << frames << std::endl;
        totalTime -= 1.0;
        frames = 0;
    }

    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
        switch ( event.type )
        {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            if ( event.key.keysym.sym == SDLK_ESCAPE )
            {
                isRunning = false;
            }
            break;
        default:;
        }
    }

    render();
}

void destroy()
{
    Device::destroy();
}

int main()
{
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg( update, nullptr, 0, 1 );
#else

    while ( isRunning )
    {
        update();
    }

    destroy();

#endif

    return 0;
}
