
#include "img.h"
#include "hub.h"

#include <fstream>
#include <iostream>

ImageProcessing::ImageProcessing()
{
    {
        std::cout << "Loading color_zx.isw..." << std::endl;
        std::ifstream f("color_zx.isw", std::ios_base::in);
        m_color_zx = nlohmann::json::parse(std::string((std::istreambuf_iterator<char>(f)),
            std::istreambuf_iterator<char>()));
    }
    {
        std::cout << "Loading grayscale_zx.isw..." << std::endl;
        std::ifstream f("grayscale_zx.isw", std::ios_base::in);
        m_grayscale_zx = nlohmann::json::parse(std::string((std::istreambuf_iterator<char>(f)),
            std::istreambuf_iterator<char>()));
    }
}


std::string ImageProcessing::generate_config(ImageEncoding encoding, float image_scale, bool linear_order,
    uint16_t target_w, uint16_t target_h)
{
    std::string config_name = std::string(std::tmpnam(nullptr)) + ".json";

    image_scale = (float)(int)(image_scale * 1000.0f) / 1000.0f;

    switch (encoding)
    {
        case ImageEncoding::color_zx:
        {
            std::ofstream f(config_name, std::ios_base::out | std::ios_base::binary);
            m_color_zx["Stack"]["Item[0]"]["mScale"] = image_scale;
            m_color_zx["Device"]["mOptScreenOrder"] = linear_order ? 0 : 1;
            m_color_zx["Device"]["mOptWidthCells"] = target_w;
            m_color_zx["Device"]["mOptHeightCells"] = target_h;

            std::string str = m_color_zx.dump();
            f.write((char*)str.data(), str.size());
            break;
        }
        case ImageEncoding::grayscale_zx:
        {
            std::ofstream f(config_name, std::ios_base::out | std::ios_base::binary);
            m_grayscale_zx["Stack"]["Item[0]"]["mScale"] = image_scale;
            m_grayscale_zx["Device"]["mOptScreenOrder"] = linear_order ? 0 : 1;
            m_grayscale_zx["Device"]["mOptWidthCells"] = target_w;
            m_grayscale_zx["Device"]["mOptHeightCells"] = target_h;

            std::string str = m_grayscale_zx.dump();
            f.write((char*)str.data(), str.size());
            break;
        }
        default:
            return "";
    }

    return config_name;
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

    std::string img_config = generate_config(encoding, scale, linear_order, target_w / 8, target_h / 8);
    std::string result_file = std::string(std::tmpnam(nullptr));

    std::string command =
#ifndef WIN32
        "./"
#endif
        "img2spec \"" + source_file + "\" \"" + img_config + "\" -s \"" + result_file + "\"";
    if (system(command.c_str()))
    {
        std::cerr << "Failed to execute: " << command << std::endl;
        return GetImageResult(CallbackStatus::failed);
    }
    else
    {
        if (ChannelHub::IsVerbose())
        {
            std::cout << "Executed: " << command << std::endl;
        }
    }

    std::unique_ptr<std::vector<uint8_t>> v(new std::vector<uint8_t>());

    {
        std::ifstream instream(result_file, std::ios::in | std::ios::binary);
        v->assign((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    }

    uint16_t target_pixels = (target_w / 8) * (target_h / 8) * 8;
    uint16_t target_bytes = target_pixels + (target_w / 8) * (target_h / 8);

    switch (encoding)
    {
        case ImageEncoding::grayscale_zx:
        case ImageEncoding::color_zx:
        {
            if (v->size() != target_bytes)
            {
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