#include "post.h"
#include "hub.h"

ChannelObject* Post::write() const
{
    std::string stripped_comment = comment;
    if (stripped_comment.length() > 1024)
    {
        stripped_comment = stripped_comment.substr(0, 1021) + "...";
    }

    uint16_t replies_count = replies.size();

    declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, id.c_str(), nullptr);
    declare_str_property_on_stack(title_, OBJ_PROPERTY_TITLE, title.c_str(), &id_);
    declare_str_property_on_stack(comment_, OBJ_PROPERTY_COMMENT, stripped_comment.c_str(), &title_);
    declare_arg_property_on_stack(replies_, 'r', replies_count, &comment_);

    ChannelStackObjectProperty* last = &replies_;
    ChannelStackObjectProperty extra_props[attachments.size()];
    uint16_t extra_ids[attachments.size()];
    for (size_t i = 0, t = attachments.size(); i < t; i++)
    {
        extra_props[i].key = 'a';
        extra_ids[i] = ChannelHub::Get()->register_image(attachments[i].url);
        extra_props[i].value = (char*)&extra_ids[i];
        extra_props[i].value_size = sizeof(uint16_t);
        extra_props[i].prev = last;
        last = &extra_props[i];
    }

    return channel_object_allocate(last);
}