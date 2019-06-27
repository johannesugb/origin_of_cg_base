#include "transform.h"

namespace cgb
{
	using namespace glm;

	void transform::update_matrix_from_transforms()
	{
		auto x = mRotation * vec3{ 1.0f, 0.0f, 0.0f };
		auto y = mRotation * vec3{ 0.0f, 1.0f, 0.0f };
		auto z = glm::cross(x, y);
		y = glm::cross(z, x);
		mMatrix = mat4(
			vec4(x, 0.0f) * mScale.x,
			vec4(y, 0.0f) * mScale.y,
			vec4(z, 0.0f) * mScale.z,
			vec4(mTranslation, 1.0f)
		);
	}
	
	void transform::update_transforms_from_matrix()
	{
		mTranslation = vec3(mMatrix[3]);
		mScale = {
			glm::length(vec3(mMatrix[0])),
			glm::length(vec3(mMatrix[1])),
			glm::length(vec3(mMatrix[2]))
		};
		mRotation = glm::quat_cast(glm::mat3(
			mMatrix[0] / mScale.x,
			mMatrix[1] / mScale.y,
			mMatrix[2] / mScale.z
		));
	}

	transform::transform(vec3 pTranslation, quat pRotation, vec3 pScale) noexcept
		: mTranslation(pTranslation)
		, mRotation(pRotation)
		, mScale(pScale)
		//, mParent(nullptr)
	{ 
		update_matrix_from_transforms();
	}
	
	transform::transform(glm::vec3 pBasisX, vec3 pBasisY, vec3 pBasisZ, vec3 pTranslation) noexcept
		: mMatrix(mat4(
			vec4(pBasisX, 0.0f)* mScale.x,
			vec4(pBasisY, 0.0f)* mScale.y,
			vec4(pBasisZ, 0.0f)* mScale.z,
			vec4(pTranslation, 1.0f)
		))
		//, mParent(nullptr)
	{
		update_transforms_from_matrix();
	}

	transform::transform(transform&& other) noexcept
		: mMatrix{ std::move(other.mMatrix) }
		, mTranslation{ std::move(other.mTranslation) }
		, mRotation{ std::move(other.mRotation) }
		, mScale{ std::move(other.mScale) }
		, mParent{ std::move(other.mParent) }
		, mChilds{ std::move(other.mChilds) }
	{
		for (auto& child : mChilds) {
			// This will overwrite their previous parent-pointer:
			attach_transform(shared_from_this(), child);
		}
		other.mParent = nullptr;
		other.mChilds.clear();
	}
	
	transform::transform(const transform& other) noexcept
		: mMatrix{ other.mMatrix }
		, mTranslation{ other.mTranslation }
		, mRotation{ other.mRotation }
		, mScale{ other.mScale }
		, mParent{ other.mParent }
	{
		for (auto& child : other.mChilds) {
			// Copy the childs. This can have undesired side effects, actually (e.g. if 
			// childs' classes are derived from transform, what happens then? Don't know.)
			auto clonedChild = std::make_shared<transform>(*child);
			attach_transform(shared_from_this(), child);
		}
	}
	
	transform& transform::operator=(transform&& other) noexcept
	{
		mMatrix = std::move(other.mMatrix);
		mTranslation = std::move(other.mTranslation);
		mRotation = std::move(other.mRotation);
		mScale = std::move(other.mScale);
		mParent = std::move(other.mParent);
		mChilds = std::move(other.mChilds);
		for (auto& child : mChilds) {
			// This will overwrite their previous parent-pointer:
			attach_transform(shared_from_this(), child);
		}
		other.mParent = nullptr;
		other.mChilds.clear();
		return *this;
	}
	
	transform& transform::operator=(const transform& other) noexcept
	{
		mMatrix = other.mMatrix;
		mTranslation = other.mTranslation;
		mRotation = other.mRotation;
		mScale = other.mScale;
		mParent = other.mParent;
		for (auto& child : other.mChilds) {
			// Copy the childs. This can have undesired side effects, actually (e.g. if 
			// childs' classes are derived from transform, what happens then? Don't know.)
			auto clonedChild = std::make_shared<transform>(*child);
			attach_transform(shared_from_this(), child);
		}
		return *this;
	}

