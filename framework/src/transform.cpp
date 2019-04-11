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
		mTransformationMatrix = mat4(
			vec4(x, 0.0f) * mScale.x,
			vec4(y, 0.0f) * mScale.y,
			vec4(z, 0.0f) * mScale.z,
			vec4(mTranslation, 1.0f)
		);
	}
	
	void transform::update_transforms_from_matrix()
	{
		mTranslation = mTransformationMatrix[3].xyz;
		mScale = {
			glm::length(mTransformationMatrix[0].xyz),
			glm::length(mTransformationMatrix[1].xyz),
			glm::length(mTransformationMatrix[2].xyz)
		};
		mRotation = glm::quat_cast(glm::mat3(
			mTransformationMatrix[0] / mScale.x,
			mTransformationMatrix[1] / mScale.y,
			mTransformationMatrix[2] / mScale.z
		));
	}

	transform::transform(vec3 pTranslation, quat pRotation, vec3 pScale) noexcept
		: mTranslation(pTranslation)
		, mRotation(pRotation)
		, mScale(pScale)
	{ 
		update_matrix_from_transforms();
	}
	
	transform::transform(glm::vec3 pBasisX = { 1.f, 0.f, 0.f }, vec3 pBasisY = { 0.f, 1.f, 0.f }, vec3 pBasisZ = { 0.f, 0.f, 1.f }, vec3 pTranslation = { 0.f, 0.f, 0.f }) noexcept
		: mTransformationMatrix(mat4(
			vec4(pBasisX, 0.0f)* mScale.x,
			vec4(pBasisY, 0.0f)* mScale.y,
			vec4(pBasisZ, 0.0f)* mScale.z,
			vec4(pTranslation, 1.0f)
		))
	{
		update_transforms_from_matrix();
	}
	
	transform::transform(transform&& other) noexcept
		: mTransformationMatrix{ std::move(other.mTransformationMatrix) }
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
		: mTransformationMatrix{ other.mTransformationMatrix }
		, mTranslation{ other.mTranslation }
		, mRotation{ other.mRotation }
		, mScale{ other.mScale }
		, mParent{ other.mParent }
	{
		for (auto& child : other.mChilds) {
			// Copy the childs. This can have undesired side effects, actually (e.g. if 
			// childs' classes are derived from transform, what happens then? Don't know.)
			auto clonedChild = std::make_shared<transform>(child);
			attach_transform(shared_from_this(), child);
		}
	}
	
	transform& transform::operator=(transform&& other) noexcept
	{
		mTransformationMatrix = std::move(other.mTransformationMatrix);
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
	}
	
	transform& transform::operator=(const transform& other) noexcept
	{
		mTransformationMatrix = other.mTransformationMatrix;
		mTranslation = other.mTranslation;
		mRotation = other.mRotation;
		mScale = other.mScale;
		mParent = other.mParent;
		for (auto& child : other.mChilds) {
			// Copy the childs. This can have undesired side effects, actually (e.g. if 
			// childs' classes are derived from transform, what happens then? Don't know.)
			auto clonedChild = std::make_shared<transform>(child);
			attach_transform(shared_from_this(), child);
		}
	}

	transform::~transform()
	{
		mParent = nullptr;
		mChilds.clear();
	}

	transform& transform::set_translation(const vec3& value)
	{
		mTranslation = value;
		update_matrix_from_transforms();

	}

	transform& transform::set_rotation(const quat& value)
	{
		mRotation = value;
		update_matrix_from_transforms();
	}

	transform& transform::set_scale(const vec3& value)
	{
		mScale = value;
		update_matrix_from_transforms();
	}


	transform& transform::translate_local(const vec3& pTranslation)
	{
		mTranslation += pTranslation;
		update_matrix_from_transforms();
	}

	transform& transform::rotate_local(const quat& pRotation)
	{
		mRotation = pRotation * mRotation;
		update_matrix_from_transforms();
	}

	transform& transform::scale_local(const vec3& pScale)
	{
		mScale *= pScale;
		update_matrix_from_transforms();
	}


	const mat4& transform::local_transformation_matrix() const
	{
		return mTransformationMatrix;
	}

	glm::mat4 transform::global_transformation_matrix() const
	{
		if (mParent) {
			return mParent->global_transformation_matrix() * mTransformationMatrix;
		}
		else {
			return mTransformationMatrix;
		}
	}


	glm::vec3 transform::front_vector()
	{

	}

	glm::vec3 transform::x_axis()
	{

	}

	glm::vec3 transform::y_axis()
	{

	}

	glm::vec3 transform::z_axis()
	{

	}


	glm::vec4 transform::local_rotation()
	{

	}

	glm::vec4 transform::global_rotation()
	{

	}


	glm::vec3 transform::local_scale()
	{

	}

	glm::vec3 transform::global_scale()
	{

	}


	glm::vec3 transform::local_translation()
	{

	}

	glm::vec3 transform::global_translation()
	{

	}


	void transform::LookAt(transform* target)
	{

	}

	void transform::LookAt(const vec3& target)
	{

	}

	void transform::LookAlong(const vec3& direction)
	{

	}


	void transform::AlignUpVectorTowards(transform* target)
	{

	}

	void transform::AlignUpVectorTowards(const vec3& target)
	{

	}

	void transform::AlignUpVectorAlong(const vec3& direction)
	{

	}


	bool transform::has_parent()
	{

	}

	bool transform::has_childs()
	{

	}

	transform::ptr transform::parent()
	{

	}



	transform::transform() :
		m_update_id(0),
		m_query_id(0),
		mRotation(1.0f), // rotation is a matrix
		mTranslation(0.0f),
		mScale(1.0f),
		m_model_matrix(1.0f),
		m_parent_ptr(nullptr)
	{
	}

	transform::transform(glm::vec3 position) :
		m_update_id(0),
		m_query_id(0),
		mRotation(1.0f), // rotation is a matrix
		mTranslation(std::move(position)),
		mScale(1.0f),
		m_model_matrix(1.0f),
		m_parent_ptr(nullptr)
	{
	}

	transform::transform(transform&& other) noexcept :
		m_update_id(other.m_update_id),
		m_query_id(other.m_query_id),
		mRotation(std::move(other.mRotation)),
		mTranslation(std::move(other.mTranslation)),
		mScale(std::move(other.mScale)),
		m_model_matrix(std::move(other.m_model_matrix)),
		m_parent_ptr(std::move(other.m_parent_ptr)),
		m_childs(std::move(other.m_childs))
	{
		other.m_update_id = 0;
		other.m_query_id = 0;
		other.m_parent_ptr = nullptr;
		other.m_childs.clear();
	}

	transform::transform(const transform& other) noexcept :
		m_update_id(other.m_update_id),
		m_query_id(other.m_query_id),
		mRotation(other.mRotation),
		mTranslation(other.mTranslation),
		mScale(other.mScale),
		m_model_matrix(other.m_model_matrix),
		m_parent_ptr(other.m_parent_ptr),
		m_childs(other.m_childs)
	{
	}

	transform& transform::operator=(transform&& other) noexcept
	{
		m_update_id = other.m_update_id;
		m_query_id = other.m_query_id;
		mRotation = std::move(other.mRotation);
		mTranslation = std::move(other.mTranslation);
		mScale = std::move(other.mScale);
		m_model_matrix = std::move(other.m_model_matrix);
		m_parent_ptr = std::move(other.m_parent_ptr);
		m_childs = std::move(other.m_childs);
		other.m_update_id = 0;
		other.m_query_id = 0;
		other.m_parent_ptr = nullptr;
		other.m_childs.clear();
		return *this;
	}

	transform& transform::operator=(const transform& other) noexcept
	{
		m_update_id = other.m_update_id;
		m_query_id = other.m_query_id;
		mRotation = other.mRotation;
		mTranslation = other.mTranslation;
		mScale = other.mScale;
		m_model_matrix = other.m_model_matrix;
		m_parent_ptr = other.m_parent_ptr;
		m_childs = other.m_childs;
		return *this;
	}

	transform::~transform()
	{
	}

	void transform::DataUpdated()
	{
		++m_update_id;
	}

	void transform::UpdateMatrices()
	{
		if (m_update_id != m_query_id) {
			//                                                      3.          2.        1.
			// calculate model-matrix, multiplication order is translation * rotation * scale
			m_model_matrix = translate(mTranslation);
			m_model_matrix = m_model_matrix * mRotation;
			m_model_matrix = scale(m_model_matrix, mScale);

			m_query_id = m_update_id;
		}
	}


	// Set transformations

	void transform::set_position(glm::vec3 position)
	{
		mTranslation = std::move(position);
		DataUpdated();
	}

	void transform::set_rotation(glm::mat4 rotation)
	{
		mRotation = std::move(rotation);
		DataUpdated();
	}

	void transform::set_scale(glm::vec3 scale)
	{
		mScale = std::move(scale);
		DataUpdated();
	}

	void transform::set_scale(const float scale)
	{
		mScale = vec3(scale);
		DataUpdated();
	}


	// Alter current transformations 

	void transform::Translate(const vec3& translation)
	{
		mTranslation += translation;
		DataUpdated();
	}

	void transform::Rotate(const vec3& axis, const float angle)
	{
		mRotation = rotate(angle, axis) * mRotation;
		DataUpdated();
	}

	void transform::Rotate(const mat4& mat)
	{
		mRotation = mat * mRotation;
		DataUpdated();
	}

	void transform::RotateX(const float angle)
	{
		Rotate(kUnitVec3X, angle);
	}

	void transform::RotateY(const float angle)
	{
		Rotate(kUnitVec3Y, angle);
	}

	void transform::RotateZ(const float angle)
	{
		Rotate(kUnitVec3Z, angle);
	}

	void transform::Scale(const vec3& scale)
	{
		mScale += scale;
		DataUpdated();
	}

	void transform::Scale(const float scale)
	{
		mScale += vec3(scale);
		DataUpdated();
	}



	// Query matrices

	glm::mat4 transform::GetModelMatrix()
	{
		UpdateMatrices();
		if (m_parent_ptr) {
			return m_parent_ptr->GetModelMatrix() * m_model_matrix;
		}
		else
			return m_model_matrix;
	}



	glm::mat4 transform::model_matrix()
	{
		return m_model_matrix;
	}

	glm::vec3 transform::GetLocalFrontVector()
	{
		glm::mat4 mM = model_matrix();
		glm::mat4 itM = inverseTranspose(mM);
		return normalize(glm::vec3(itM * kFrontVec4));
	}




	glm::mat4 transform::GetRotationMatrix()
	{
		if (m_parent_ptr)
			return m_parent_ptr->GetRotationMatrix() * mRotation;
		else
			return mRotation;
	}

	glm::mat4 transform::rotation_matrix()
	{
		return mRotation;
	}


	glm::vec3 transform::GetScale()
	{
		if (m_parent_ptr)
			return m_parent_ptr->GetScale() * mScale;
		else
			return mScale;
	}

	glm::vec3 transform::scale()
	{
		return mScale;
	}


	// query position and orientation-vectors

	glm::vec3 transform::translation()
	{
		return mTranslation;
	}

	glm::vec3 transform::GetPosition()
	{
		return  get_translation_from_matrix(GetModelMatrix());
	}

	glm::vec3 transform::GetFrontVector()
	{
		glm::mat4 mM = GetModelMatrix();
		glm::mat4 itM = inverseTranspose(mM);
		return normalize(glm::vec3(itM * kFrontVec4));
	}

	glm::vec3 transform::GetUpVector()
	{
		glm::mat4 mM = GetModelMatrix();
		glm::mat4 itM = inverseTranspose(mM);
		return normalize(glm::vec3(itM * kUpVec4));
	}

	glm::vec3 transform::GetSideVector()
	{
		return cross(GetFrontVector(), GetUpVector());
	}


	void transform::LookAt(transform* target)
	{
		assert(target);
		LookAt(target->GetPosition());
	}

	void transform::LookAt(const vec3& target)
	{
		glm::vec3 direction = normalize(target - GetPosition());
		glm::vec2 anglesToTarget = get_angles_from_direction_yaw_pitch(direction);
		glm::mat4 rotationToTarget = rotate(anglesToTarget.x, kUnitVec3Y) * rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}

	void transform::LookAlong(const vec3& direction)
	{
		glm::vec2 anglesToTarget = get_angles_from_direction_yaw_pitch(direction);
		glm::mat4 rotationToTarget = rotate(anglesToTarget.x, kUnitVec3Y) * rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}


	void transform::AlignUpVectorTowards(transform* target)
	{
		assert(target);
		AlignUpVectorTowards(target->GetPosition());
	}

	void transform::AlignUpVectorTowards(const vec3& target)
	{
		glm::vec3 directon = normalize(target - GetPosition());
		glm::vec2 anglesToTarget = get_angles_from_direction_roll_pitch(directon);
		glm::mat4 rotationToTarget = rotate(anglesToTarget.x, kUnitVec3Z) * rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}

	void transform::AlignUpVectorAlong(const vec3& direction)
	{
		glm::vec2 anglesToTarget = get_angles_from_direction_roll_pitch(direction);
		glm::mat4 rotationToTarget = rotate(anglesToTarget.x, kUnitVec3Z) * rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}



	transform* transform::parent()
	{
		return m_parent_ptr;
	}


	void attach_transform(transform::ptr pParent, transform::ptr pChild)
	{
		assert(pParent);
		assert(pChild);

		pChild->m_parent_ptr = pParent;

		auto result = std::find(pParent->m_childs.begin(), pParent->m_childs.end(), child);
		if (parent->m_childs.end() == result) {
			parent->m_childs.push_back(child);
		}
		else {
			LOG_WARNING(fmt::format("In AttachTransform: Prevented double-adding of child[{}] to parent[{}]", fmt::ptr(child), fmt::ptr(parent)));
		}
	}

	void detach_transform(transform::ptr pParent, transform::ptr pChild)
	{
		assert(pParent);
		assert(pChild);

		child->m_parent_ptr = nullptr;
		auto it = parent->m_childs.begin();
		while (it != parent->m_childs.end()) {
			if (*it == child) {
				break;
			}
		}

		if (it != parent->m_childs.end()) {
			parent->m_childs.erase(it);
		}
		else {
			LOG_WARNING(fmt::format("In DetachTransform: Couldn't find the Transform[{}] in the parent's[{}] m_childs", fmt::ptr(child), fmt::ptr(parent)));
		}
	}
}
