/**
 * This file is part of Tales of Symphonia "Fix".
 *
 * Tales of Symphonia "Fix" is free software : you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by The Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Tales of Symphonia "Fix" is distributed in the hope that it will be
 * useful,
 *
 * But WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tales of Symphonia "Fix".
 *
 *   If not, see <http://www.gnu.org/licenses/>.
 *
**/

#include <d3d9.h>

#include "textures.h"
#include "../config.h"
#include "../timing.h"
#include "../hook.h"
#include "../log.h"

#include <cstdint>

#define TSFIX_TEXTURE_DIR L"TSFix_Textures"
#define TSFIX_TEXTURE_EXT L".dds"

D3DXSaveTextureToFile_pfn               D3DXSaveTextureToFile                        = nullptr;
D3DXCreateTextureFromFileInMemoryEx_pfn D3DXCreateTextureFromFileInMemoryEx_Original = nullptr;

StretchRect_pfn                         D3D9StretchRect_Original                     = nullptr;
CreateTexture_pfn                       D3D9CreateTexture_Original                   = nullptr;
CreateRenderTarget_pfn                  D3D9CreateRenderTarget_Original              = nullptr;
CreateDepthStencilSurface_pfn           D3D9CreateDepthStencilSurface_Original       = nullptr;

SetTexture_pfn                          D3D9SetTexture_Original                      = nullptr;
SetRenderTarget_pfn                     D3D9SetRenderTarget_Original                 = nullptr;
SetDepthStencilSurface_pfn              D3D9SetDepthStencilSurface_Original          = nullptr;

tsf::RenderFix::TextureManager
  tsf::RenderFix::tex_mgr;

tsf_logger_s tex_log;

std::wstring
SK_D3D9_UsageToStr (DWORD dwUsage)
{
  std::wstring usage;

  if (dwUsage & D3DUSAGE_RENDERTARGET)
    usage += L"RenderTarget ";

  if (dwUsage & D3DUSAGE_DEPTHSTENCIL)
    usage += L"Depth/Stencil ";

  if (dwUsage & D3DUSAGE_DYNAMIC)
    usage += L"Dynamic";

  if (usage.empty ())
    usage = L"Don't Care";

  return usage;
}

//jne 005C9203

//sec34FC.tmp+1C91FA - 72 D1                 - jb sec34FC.tmp+1C91CD

//sec2F9C.tmp+1CBB8A - A1 C0408A00           - mov eax,[sec2F9C.tmp+4A40C0] { [00000002] }
//sec2F9C.tmp+1CBB8F - 8B 0D AC408A00        - mov ecx,[sec2F9C.tmp+4A40AC] { [00000002] }