	transform::~transform()
	{
		mParent = nullptr;
		mChilds.clear();
	}

	void transform::set_translation(const vec3& pValue)
	{
		mTranslation = pValue;
		update_matrix_from_transforms();
	}

	void transform::set_rotation(const quat& pValue)
	{
		mRotation = pValue;
		update_matrix_from_transforms();
	}

	void transform::set_scale(const vec3& pValue)
	{
		mScale = pValue;
		update_matrix_from_transforms();
	}

	void transform::look_along(const glm::vec3& pDirection)
	{
		if (glm::dot(pDirection, pDirection) < 0.0001f) {
			LOG_ERROR("Direction vector passed to transform::look_along has (almost) zero length.");
			return;
		}

		auto back = normalize(-pDirection);
		auto right = glm::normalize(glm::cross(up(), back));
		auto up = glm::normalize(glm::cross(back, right));

		mMatrix = glm::mat4(
			glm::vec4(right, 0.0f) * mScale.x,
			glm::vec4(up   , 0.0f) * mScale.y,
			glm::vec4(back , 0.0f) * mScale.z,
			vec4(mTranslation, 1.0f)
		);
		update_transforms_from_matrix();

		//// Source: opengl-tutorial, Tutorial 17 : Rotations, accessed 26.06.2019,
		////	       http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
		////

		//// Orient the front vector towards the given direction:
		//auto dir = glm::normalize(pDirection);
		//auto rot1 = rotation_between_vectors(front(), dir);

		//// Recompute desiredUp so that it's perpendicular to the direction
		//// You can skip that part if you really want to force desiredUp
		//vec3 right = glm::cross(dir, up());
		//vec3 desiredUp = glm::cross(right, dir);

		//// Because of the 1rst rotation, the up is probably completely screwed up.
		//// Find the rotation between the "up" of the rotated object, and the desired up
		//vec3 newUp = rot1 * up();
		//quat rot2 = rotation_between_vectors(newUp, desiredUp);

		//mRotation = rot2 * rot1;
		//update_matrix_from_transforms();
	}

	void transform::look_at(const glm::vec3& pTarget)
	{
		look_along(pTarget - position(*this));
	}

	void transform::look_at(const transform& pTarget)
	{
		look_along(position(pTarget) - position(*this));
	}

	const mat4& transform::local_transformation_matrix() const
	{
		return mMatrix;
	}

	glm::mat4 transform::global_transformation_matrix() const
	{
		if (mParent) {
			return mParent->global_transformation_matrix() * mMatrix;
		}
		else {
			return mMatrix;
		}
	}

	bool transform::has_parent()
	{
		return static_cast<bool>(mParent);
	}

	bool transform::has_childs()
	{
		return mChilds.size() > 0;
	}

	transform::ptr transform::parent()
	{
		return mParent;
	}

	void attach_transform(transform::ptr pParent, transform::ptr pChild)
	{
		assert(pParent);
		assert(pChild);
		if (pChild->has_parent()) {
			detach_transform(pChild->parent(), pChild);
		}
		pChild->mParent = pParent;
		pParent->mChilds.push_back(pChild);
	}

	void detach_transform(transform::ptr pParent, transform::ptr pChild)
	{
		assert(pParent);
		assert(pChild);
		if (!pChild->has_parent() || pChild->parent() != pParent) {
			LOG_WARNING(fmt::format("Can not detach child[{}] from parent[{}]", fmt::ptr(pChild.get()), fmt::ptr(pParent.get())));
			return;
		}
		pChild->mParent = nullptr;
		pParent->mChilds.erase(std::remove(
			std::begin(pParent->mChilds),
			std::end(pParent->mChilds),
			pChild
		));
	}

