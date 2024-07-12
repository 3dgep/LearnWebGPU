R"(

// When reducing the size of a texture, it could be that downscaling the texture 
// will result in a less than exactly 50% (1/2) of the original texture size.
// This happens if either the width, or the height (or both) dimensions of the texture
// are odd. For example, downscaling a 5x3 texture will result in a 2x1 texture which
// has a 60% reduction in the texture width and 66% reduction in the height.
// When this happens, we need to take more samples from the source texture to 
// determine the pixel value in the destination texture.

const WIDTH_HEIGHT_EVEN = 0u;     // Both the width and the height of the texture are even.
const WIDTH_ODD_HEIGHT_EVEN = 1u; // The texture width is odd and the height is even.
const WIDTH_EVEN_HEIGHT_ODD = 2u; // The texture width is even and teh height is odd.
const WIDTH_HEIGHT_ODD = 3u;      // Both the width and height of the texture are odd.

struct ComputeShaderInput
{
    @builtin(local_invocation_id) localId : vec3u,      // Local 3D index of the thread in the workgroup.
    @builtin(global_invocation_id) globalId : vec3u,    // Global 3D index of the thread in the dispatch.
    @builtin(workgroup_id) groupId : vec3u,             // Workgroup index in the dispatch.
    @builtin(local_invocation_index) localIndex : u32,  // Local index of the thread in the workgroup.
};

struct Mip
{
    srcMipLevel : u32, // The source mip level to downscale.
    numMips : u32, // The number of mips to write.
    dimensions : u32, // A bitfield that represents the even/odd dimension of the source texture.
    isSRGB : bool, // Apply gamma correction on sRGB textures.
    texelSize : vec2f, // 1.0 / dstMip1.Size
};

@group(0) @binding(0) var<uniform> mip : Mip;

// The source mip level to downscale.
@group(0) @binding(1) var srcMip : texture_2d<f32>;

// Write up to 4 textures per dispatch.
@group(0) @binding(2) var dstMip1 : texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(3) var dstMip2 : texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(4) var dstMip3 : texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(5) var dstMip4 : texture_storage_2d<rgba8unorm, write>;

@group(0) @binding(6) var linearClampSampler : sampler;

// The reason for separating channels is to reduce bank conflicts in the
// local data memory controller.  A large stride will cause more threads
// to collide on the same memory bank.
var<workgroup> gs_R: array<f32, 64>;
var<workgroup> gs_G: array<f32, 64>;
var<workgroup> gs_B: array<f32, 64>;
var<workgroup> gs_A: array<f32, 64>;

fn storeColor( color : vec4f, i : u32 )
{
    gs_R[i] = color.r;
    gs_G[i] = color.g;
    gs_B[i] = color.b;
    gs_A[i] = color.a;
}

fn loadColor( i : u32 ) -> vec4f
{
    return vec4f(gs_R[i], gs_G[i], gs_B[i], gs_A[i]);
}

// Source: https://en.wikipedia.org/wiki/SRGB#The_reverse_transformation
fn convertToLinear( x : vec3f ) -> vec3f
{
    return x < 0.04045f ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
}

// Source: https://en.wikipedia.org/wiki/SRGB#The_forward_transformation_(CIE_XYZ_to_sRGB)
fn convertToSRGB( x : vec3f ) -> vec3f
{
    return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
}

fn packColor( x : vec4f ) -> vec4f
{
    if (mip.isSRGB)
	{
		return vec4f(convertToSRGB(x.rgb), x.a);
	}
	else
	{
		return x;
	}
}