const wchar_t*
SK_D3D9_FormatToStr (D3DFORMAT Format)
{
  switch (Format)
  {
    case D3DFMT_UNKNOWN:
      return L"Unknown (0)";

    case D3DFMT_R8G8B8:
      return L"R8G8B8 (20)";
    case D3DFMT_A8R8G8B8:
      return L"A8R8G8B8 (21)";
    case D3DFMT_X8R8G8B8:
      return L"X8R8G8B8 (22)";
    case D3DFMT_R5G6B5               :
      return L"R5G6B5 (23)";
    case D3DFMT_X1R5G5B5             :
      return L"X1R5G5B5 (24)";
    case D3DFMT_A1R5G5B5             :
      return L"A1R5G5B5 (25)";
    case D3DFMT_A4R4G4B4             :
      return L"A4R4G4B4 (26)";
    case D3DFMT_R3G3B2               :
      return L"R3G3B2 (27)";
    case D3DFMT_A8                   :
      return L"A8 (28)";
    case D3DFMT_A8R3G3B2             :
      return L"A8R3G3B2 (29)";
    case D3DFMT_X4R4G4B4             :
      return L"X4R4G4B4 (30)";
    case D3DFMT_A2B10G10R10          :
      return L"A2B10G10R10 (31)";
    case D3DFMT_A8B8G8R8             :
      return L"A8B8G8R8 (32)";
    case D3DFMT_X8B8G8R8             :
      return L"X8B8G8R8 (33)";
    case D3DFMT_G16R16               :
      return L"G16R16 (34)";
    case D3DFMT_A2R10G10B10          :
      return L"A2R10G10B10 (35)";
    case D3DFMT_A16B16G16R16         :
      return L"A16B16G16R16 (36)";

    case D3DFMT_A8P8                 :
      return L"A8P8 (40)";
    case D3DFMT_P8                   :
      return L"P8 (41)";

    case D3DFMT_L8                   :
      return L"L8 (50)";
    case D3DFMT_A8L8                 :
      return L"A8L8 (51)";
    case D3DFMT_A4L4                 :
      return L"A4L4 (52)";

    case D3DFMT_V8U8                 :
      return L"V8U8 (60)";
    case D3DFMT_L6V5U5               :
      return L"L6V5U5 (61)";
    case D3DFMT_X8L8V8U8             :
      return L"X8L8V8U8 (62)";
    case D3DFMT_Q8W8V8U8             :
      return L"Q8W8V8U8 (63)";
    case D3DFMT_V16U16               :
      return L"V16U16 (64)";
    case D3DFMT_A2W10V10U10          :
      return L"A2W10V10U10 (67)";

    case D3DFMT_UYVY                 :
      return L"FourCC 'UYVY'";
    case D3DFMT_R8G8_B8G8            :
      return L"FourCC 'RGBG'";
    case D3DFMT_YUY2                 :
      return L"FourCC 'YUY2'";
    case D3DFMT_G8R8_G8B8            :
      return L"FourCC 'GRGB'";
    case D3DFMT_DXT1                 :
      return L"FourCC 'DXT1'";
    case D3DFMT_DXT2                 :
      return L"FourCC 'DXT2'";
    case D3DFMT_DXT3                 :
      return L"FourCC 'DXT3'";
    case D3DFMT_DXT4                 :
      return L"FourCC 'DXT4'";
    case D3DFMT_DXT5                 :
      return L"FourCC 'DXT5'";

    case D3DFMT_D16_LOCKABLE         :
      return L"D16_LOCKABLE (70)";
    case D3DFMT_D32                  :
      return L"D32 (71)";
    case D3DFMT_D15S1                :
      return L"D15S1 (73)";
    case D3DFMT_D24S8                :
      return L"D24S8 (75)";
    case D3DFMT_D24X8                :
      return L"D24X8 (77)";
    case D3DFMT_D24X4S4              :
      return L"D24X4S4 (79)";
    case D3DFMT_D16                  :
      return L"D16 (80)";

    case D3DFMT_D32F_LOCKABLE        :
      return L"D32F_LOCKABLE (82)";
    case D3DFMT_D24FS8               :
      return L"D24FS8 (83)";

/* D3D9Ex only -- */
#if !defined(D3D_DISABLE_9EX)

    /* Z-Stencil formats valid for CPU access */
    case D3DFMT_D32_LOCKABLE         :
      return L"D32_LOCKABLE (84)";
    case D3DFMT_S8_LOCKABLE          :
      return L"S8_LOCKABLE (85)";

#endif // !D3D_DISABLE_9EX



    case D3DFMT_L16                  :
      return L"L16 (81)";

    case D3DFMT_VERTEXDATA           :
      return L"VERTEXDATA (100)";
    case D3DFMT_INDEX16              :
      return L"INDEX16 (101)";
    case D3DFMT_INDEX32              :
      return L"INDEX32 (102)";

    case D3DFMT_Q16W16V16U16         :
      return L"Q16W16V16U16 (110)";

    case D3DFMT_MULTI2_ARGB8         :
      return L"FourCC 'MET1'";

    // Floating point surface formats

    // s10e5 formats (16-bits per channel)
    case D3DFMT_R16F                 :
      return L"R16F (111)";
    case D3DFMT_G16R16F              :
      return L"G16R16F (112)";
    case D3DFMT_A16B16G16R16F        :
      return L"A16B16G16R16F (113)";

    // IEEE s23e8 formats (32-bits per channel)
    case D3DFMT_R32F                 :
      return L"R32F (114)";
    case D3DFMT_G32R32F              :
      return L"G32R32F (115)";
    case D3DFMT_A32B32G32R32F        :
      return L"A32B32G32R32F (116)";

    case D3DFMT_CxV8U8               :
      return L"CxV8U8 (117)";

/* D3D9Ex only -- */
#if !defined(D3D_DISABLE_9EX)

    // Monochrome 1 bit per pixel format
    case D3DFMT_A1                   :
      return L"A1 (118)";

    // 2.8 biased fixed point
    case D3DFMT_A2B10G10R10_XR_BIAS  :
      return L"A2B10G10R10_XR_BIAS (119)";


    // Binary format indicating that the data has no inherent type
    case D3DFMT_BINARYBUFFER         :
      return L"BINARYBUFFER (199)";

#endif // !D3D_DISABLE_9EX
/* -- D3D9Ex only */
  }

  return L"UNKNOWN?!";
}