	glm::vec3 front_wrt(const transform& pTransform, glm::mat4 pReference)
	{
		glm::mat4 trSpace = pReference * pTransform.global_transformation_matrix();
		glm::mat4 invTrSpace = glm::inverse(trSpace);
		return invTrSpace * glm::vec4(cgb::front(pTransform), 1.0f);
	}

	glm::vec3 back_wrt(const transform& pTransform, glm::mat4 pReference)
	{
		glm::mat4 trSpace = pReference * pTransform.global_transformation_matrix();
		glm::mat4 invTrSpace = glm::inverse(trSpace);
		return invTrSpace * glm::vec4(cgb::back(pTransform), 1.0f);
	}

	glm::vec3 right_wrt(const transform& pTransform, glm::mat4 pReference)
	{
		glm::mat4 trSpace = pReference * pTransform.global_transformation_matrix();
		glm::mat4 invTrSpace = glm::inverse(trSpace);
		return invTrSpace * glm::vec4(cgb::right(pTransform), 1.0f);
	}

	glm::vec3 left_wrt(const transform& pTransform, glm::mat4 pReference)
	{
		glm::mat4 trSpace = pReference * pTransform.global_transformation_matrix();
		glm::mat4 invTrSpace = glm::inverse(trSpace);
		return invTrSpace * glm::vec4(cgb::left(pTransform), 1.0f);
	}

	glm::vec3 up_wrt(const transform& pTransform, glm::mat4 pReference)
	{
		glm::mat4 trSpace = pReference * pTransform.global_transformation_matrix();
		glm::mat4 invTrSpace = glm::inverse(trSpace);
		return invTrSpace * glm::vec4(cgb::up(pTransform), 1.0f);
	}

	glm::vec3 down_wrt(const transform& pTransform, glm::mat4 pReference)
	{
		glm::mat4 trSpace = pReference * pTransform.global_transformation_matrix();
		glm::mat4 invTrSpace = glm::inverse(trSpace);
		return invTrSpace * glm::vec4(cgb::down(pTransform), 1.0f);
	}

	void translate(transform& pTransform, const glm::vec3& pTranslation)
	{
		pTransform.set_translation(pTransform.translation() + pTranslation);
	}

	void rotate(transform& pTransform, const glm::quat& pRotation)
	{
		pTransform.set_rotation(pRotation * pTransform.rotation());
	}

	void scale(transform& pTransform, const glm::vec3& pScale)
	{
		pTransform.set_scale(pTransform.scale() * pScale);
	}

	void translate_wrt(transform& pTransform, const glm::vec3& pTranslation, glm::mat4 pReference)
	{
		// TODO: How to?
	}

	void rotate_wrt(transform& pTransform, const glm::quat& pRotation, glm::mat4 pReference)
	{
		// TODO: How to?
	}

	void scale_wrt(transform& pTransform, const glm::vec3& pScale, glm::mat4 pReference)
	{
		// TODO: How to?
	}

	glm::vec3 position(const transform& pTransform)
	{
		return pTransform.translation();
	}

	glm::vec3 position_wrt(const transform& pTransform, glm::mat4 pReference)
	{
		// TODO: How to?
		return position(pTransform);
	}


	//// Alter current transformations 

	//void transform::Translate(const vec3& translation)
	//{
	//	mTranslation += translation;
	//	DataUpdated();
	//}

	//void transform::Rotate(const vec3& axis, const float angle)
	//{
	//	mRotation = rotate(angle, axis) * mRotation;
	//	DataUpdated();
	//}

	//void transform::Rotate(const mat4& mat)
	//{
	//	mRotation = mat * mRotation;
	//	DataUpdated();
	//}

	//void transform::RotateX(const float angle)
	//{
	//	Rotate(kUnitVec3X, angle);
	//}

	//void transform::RotateY(const float angle)
	//{
	//	Rotate(kUnitVec3Y, angle);
	//}

	//void transform::RotateZ(const float angle)
	//{
	//	Rotate(kUnitVec3Z, angle);
	//}

