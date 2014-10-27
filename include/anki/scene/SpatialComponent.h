// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SPATIAL_COMPONENT_H
#define ANKI_SCENE_SPATIAL_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneComponent.h"
#include "anki/Collision.h"
#include "anki/util/Bitset.h"
#include "anki/util/Enum.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Spatial flags
enum class SpatialComponentFlag: U8
{
	NONE = 0,
	VISIBLE_CAMERA = 1 << 1,
	VISIBLE_LIGHT = 1 << 2,

	/// Visible or not. The visibility tester sets it
	VISIBLE_ANY = VISIBLE_CAMERA | VISIBLE_LIGHT,

	/// This is used for example in lights. If the light does not collide 
	/// with any surface then it shouldn't be visible and be processed 
	/// further. This flag is being used to check if we should test agains
	/// near plane when using the tiler for visibility tests.
	FULLY_TRANSPARENT = 1 << 3,

	MARKED_FOR_UPDATE = 1 << 4
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(SpatialComponentFlag, inline)

/// Spatial component for scene nodes. It indicates scene nodes that need to 
/// be placed in the a sector and they participate in the visibility tests
class SpatialComponent: public SceneComponent, 
	public Bitset<SpatialComponentFlag>
{
public:
	using Flag = SpatialComponentFlag;

	/// Pass the collision shape here so we can avoid the virtuals
	/// @param node The scene node. Used only to steal it's allocators
	/// @param flags A mask of SpatialFlag
	SpatialComponent(SceneNode* node, Flag flags = Flag::NONE);

	// Remove from current OctreeNode
	~SpatialComponent();

	virtual const CollisionShape& getSpatialCollisionShape() = 0;

	const Aabb& getAabb() const
	{
		return m_aabb;
	}

	/// Get optimal collision shape for visibility tests
	const CollisionShape& getVisibilityCollisionShape()
	{
		const CollisionShape& cs = getSpatialCollisionShape();
		if(cs.getType() == CollisionShape::Type::SPHERE)
		{
			return cs;
		}
		else
		{
			return m_aabb;
		}
	}

	/// Used for sorting spatials. In most object the origin is the center of
	/// mess but for cameras the origin is the eye point
	virtual Vec4 getSpatialOrigin() = 0;

	/// The derived class has to manually call this method when the collision 
	/// shape got updated
	void markForUpdate()
	{
		enableBits(Flag::MARKED_FOR_UPDATE);
	}

	/// Called when the component gets updated. It should be overriden, by 
	/// default it does nothing.
	virtual ANKI_USE_RESULT Error onSpatialComponentUpdate(
		SceneNode& node, F32 prevTime, F32 crntTime)
	{
		return ErrorCode::NONE;
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override;

	ANKI_USE_RESULT Error onUpdate(
		SceneNode& node, F32 prevTime, F32 crntTime) final
	{
		return onSpatialComponentUpdate(node, prevTime, crntTime);
	}

	/// Disable some flags
	void reset() override;
	/// @}

	static constexpr Type getClassType()
	{
		return Type::SPATIAL;
	}

private:
	Aabb m_aabb; ///< A faster shape
};

/// @}

} // end namespace anki

#endif