const wchar_t*
SK_D3D9_PoolToStr (D3DPOOL pool)
{
  switch (pool)
  {
    case D3DPOOL_DEFAULT:
      return L"    Default   (0)";
    case D3DPOOL_MANAGED:
      return L"    Managed   (1)";
    case D3DPOOL_SYSTEMMEM:
      return L"System Memory (2)";
    case D3DPOOL_SCRATCH:
      return L"   Scratch    (3)";
    default:
      return L"   UNKNOWN?!     ";
  }
}

COM_DECLSPEC_NOTHROW
__declspec (noinline)
HRESULT
STDMETHODCALLTYPE
D3D9StretchRect_Detour (      IDirect3DDevice9    *This,
                              IDirect3DSurface9   *pSourceSurface,
                        const RECT                *pSourceRect,
                              IDirect3DSurface9   *pDestSurface,
                        const RECT                *pDestRect,
                              D3DTEXTUREFILTERTYPE Filter )
{
#if 0
  {
    RECT source, dest;

    if (pSourceRect == nullptr) {
      D3DSURFACE_DESC desc;
      pSourceSurface->GetDesc (&desc);
      source.left   = 0;
      source.top    = 0;
      source.bottom = desc.Height;
      source.right  = desc.Width;
    } else
      source = *pSourceRect;

    if (pDestRect == nullptr) {
      D3DSURFACE_DESC desc;
      pDestSurface->GetDesc (&desc);
      dest.left   = 0;
      dest.top    = 0;
      dest.bottom = desc.Height;
      dest.right  = desc.Width;
    } else
      dest = *pDestRect;

  tex_log.Log ( L" [!] IDirect3DDevice9::StretchRect (...) "
                L"%s[%lu,%lu/%lu,%lu] :%s[%lu,%lu/%lu,%lu]",
                pSourceRect != nullptr ?
                  L" " : L" *",
                source.left, source.top, source.right, source.bottom,
                pDestRect != nullptr ?
                  L" " : L" *",
                dest.left,   dest.top,   dest.right,   dest.bottom );
  }
#endif

  return D3D9StretchRect_Original (This, pSourceSurface, pSourceRect,
                                         pDestSurface,   pDestRect,
                                         Filter);
}


COM_DECLSPEC_NOTHROW
HRESULT
STDMETHODCALLTYPE
D3D9CreateRenderTarget_Detour (IDirect3DDevice9     *This,
                               UINT                  Width,
                               UINT                  Height,
                               D3DFORMAT             Format,
                               D3DMULTISAMPLE_TYPE   MultiSample,
                               DWORD                 MultisampleQuality,
                               BOOL                  Lockable,
                               IDirect3DSurface9   **ppSurface,
                               HANDLE               *pSharedHandle)
{
  tex_log.Log (L"[Unexpected][!] IDirect3DDevice9::CreateRenderTarget (%lu, %lu, "
                      L"%lu, %lu, %lu, %lu, %08Xh, %08Xh)",
                 Width, Height, Format, MultiSample, MultisampleQuality,
                 Lockable, ppSurface, pSharedHandle);

  return D3D9CreateRenderTarget_Original (This, Width, Height, Format,
                                          MultiSample, MultisampleQuality,
                                          Lockable, ppSurface, pSharedHandle);
}

COM_DECLSPEC_NOTHROW
HRESULT
STDMETHODCALLTYPE
D3D9CreateDepthStencilSurface_Detour (IDirect3DDevice9     *This,
                                      UINT                  Width,
                                      UINT                  Height,
                                      D3DFORMAT             Format,
                                      D3DMULTISAMPLE_TYPE   MultiSample,
                                      DWORD                 MultisampleQuality,
                                      BOOL                  Discard,
                                      IDirect3DSurface9   **ppSurface,
                                      HANDLE               *pSharedHandle)
{
  tex_log.Log (L"[Unexpected][!] IDirect3DDevice9::CreateDepthStencilSurface (%lu, %lu, "
                      L"%lu, %lu, %lu, %lu, %08Xh, %08Xh)",
                 Width, Height, Format, MultiSample, MultisampleQuality,
                 Discard, ppSurface, pSharedHandle);

  return D3D9CreateDepthStencilSurface_Original (This, Width, Height, Format,
                                                 MultiSample, MultisampleQuality,
                                                 Discard, ppSurface, pSharedHandle);
}

