#include "channel.h"
#include "proto_objects.h"
#include <regex>

static const std::regex tag_pattern("\\<.*?\\>");
static const std::regex gt("&gt;");
static const std::regex lt("&lt;");
static const std::regex symbs("&#?[a-zA-Z0-9]+;");
static const std::regex br("<br>");

ChannelObject* Channel::write() const
{
    declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, get_name().c_str(), nullptr);
    declare_str_property_on_stack(title_, OBJ_PROPERTY_TITLE, get_title().c_str(), &id_);

    return channel_object_allocate(&title_);
}

std::string Channel::cache_key(const std::string& key)
{
    return "cache/" + m_name + "_" + key;
}


std::string Channel::preprocess_html(const std::string& source)
{
    std::string a = std::regex_replace(source, br, "\n");
    a = std::regex_replace(a, gt, ">");
    a = std::regex_replace(a, lt, "<");
    a = std::regex_replace(a, tag_pattern, "");
    return std::regex_replace(a, symbs, "");
}
