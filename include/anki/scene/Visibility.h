// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/collision/Forward.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/scene/RenderComponent.h>
#include <anki/util/NonCopyable.h>

namespace anki
{

// Forward
class Renderer;

/// @addtogroup scene
/// @{

/// Visibility test type
enum VisibilityTest
{
	VT_RENDERABLES = 1 << 0,
	VT_ONLY_SHADOW_CASTERS = 1 << 1,
	VT_LIGHTS = 1 << 2
};

/// Visible by
enum VisibleBy
{
	VB_NONE = 0,
	VB_CAMERA = 1 << 0,
	VB_LIGHT = 1 << 1
};

/// Visible node pointer with some more info
/// @note Keep this structure as small as possible
class VisibleNode
{
public:
	SceneNode* m_node = nullptr;
	/// An array of the visible spatials
	U8* m_spatialIndices = nullptr;
	/// Distance from the frustum component.
	F32 m_frustumDistanceSquared = 0.0;
	U8 m_spatialsCount = 0;

	VisibleNode()
	{
	}

	VisibleNode(const VisibleNode& other)
	{
		*this = other;
	}

	VisibleNode& operator=(const VisibleNode& other)
	{
		m_node = other.m_node;
		m_spatialIndices = other.m_spatialIndices;
		m_frustumDistanceSquared = other.m_frustumDistanceSquared;
		m_spatialsCount = other.m_spatialsCount;
		return *this;
	}

	U8 getSpatialIndex(U i) const
	{
		ANKI_ASSERT(m_spatialsCount != 0 && i < m_spatialsCount);
		return m_spatialIndices[i];
	}
};

/// The group of nodes that a VisibilityTestResults holds.
enum class VisibilityGroupType
{
	RENDERABLES,
	LIGHTS,
	FLARES,
	REFLECTION_PROBES,
	REFLECTION_PROXIES,

	TYPE_COUNT,
	FIRST = RENDERABLES
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VisibilityGroupType, inline)

/// It's actually a container for visible entities. It should be per frame.
class VisibilityTestResults
{
public:
	~VisibilityTestResults()
	{
		ANKI_ASSERT(0 && "It's supposed to be deallocated on frame start");
	}

	void create(SceneFrameAllocator<U8> alloc,
		U32 renderablesReservedSize,
		U32 lightsReservedSize,
		U32 lensFlaresReservedSize,
		U32 reflectionProbesReservedSize);

	void prepareMerge();

	VisibleNode* getBegin(VisibilityGroupType type)
	{
		return (getCount(type)) ? &m_groups[type].m_nodes[0] : nullptr;
	}

	VisibleNode* getEnd(VisibilityGroupType type)
	{
		return (getCount(type)) ? (&m_groups[type].m_nodes[0] + getCount(type))
								: nullptr;
	}

	void moveBack(SceneFrameAllocator<U8> alloc,
		VisibilityGroupType type,
		VisibleNode& x);

	U32 getCount(VisibilityGroupType type) const
	{
		return m_groups[type].m_count;
	}

	Timestamp getShapeUpdateTimestamp() const
	{
		return m_shapeUpdateTimestamp;
	}

	void setShapeUpdateTimestamp(Timestamp t)
	{
		m_shapeUpdateTimestamp = t;
	}

	void combineWith(
		SceneFrameAllocator<U8> alloc, SArray<VisibilityTestResults*>& results);

private:
	using Container = DArray<VisibleNode>;

	class Group
	{
	public:
		Container m_nodes;
		U32 m_count = 0;
	};

	Array<Group, U(VisibilityGroupType::TYPE_COUNT)> m_groups;

	Timestamp m_shapeUpdateTimestamp = 0;
};

/// Do visibility tests bypassing portals
ANKI_USE_RESULT Error doVisibilityTests(
	SceneNode& frustumable, SceneGraph& scene, const Renderer& renderer);
/// @}

} // end namespace anki