//
// We will StretchRect (...) these into our textures whenever they are dirty and
//   one of the textures they are associated with are bound.
//
std::set           <IDirect3DSurface9*>                     dirty_surfs;
std::set           <IDirect3DSurface9*>                     msaa_surfs;       // Smurfs? :)
std::unordered_map <IDirect3DTexture9*, IDirect3DSurface9*> msaa_backing_map;
std::unordered_map <IDirect3DSurface9*, IDirect3DSurface9*> rt_msaa;

int
tsf::RenderFix::TextureManager::numMSAASurfs (void)
{
  return msaa_surfs.size ();
}

COM_DECLSPEC_NOTHROW
HRESULT
STDMETHODCALLTYPE
D3D9SetTexture_Detour (
                  _In_ IDirect3DDevice9      *This,
                  _In_ DWORD                  Sampler,
                  _In_ IDirect3DBaseTexture9 *pTexture
)
{
  // Ignore anything that's not the primary render device.
  if (This != tsf::RenderFix::pDevice) {
    return D3D9SetTexture_Original (This, Sampler, pTexture);
  }

  //
  // MSAA Blit (Before binding a texture, do MSAA resolve from its backing store)
  //
  if (config.render.msaa_samples > 0 && tsf::RenderFix::draw_state.use_msaa) {
    if (msaa_backing_map.find ((IDirect3DTexture9 *)pTexture) != msaa_backing_map.end ()) {
      IDirect3DSurface9* pSurf = nullptr;

      //
      // Only do this if the rendertarget is dirty...
      //
      if (dirty_surfs.find (msaa_backing_map [(IDirect3DTexture9 *)pTexture]) !=
          dirty_surfs.end ()) {
        if (SUCCEEDED (((IDirect3DTexture9 *)pTexture)->GetSurfaceLevel (0, &pSurf))) {
//          tex_log.Log (L"MSAA Resolve (StretchRect)");
          D3D9StretchRect_Original (This, msaa_backing_map [(IDirect3DTexture9 *)pTexture], nullptr,
                                          pSurf,                                            nullptr,
                                          D3DTEXF_NONE);
          pSurf->Release ();
        }

        // Render target is now clean, we've resolved it to its texture
        dirty_surfs.erase (msaa_backing_map [(IDirect3DTexture9 *)pTexture]);
      }
    }
  }

  return D3D9SetTexture_Original (This, Sampler, pTexture);
}

COM_DECLSPEC_NOTHROW
HRESULT
STDMETHODCALLTYPE
D3D9SetRenderTarget_Detour (
                  _In_ IDirect3DDevice9  *This,
                  _In_ DWORD              RenderTargetIndex,
                  _In_ IDirect3DSurface9 *pRenderTarget
)
{
  // Ignore anything that's not the primary render device.
  if (This != tsf::RenderFix::pDevice) {
    return D3D9SetRenderTarget_Original (This, RenderTargetIndex, pRenderTarget);
  }

  //
  // MSAA Override
  //
  if (config.render.msaa_samples > 0 && tsf::RenderFix::draw_state.use_msaa) {
    if (rt_msaa.find (pRenderTarget) != rt_msaa.end ()) {
//      tex_log.Log (L"MSAA RenderTarget Override");
      IDirect3DSurface9* pSurf = rt_msaa [pRenderTarget];
      dirty_surfs.insert (pSurf);
      return D3D9SetRenderTarget_Original (This, RenderTargetIndex, pSurf);
    }
  }

  return D3D9SetRenderTarget_Original (This, RenderTargetIndex, pRenderTarget);
}

COM_DECLSPEC_NOTHROW
HRESULT
STDMETHODCALLTYPE
D3D9SetDepthStencilSurface_Detour (
                  _In_ IDirect3DDevice9  *This,
                  _In_ IDirect3DSurface9 *pNewZStencil
)
{
  // Ignore anything that's not the primary render device.
  if (This != tsf::RenderFix::pDevice) {
    return D3D9SetDepthStencilSurface_Original (This, pNewZStencil);
  }

  //
  // MSAA Depth/Stencil Override
  //
  if (config.render.msaa_samples > 0 && tsf::RenderFix::draw_state.use_msaa) {
    if (rt_msaa.find (pNewZStencil) != rt_msaa.end ()) {
      return D3D9SetDepthStencilSurface_Original ( This,
                                                     rt_msaa [pNewZStencil] );
    }
  }

  return D3D9SetDepthStencilSurface_Original (This, pNewZStencil);
}

