// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/TextureView.h>
#include <anki/gr/vulkan/VulkanObject.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Texture view implementation.
class TextureViewImpl final : public TextureView, public VulkanObject<TextureView, TextureViewImpl>
{
public:
	VkImageView m_handle = {};
	TexturePtr m_tex; ///< Hold a reference.
	TextureSubresourceInfo m_subresource;
	VkImageSubresourceRange m_vkSubresource;

	/// This is a hash that depends on the Texture and the VkImageView. It's used as a replacement of
	/// TextureView::m_uuid since it creates less unique IDs.
	U64 m_hash = 0;

	TextureViewImpl(GrManager* manager)
		: TextureView(manager)
	{
	}

	~TextureViewImpl();

	ANKI_USE_RESULT Error init(const TextureViewInitInfo& inf);
};
/// @}

} // end namespace anki