@compute @workgroup_size(8, 8, 1)
fn main( ComputeShaderInput IN )
{
    var src1 = vec4f(); 

    // One bilinear sample is insufficient when scaling down by more than 2x.
    // You will slightly undersample in the case where the source dimension
    // is odd.  This is why it's a really good idea to only generate mips on
    // power-of-two sized textures.  Trying to handle the undersampling case
    // will force this shader to be slower and more complicated as it will
    // have to take more source texture samples.

    // Determine the path to use based on the dimension of the 
    // source texture.
    // 0b00(0): Both width and height are even.
    // 0b01(1): Width is odd, height is even.
    // 0b10(2): Width is even, height is odd.
    // 0b11(3): Both width and height are odd.
    switch mip.dimensions {
        case WIDTH_HEIGHT_EVEN:
        {
            var uv = mip.texelSize * vec2f(IN.globalId.xy + 0.5f);
            src1 = textureSampleLevel(srcMip, linearClampSampler, uv, mip.srcMipLevel);
        }
        break;
        case WIDTH_ODD_HEIGHT_EVEN:
        {
            // > 2:1 in X dimension
            // Use 2 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
            // horizontally.
            var uv = mip.texelSize * vec2f(IN.globalId.xy + vec2f(0.25f, 0.5f);
            var offset = mip.texelSize * vec2f(0.5f, 0.0f);

            src1 = 0.5f * ( textureSampleLevel(srcMip, linearClampSampler, uv, mip.srcMipLevel ) + 
                            textureSampleLevel(srcMip, linearClampSampler, uv + offset, mip.srcMipLevel );
        }
        break;
        case WIDTH_EVEN_HEIGHT_ODD:
        {
            // > 2:1 in Y dimension
            // Use 2 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
            // vertically.
            var uv = mip.texelSize * ( IN.globalId.xy + vec2f( 0.5f, 0.25f ) );
            var offset = mip.texelSize * vec2f( 0.0, 0.5 );

            src1 = 0.5f * ( textureSampleLevel( srcMip, linearClampSampler, uv, mip.srcMipLevel ) +
                           textureSampleLevel( srcMip, linearClampSampler, uv + offset, mip.srcMipLevel ) );
        }
        break;
        case WIDTH_HEIGHT_ODD:
        {
            // > 2:1 in in both dimensions
            // Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
            // in both directions.
            var uv = mip.texelSize * ( IN.globalId.xy + vec2f( 0.25f, 0.25f ) );
            var offset = mip.texelSize * 0.5f;

            src1 =  textureSampleLevel( srcMip, linearClampSampler, uv, mip.srcMipLevel );
            src1 += textureSampleLevel( srcMip, linearClampSampler, uv + vec2f( offset.x, 0.0   ), mip.srcMipLevel );
            src1 += textureSampleLevel( srcMip, linearClampSampler, uv + vec2f( 0.0,   offset.y ), mip.srcMipLevel );
            src1 += textureSampleLevel( srcMip, linearClampSampler, uv + vec2f( offset.x, offset.y ), mip.srcMipLevel );
            src1 *= 0.25f;
        }
        break;
    }

    textureStore(dstMip1, IN.globalId.xy, packColor(src1));

    if( mip.numMips == 1 ) 
        return;

    // Without lane swizzle operations, the only way to share data with other
    // threads is through LDS.
    storeColor( src1, IN.localIndex );

    // This guarantees all LDS writes are complete and that all threads have
    // executed all instructions so far (and therefore have issued their LDS
    // write instructions.)
    workgroupBarrier();

    // With low three bits for X and high three bits for Y, this bit mask
    // (binary: 001001) checks that X and Y are even.
    if ( ( IN.localIndex & 0x9 ) == 0 )
    {
        var src2 = loadColor( IN.localIndex + 0x01 );
        var src3 = loadColor( IN.localIndex + 0x08 );
        var src4 = loadColor( IN.localIndex + 0x09 );
        src1 = 0.25f * ( src1 + src2 + src3 + src4 );

        textureStore( dstMip2, IN.globalId.xy, packColor( src1 ) );
        storeColor( src1, IN.localIndex );
    }

    if( mip.numMips == 2 ) 
        return;

    workgroupBarrier();

        // This bit mask (binary: 011011) checks that X and Y are multiples of four.
    if ( ( IN.localIndex & 0x1B ) == 0 )
    {
        var src2 = loadColor( IN.localIndex + 0x02 );
        var src3 = loadColor( IN.localIndex + 0x10 );
        var src4 = loadColor( IN.localIndex + 0x12 );
        src1 = 0.25 * ( src1 + src2 + src3 + src4 );

        textureStore( dstMip3, IN.globalId.xy / 4, packColor( src1 ) );
        storeColor( IN.localIndex, Src1 );
    }

    if ( mip.numMips == 3 )
        return;

    workgroupBarrier();

    // This bit mask would be 111111 (X & Y multiples of 8), but only one
    // thread fits that criteria.
    if ( IN.localIndex == 0 )
    {
        var src2 = loadColor( IN.localIndex + 0x04 );
        var src3 = loadColor( IN.localIndex + 0x20 );
        var src4 = loadColor( IN.localIndex + 0x24 );
        src1 = 0.25f * ( src1 + src2 + src3 + src4 );

        textureStore( dstMip4, IN.globalId.xy / 8, packColor( src1 ) );
    }
}
)"