COM_DECLSPEC_NOTHROW
HRESULT
STDMETHODCALLTYPE
D3D9CreateTexture_Detour (IDirect3DDevice9   *This,
                          UINT                Width,
                          UINT                Height,
                          UINT                Levels,
                          DWORD               Usage,
                          D3DFORMAT           Format,
                          D3DPOOL             Pool,
                          IDirect3DTexture9 **ppTexture,
                          HANDLE             *pSharedHandle)
{
  // Ignore anything that's not the primary render device.
  if (This != tsf::RenderFix::pDevice) {
    return D3D9CreateTexture_Original ( This, Width, Height,
                                          Levels, Usage, Format,
                                            Pool, ppTexture, pSharedHandle );
  }

  if (config.textures.log) {
    tex_log.Log ( L"[Load Trace] >> Creating Texture: "
                  L"(%d x %d), Format: %s, Usage: [%s], Pool: %s",
                    Width, Height,
                      SK_D3D9_FormatToStr (Format),
                      SK_D3D9_UsageToStr  (Usage).c_str (),
                      SK_D3D9_PoolToStr   (Pool) );
  }

  bool create_msaa_surf = config.render.msaa_samples > 0 &&
                          tsf::RenderFix::draw_state.has_msaa;

  // Resize the primary framebuffer
  if (Width == 1280 && Height == 720) {
    if (((Usage & D3DUSAGE_RENDERTARGET) && Format == D3DFMT_A8R8G8B8) ||
                                            Format == D3DFMT_D24S8) {
      Width  = tsf::RenderFix::width;
      Height = tsf::RenderFix::height;
    } else {
      // Not a rendertarget!
      create_msaa_surf = false;
    }
  }

  else if (Width == 512 && Height == 256 && (Usage & D3DUSAGE_RENDERTARGET)) {
    Width  = tsf::RenderFix::width  * config.render.postproc_ratio;
    Height = tsf::RenderFix::height * config.render.postproc_ratio;
  }

  else {
    create_msaa_surf = false;
  }

  HRESULT result =
    D3D9CreateTexture_Original ( This, Width, Height,
                                   Levels, Usage, Format,
                                     Pool, ppTexture, pSharedHandle );

  if (create_msaa_surf) {
    IDirect3DSurface9* pSurf;

    HRESULT hr = E_FAIL;

    if (! (Usage & D3DUSAGE_DEPTHSTENCIL)) {
      hr = 
        D3D9CreateRenderTarget_Original ( This,
                                            Width, Height, Format,
                                              (D3DMULTISAMPLE_TYPE)config.render.msaa_samples,
                                                                   config.render.msaa_quality,
                                                FALSE,
                                                  &pSurf, nullptr);
    } else {
      hr = 
        D3D9CreateDepthStencilSurface_Original ( This,
                                                   Width, Height, Format,
                                                     (D3DMULTISAMPLE_TYPE)config.render.msaa_samples,
                                                                          config.render.msaa_quality,
                                                       FALSE,
                                                         &pSurf, nullptr);
    }

    if (SUCCEEDED (hr)) {
      msaa_surfs.insert       (pSurf);
      msaa_backing_map.insert (
        std::pair <IDirect3DTexture9*, IDirect3DSurface9*> (
          *ppTexture, pSurf
        )
      );

      IDirect3DSurface9* pFakeSurf = nullptr;
      (*ppTexture)->GetSurfaceLevel (0, &pFakeSurf);
      rt_msaa.insert (
        std::pair <IDirect3DSurface9*, IDirect3DSurface9*> (
          pFakeSurf, pSurf
        )
      );
    } else {
      tex_log.Log ( L"[ MSAA Mgr ] >> ERROR: Unable to Create MSAA Surface for Render Target: "
                    L"(%d x %d), Format: %s, Usage: [%s], Pool: %s",
                        Width, Height,
                          SK_D3D9_FormatToStr (Format),
                          SK_D3D9_UsageToStr  (Usage).c_str (),
                          SK_D3D9_PoolToStr   (Pool) );
    }
  }

  return result;
}

static uint32_t crc32_tab[] = { 
   0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 
   0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 
   0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 
   0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 
   0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 
   0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 
   0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 
   0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 
   0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 
   0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 
   0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106, 
   0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433, 
   0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 
   0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 
   0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 
   0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 
   0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 
   0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 
   0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 
   0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f, 
   0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 
   0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 
   0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 
   0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 
   0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 
   0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 
   0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e, 
   0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 
   0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 
   0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 
   0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 
   0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 
   0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 
   0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 
   0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 
   0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777, 
   0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 
   0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 
   0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 
   0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 
   0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 
   0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 
   0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d 
 };

uint32_t
crc32 (uint32_t crc, const void *buf, size_t size)
{
  const uint8_t *p;

  p = (uint8_t *)buf;
  crc = crc ^ ~0U;

  while (size--)
    crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

  return crc ^ ~0U;
}

