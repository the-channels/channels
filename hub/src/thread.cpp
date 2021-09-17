#include "thread.h"
#include "hub.h"

ChannelObject* Thread::write() const
{
    std::string stripped_comment = comment;
    if (stripped_comment.length() > 1024)
    {
        stripped_comment = stripped_comment.substr(0, 1021) + "...";
    }

    declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, id.c_str(), nullptr);
    declare_str_property_on_stack(title_, OBJ_PROPERTY_TITLE, title.c_str(), &id_);
    declare_str_property_on_stack(comment_, OBJ_PROPERTY_COMMENT, stripped_comment.c_str(), &title_);
    declare_arg_property_on_stack(replies_, 'r', num_replies, &comment_);

    ChannelStackObjectProperty* last = &replies_;
    std::vector<ChannelStackObjectProperty> extra_props;
    extra_props.resize(attachments.size());
    std::vector<uint16_t> extra_ids;
    extra_ids.resize(attachments.size());

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