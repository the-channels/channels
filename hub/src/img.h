#ifndef CHANNEL_HUB_IMG_H
#define CHANNEL_HUB_IMG_H

#include <string>
#include <thread>
#include <mutex>
#include <map>
#include "callbacks.h"

struct ClientCache;
using Cache = std::map<std::string, ClientCache>;

struct CachedImage
{
    std::unique_ptr<std::vector<uint8_t>> data;
    uint32_t data_size;
    uint32_t w;
    uint32_t h;
};

class Device;

class ImageProcessing
{
public:
    enum class ImageEncoding
    {
        color_zx,
        grayscale_zx,
        unknown,
    };

    static ImageEncoding parse_encoding(const std::string& encoding_name)
    {

        if (encoding_name == "color_zx")
        {
            return ImageProcessing::ImageEncoding::color_zx;
        }
        else if (encoding_name == "grayscale_zx")
        {
            return ImageProcessing::ImageEncoding::grayscale_zx;
        }
        else
        {
            return ImageEncoding::unknown;
        }
    }
public:
    ImageProcessing();
    ~ImageProcessing();
    GetImageResult reencode_image(const std::string& source_file,
        uint32_t target_w, uint32_t target_h,
        ImageEncoding encoding);

private:
    Device* obtain_encoding_device(ImageEncoding encoding,
        bool linear_order, uint16_t target_w, uint16_t target_h);
    void set_encoding_device(ImageEncoding encoding, float image_scale);

private:
    Device* m_color_zx;
    Device* m_grayscale_zx;
    std::map<std::string, CachedImage> m_image_cache;
    std::mutex m_image_cache_mutex;
};

#endif //CHANNEL_HUB_IMG_H