typedef HRESULT (WINAPI *D3DXCreateTextureFromFile_pfn)
(
  _In_  LPDIRECT3DDEVICE9   pDevice,
  _In_  LPCWSTR             pSrcFile,
  _Out_ LPDIRECT3DTEXTURE9 *ppTexture
);

D3DXCreateTextureFromFile_pfn
  D3DXCreateTextureFromFile = nullptr;

#define FONT_CRC32 0xef2d9b55

COM_DECLSPEC_NOTHROW
HRESULT
STDMETHODCALLTYPE
D3DXCreateTextureFromFileInMemoryEx_Detour (
  _In_    LPDIRECT3DDEVICE9  pDevice,
  _In_    LPCVOID            pSrcData,
  _In_    UINT               SrcDataSize,
  _In_    UINT               Width,
  _In_    UINT               Height,
  _In_    UINT               MipLevels,
  _In_    DWORD              Usage,
  _In_    D3DFORMAT          Format,
  _In_    D3DPOOL            Pool,
  _In_    DWORD              Filter,
  _In_    DWORD              MipFilter,
  _In_    D3DCOLOR           ColorKey,
  _Inout_ D3DXIMAGE_INFO     *pSrcInfo,
  _Out_   PALETTEENTRY       *pPalette,
  _Out_   LPDIRECT3DTEXTURE9 *ppTexture
)
{
  // Performance statistics for caching system
  LARGE_INTEGER start, end;

  QueryPerformanceCounter_Original (&start);

  // Faster CRC32
#if 0
  uint32_t checksum =
    crc32 (0, pSrcData, min (SrcDataSize, 4096));//SrcDataSize);
  checksum =
    crc32 (checksum, (uint8_t *)pSrcData + min (1, (SrcDataSize - 4096)), min (4096, 4096 - SrcDataSize - 1));
#else
  // FULL CRC32
  uint32_t checksum =
    crc32 (0, pSrcData, SrcDataSize);
#endif

  if (config.textures.cache) {
    tsf::RenderFix::Texture* pTex = tsf::RenderFix::tex_mgr.getTexture (checksum);

    if (pTex != nullptr) {
      tsf::RenderFix::tex_mgr.refTexture (pTex);

      *ppTexture = pTex->d3d9_tex;

      return S_OK;
    }
  }

  // Necessary to make D3DX texture write functions work
  if (config.textures.dump)
    Usage = D3DUSAGE_DYNAMIC;

  if (config.textures.uncompressed) {
    if (Format == D3DFMT_DXT1 ||
        Format == D3DFMT_DXT3 ||
        Format == D3DFMT_DXT5) {
      Format = D3DFMT_A8R8G8B8;

      //MipLevels = D3DX_DEFAULT;
    }
  }

  if (config.textures.optimize_ui) {
    // Generating mipmaps adds a lot of overhead, don't do it for
    //   UI textures and that will speed things up.
    if (Width <= 128 || Height <= 128)
      MipLevels = 1;
  }

  // Generate complete mipmap chains for best image quality
  //  (will increase load-time on uncached textures)
  if (config.textures.full_mipmaps) {
    if (Width > 128 || Height > 128)
      MipLevels = D3DX_DEFAULT;
  }

  HRESULT hr = E_FAIL;

  if (checksum == FONT_CRC32) {
    if (GetFileAttributes (L"font.dds") != INVALID_FILE_ATTRIBUTES) {
      if (D3DXCreateTextureFromFile == nullptr) {
        D3DXCreateTextureFromFile =
          (D3DXCreateTextureFromFile_pfn)
            GetProcAddress ( tsf::RenderFix::d3dx9_43_dll,
                               "D3DXCreateTextureFromFileW" );
      }
      tex_log.LogEx (true, L"[   Font   ] Loading user-defined font... ");

      hr = D3DXCreateTextureFromFile (pDevice, L"font.dds", ppTexture);

      if (SUCCEEDED (hr)) {
        tex_log.LogEx (false, L"done\n");
        (*ppTexture)->Release ();
      } else
        tex_log.LogEx (false, L"failed (%x)\n", hr);
    }
  }

  // Any previous attempts to load a custom texture failed, so load it the normal way
  if (hr == E_FAIL) {
    //tex_log.Log (L"D3DXCreateTextureFromFileInMemoryEx (... MipLevels=%lu ...)", MipLevels);
    hr =
      D3DXCreateTextureFromFileInMemoryEx_Original ( pDevice,
                                                       pSrcData,         SrcDataSize,
                                                         Width,          Height,    MipLevels,
                                                           Usage,        Format,    Pool,
                                                             Filter,     MipFilter, ColorKey,
                                                               pSrcInfo, pPalette,
                                                                 ppTexture );
  }

  QueryPerformanceCounter_Original (&end);

  LARGE_INTEGER freq;
  QueryPerformanceFrequency (&freq);

  if (SUCCEEDED (hr)) {
    if (config.textures.cache) {
      tsf::RenderFix::Texture* pTex =
        new tsf::RenderFix::Texture ();

      pTex->crc32 = checksum;

      pTex->d3d9_tex = (*ppTexture);
      pTex->d3d9_tex->AddRef ();
      pTex->refs++;

      pTex->load_time = 1000.0f * (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;

      tsf::RenderFix::tex_mgr.addTexture (checksum, pTex);
    }

    if (config.textures.log) {
      tex_log.Log ( L"[Load Trace] Texture:   (%lu x %lu) * <LODs: %lu> - FAST_CRC32: %X",
                      Width, Height, (*ppTexture)->GetLevelCount (), checksum );
      tex_log.Log ( L"[Load Trace]              Usage: %-20s - Format: %-20s",
                      SK_D3D9_UsageToStr    (Usage).c_str (),
                        SK_D3D9_FormatToStr (Format) );
      tex_log.Log ( L"[Load Trace]                Pool: %s",
                      SK_D3D9_PoolToStr (Pool) );
      tex_log.Log ( L"[Load Trace]      Load Time: %6.4f ms", 
                    1000.0f * (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart );
    }
  }

  if (config.textures.dump) {
    wchar_t wszFileName [MAX_PATH] = { L'\0' };
    _swprintf ( wszFileName, L"%s\\MemoryTex_%x%s",
                  TSFIX_TEXTURE_DIR,
                    checksum,
                      TSFIX_TEXTURE_EXT );

    // Do not dump a texture that was already dumped
    if (GetFileAttributes (wszFileName) == INVALID_FILE_ATTRIBUTES)
      D3DXSaveTextureToFile (wszFileName, D3DXIFF_DDS, (*ppTexture), NULL);
  }

  return hr;
}


tsf::RenderFix::Texture*
tsf::RenderFix::TextureManager::getTexture (uint32_t checksum)
{
  if (textures.find (checksum) != textures.end ())
    return textures [checksum];

  return nullptr;
}

void
tsf::RenderFix::TextureManager::addTexture (uint32_t checksum, tsf::RenderFix::Texture* pTex)
{
  textures [checksum] = pTex;
}

void
tsf::RenderFix::TextureManager::refTexture (tsf::RenderFix::Texture* pTex)
{
  pTex->d3d9_tex->AddRef ();
  pTex->refs++;

  if (config.textures.log) {
    tex_log.Log ( L"[CacheTrace] Cache hit (%X), saved %2.1f ms",
                    pTex->crc32,
                      pTex->load_time );
  }

  time_saved += pTex->load_time;
}

void
tsf::RenderFix::TextureManager::Init (void)
{
  // Create the directory to store dumped textures
  if (config.textures.dump)
    CreateDirectoryW (L"TSFix_Textures", nullptr);

  tex_log.silent = false;
  tex_log.init ("logs/textures.log", "w+");

  TSFix_CreateDLLHook ( config.system.injector.c_str (),
                        "D3D9StretchRect_Override",
                         D3D9StretchRect_Detour,
               (LPVOID*)&D3D9StretchRect_Original );
#if 0
  TSFix_CreateDLLHook ( config.system.injector.c_str (),
                        "D3D9CreateDepthStencilSurface_Override",
                         D3D9CreateDepthStencilSurface_Detour,
               (LPVOID*)&D3D9CreateDepthStencilSurface_Original );
#endif

  TSFix_CreateDLLHook ( config.system.injector.c_str (),
                        "D3D9CreateTexture_Override",
                         D3D9CreateTexture_Detour,
               (LPVOID*)&D3D9CreateTexture_Original );

  TSFix_CreateDLLHook ( config.system.injector.c_str (),
                        "D3D9SetTexture_Override",
                         D3D9SetTexture_Detour,
               (LPVOID*)&D3D9SetTexture_Original );

  TSFix_CreateDLLHook ( config.system.injector.c_str (),
                        "D3D9SetRenderTarget_Override",
                         D3D9SetRenderTarget_Detour,
               (LPVOID*)&D3D9SetRenderTarget_Original );

  TSFix_CreateDLLHook ( config.system.injector.c_str (),
                        "D3D9SetDepthStencilSurface_Override",
                         D3D9SetDepthStencilSurface_Detour,
               (LPVOID*)&D3D9SetDepthStencilSurface_Original );

  TSFix_CreateDLLHook ( L"D3DX9_43.DLL",
                         "D3DXCreateTextureFromFileInMemoryEx",
                          D3DXCreateTextureFromFileInMemoryEx_Detour,
               (LPVOID *)&D3DXCreateTextureFromFileInMemoryEx_Original );

  D3DXSaveTextureToFile =
    (D3DXSaveTextureToFile_pfn)
      GetProcAddress ( tsf::RenderFix::d3dx9_43_dll,
        "D3DXSaveTextureToFileW" );

  // We don't hook this, but we still use it...
  if (D3D9CreateRenderTarget_Original == nullptr) {
    static HMODULE hModD3D9 =
      GetModuleHandle (config.system.injector.c_str ());
    D3D9CreateRenderTarget_Original =
      (CreateRenderTarget_pfn)
        GetProcAddress (hModD3D9, "D3D9CreateRenderTarget_Override");
  }

  // We don't hook this, but we still use it...
  if (D3D9CreateDepthStencilSurface_Original == nullptr) {
    static HMODULE hModD3D9 =
      GetModuleHandle (config.system.injector.c_str ());
    D3D9CreateDepthStencilSurface_Original =
      (CreateDepthStencilSurface_pfn)
        GetProcAddress (hModD3D9, "D3D9CreateDepthStencilSurface_Override");
  }

  time_saved = 0.0f;
}

void
tsf::RenderFix::TextureManager::Shutdown (void)
{
  // 33.3 ms per-frame (30 FPS)
  const float frame_time = 33.3f;

  tex_mgr.reset ();

  tex_log.Log ( L"[Perf Stats] At shutdown: %7.2f seconds (%7.2f frames)"
                L" saved by cache",
                  time_saved / 1000.0f,
                    time_saved / frame_time );
  tex_log.close ();
}

void
tsf::RenderFix::TextureManager::reset (void)
{
  int underflows    = 0;

  int ext_refs      = 0;
  int ext_textures  = 0;

  int release_count = 0;
  int ref_count     = 0;

  tex_log.Log (L"[ Tex. Mgr ] -- TextureManager::reset (...) -- ");

  tex_log.Log (L"[ Tex. Mgr ]   Releasing textures...");

  std::unordered_map <uint32_t, tsf::RenderFix::Texture *>::iterator it =
    textures.begin ();

  while (it != textures.end ()) {
    int tex_refs = 0;
    release_count++;
    for (int i = 0; i < (*it).second->refs; i++) {
      ref_count++;
      tex_refs = (*it).second->d3d9_tex->Release ();
    }

    if (tex_refs > 0) {
      ext_refs     += tex_refs;
      ext_textures ++;
    }

   if (tex_refs < 0) {
     ++underflows;
   }

    it = textures.erase (it);
  }

  tex_log.Log ( L"[ Tex. Mgr ]   %4d textures (%4d references)",
                  release_count,
                    ref_count );

  if (ext_refs > 0) {
    tex_log.Log ( L"[ Tex. Mgr ] >> WARNING: The game is still holding references (%d) to %d textures !!!",
                    ext_refs, ext_textures );
  }

  if (underflows) {
    tex_log.Log ( L"[ Tex. Mgr ] >> WARNING: Reference counting sanity check failed: "
                  L"Reference Underflow (%d times) !!!",
                    underflows );
  }

  if (config.render.msaa_samples > 0) {
    tex_log.Log ( L"[ MSAA Mgr ]   Releasing MSAA surfaces...");

    int count = 0,
        refs  = 0;

    std::unordered_map <IDirect3DSurface9*, IDirect3DSurface9*>::iterator it =
      rt_msaa.begin ();

    while (it != rt_msaa.end ()) {
      ++count;

      if ((*it).first != nullptr)
        refs += (*it).first->Release ();
      else
        refs++;

      if ((*it).second != nullptr)
        refs += (*it).second->Release ();
      else
        refs++;

      it = rt_msaa.erase (it);
    }

    dirty_surfs.clear      ();
    msaa_surfs.clear       ();
    msaa_backing_map.clear ();

    tex_log.Log ( L"[ MSAA Mgr ]   %4d surfaces (%4d zombies)\n",
                      count, refs );
  }

  tex_log.Log (L"[ Tex. Mgr ] ----------- Finished ------------ ");
}