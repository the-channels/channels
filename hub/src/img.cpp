
#include "img.h"
#include "hub.h"
#include <img2spec.h>

#include <fstream>
#include <iostream>

ImageProcessing::ImageProcessing()
{
    m_color_zx = img2spec_allocate_device(0);
    m_grayscale_zx = img2spec_allocate_device(0);

    if (img2spec_load_workspace("color_zx.isw", m_color_zx))
    {
        std::cerr << "cannot load color_zx workspace" << std::endl;
        exit(1);
    }

    if (img2spec_load_workspace("grayscale_zx.isw", m_grayscale_zx))
    {
        std::cerr << "cannot load grayscale_zx workspace" << std::endl;
        exit(1);
    }
}

ImageProcessing::~ImageProcessing()
{
    img2spec_free_device(m_color_zx);
    img2spec_free_device(m_grayscale_zx);

    m_color_zx = nullptr;
    m_grayscale_zx = nullptr;
}

Device* ImageProcessing::obtain_encoding_device(ImageEncoding encoding,
    float image_scale, bool linear_order, uint16_t target_w, uint16_t target_h)
{
    image_scale = (float)(int)(image_scale * 1000.0f) / 1000.0f;

    switch (encoding)
    {
        case ImageEncoding::color_zx:
        {
            img2spec_zx_spectrum_set_screen_order(m_color_zx, linear_order ? 0 : 1);
            img2spec_zx_spectrum_set_screen_size(m_color_zx, target_w, target_h);
            img2spec_set_scale(m_color_zx, image_scale);

            return m_color_zx;
        }
        case ImageEncoding::grayscale_zx:
        {
            img2spec_zx_spectrum_set_screen_order(m_grayscale_zx, linear_order ? 0 : 1);
            img2spec_zx_spectrum_set_screen_size(m_grayscale_zx, target_w, target_h);
            img2spec_set_scale(m_grayscale_zx, image_scale);

            return m_grayscale_zx;
        }
        default:
        {
            return nullptr;
        }
    }

    return nullptr;
}

GetImageResult ImageProcessing::reencode_image(const std::string& source_file,
    uint32_t source_w, uint32_t source_h, uint32_t target_w, uint32_t target_h,
    ImageEncoding encoding)
{
    std::string cc = source_file + "_" + std::to_string(source_w) + "_" +
        std::to_string(source_h) + "_" + std::to_string(target_w) + "_" + std::to_string((int)encoding);

    {
        std::lock_guard<std::mutex> guard(m_image_cache_mutex);

        auto ff = m_image_cache.find(cc);
        if (ff != m_image_cache.end())
        {
            return GetImageResult(CallbackStatus::ok, ff->second.data.get(), ff->second.w, ff->second.h);
        }
    }

    switch (encoding)
    {
        case ImageEncoding::color_zx:
        case ImageEncoding::grayscale_zx:
        {
            if (target_w % 8 != 0)
            {
                std::cerr << "target_w is not dividable by 8" << std::endl;
                return GetImageResult(CallbackStatus::failed);
            }
            if (target_h % 8 != 0)
            {
                std::cerr << "target_h is not dividable by 8" << std::endl;
                return GetImageResult(CallbackStatus::failed);
            }
            if (target_w > 256)
            {
                std::cerr << "target_w > 256" << std::endl;
                return GetImageResult(CallbackStatus::failed);
            }
            if (target_h > 192)
            {
                std::cerr << "target_h > 192" << std::endl;
                return GetImageResult(CallbackStatus::failed);
            }
            break;
        }
    }

    float scalew = 1.0f;
    float scaleh = 1.0f;

    if (source_w > target_w)
    {
        scalew = (float)target_w / (float)source_w;
    }

    if (source_h > target_h)
    {
        scaleh = (float)target_h / (float)source_h;
    }

    float scale = std::min(scalew, scaleh);

    bool linear_order = target_w < 256;

    Device* encoding_device = obtain_encoding_device(encoding, scale, linear_order, target_w / 8, target_h / 8);
    if (encoding_device == nullptr)
    {
        std::cerr << "Cannot obtain encoding device" << std::endl;
        return GetImageResult(CallbackStatus::unknown_resource);
    }

    if (img2spec_load_image(source_file.c_str(), encoding_device))
    {
        std::cerr << "Cannot load image" << std::endl;
        return GetImageResult(CallbackStatus::failed);
    }

    img2spec_process_image(encoding_device);

    std::unique_ptr<std::vector<uint8_t>> v(new std::vector<uint8_t>());

    img2spec_generate_result(encoding_device, [&v](unsigned char *source, uint32_t len) -> void
    {
        uint32_t old_size = v->size();
        v->resize(old_size + len);
        std::copy(source, source + len, v->begin() + old_size);
    });

    uint16_t target_pixels = (target_w / 8) * (target_h / 8) * 8;
    uint16_t target_bytes = target_pixels + (target_w / 8) * (target_h / 8);

    switch (encoding)
    {
        case ImageEncoding::grayscale_zx:
        case ImageEncoding::color_zx:
        {
            if (v->size() != target_bytes)
            {
                std::cerr << "Misaligned result" << std::endl;
                return GetImageResult(CallbackStatus::failed);
            }

            if (encoding == ImageEncoding::grayscale_zx)
            {
                // remove color
                v->resize(target_pixels);
            }

            break;
        }
    }

    {
        std::lock_guard<std::mutex> guard(m_image_cache_mutex);
        auto& cache = m_image_cache[cc];

        cache.w = target_w;
        cache.h = target_h;
        cache.data = std::move(v);

        return GetImageResult(CallbackStatus::ok, cache.data.get(), cache.w, cache.h);
    }
}