	//void transform::Scale(const vec3& scale)
	//{
	//	mScale += scale;
	//	DataUpdated();
	//}

	//void transform::Scale(const float scale)
	//{
	//	mScale += vec3(scale);
	//	DataUpdated();
	//}


	//glm::vec3 transform::GetLocalFrontVector()
	//{
	//	glm::mat4 mM = model_matrix();
	//	glm::mat4 itM = inverseTranspose(mM);
	//	return normalize(glm::vec3(itM * kFrontVec4));
	//}




	//glm::mat4 transform::GetRotationMatrix()
	//{
	//	if (m_parent_ptr)
	//		return m_parent_ptr->GetRotationMatrix() * mRotation;
	//	else
	//		return mRotation;
	//}

	//glm::mat4 transform::rotation_matrix()
	//{
	//	return mRotation;
	//}


	//glm::vec3 transform::GetScale()
	//{
	//	if (m_parent_ptr)
	//		return m_parent_ptr->GetScale() * mScale;
	//	else
	//		return mScale;
	//}

	//glm::vec3 transform::scale()
	//{
	//	return mScale;
	//}


	//// query position and orientation-vectors

	//glm::vec3 transform::translation()
	//{
	//	return mTranslation;
	//}

	//glm::vec3 transform::GetPosition()
	//{
	//	return  get_translation_from_matrix(GetModelMatrix());
	//}

	//glm::vec3 transform::GetFrontVector()
	//{
	//	glm::mat4 mM = GetModelMatrix();
	//	glm::mat4 itM = inverseTranspose(mM);
	//	return normalize(glm::vec3(itM * kFrontVec4));
	//}

	//glm::vec3 transform::GetUpVector()
	//{
	//	glm::mat4 mM = GetModelMatrix();
	//	glm::mat4 itM = inverseTranspose(mM);
	//	return normalize(glm::vec3(itM * kUpVec4));
	//}

	//glm::vec3 transform::GetSideVector()
	//{
	//	return cross(GetFrontVector(), GetUpVector());
	//}


	//void transform::LookAt(transform* target)
	//{
	//	assert(target);
	//	LookAt(target->GetPosition());
	//}

	//void transform::LookAt(const vec3& target)
	//{
	//	glm::vec3 direction = normalize(target - GetPosition());
	//	glm::vec2 anglesToTarget = get_angles_from_direction_yaw_pitch(direction);
	//	glm::mat4 rotationToTarget = rotate(anglesToTarget.x, kUnitVec3Y) * rotate(anglesToTarget.y, kUnitVec3X);
	//	set_rotation(rotationToTarget);
	//}

	//void transform::LookAlong(const vec3& direction)
	//{
	//	glm::vec2 anglesToTarget = get_angles_from_direction_yaw_pitch(direction);
	//	glm::mat4 rotationToTarget = rotate(anglesToTarget.x, kUnitVec3Y) * rotate(anglesToTarget.y, kUnitVec3X);
	//	set_rotation(rotationToTarget);
	//}


	//void transform::AlignUpVectorTowards(transform* target)
	//{
	//	assert(target);
	//	AlignUpVectorTowards(target->GetPosition());
	//}

	//void transform::AlignUpVectorTowards(const vec3& target)
	//{
	//	glm::vec3 directon = normalize(target - GetPosition());
	//	glm::vec2 anglesToTarget = get_angles_from_direction_roll_pitch(directon);
	//	glm::mat4 rotationToTarget = rotate(anglesToTarget.x, kUnitVec3Z) * rotate(anglesToTarget.y, kUnitVec3X);
	//	set_rotation(rotationToTarget);
	//}

	//void transform::AlignUpVectorAlong(const vec3& direction)
	//{
	//	glm::vec2 anglesToTarget = get_angles_from_direction_roll_pitch(direction);
	//	glm::mat4 rotationToTarget = rotate(anglesToTarget.x, kUnitVec3Z) * rotate(anglesToTarget.y, kUnitVec3X);
	//	set_rotation(rotationToTarget);
	//}

}
