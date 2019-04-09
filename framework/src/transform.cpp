#include "transform.h"

namespace cgb
{
	const glm::mat4 transform::kIdentityMatrix(1.0f);
	const glm::vec3 transform::kFrontVec3(0.0f, 0.0f, -1.0f);
	const glm::vec4 transform::kFrontVec4(0.0f, 0.0f, -1.0f, 1.0f);
	const glm::vec3 transform::kUpVec3(0.0f, 1.0f, 0.0f);
	const glm::vec4 transform::kUpVec4(0.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec3 transform::kRightVec3(1.0f, 0.0f, 0.0f);
	const glm::vec4 transform::kRightVec4(1.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec3 transform::kUnitVec3X(1.0f, 0.0f, 0.0f);
	const glm::vec3 transform::kUnitVec3Y(0.0f, 1.0f, 0.0f);
	const glm::vec3 transform::kUnitVec3Z(0.0f, 0.0f, 1.0f);
	const glm::vec4 transform::kUnitVec4X(1.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 transform::kUnitVec4Y(0.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec4 transform::kUnitVec4Z(0.0f, 0.0f, 1.0f, 1.0f);

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
			m_model_matrix = glm::translate(mTranslation);
			m_model_matrix = m_model_matrix * mRotation;
			m_model_matrix = glm::scale(m_model_matrix, mScale);

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
		mScale = glm::vec3(scale);
		DataUpdated();
	}


	// Alter current transformations 

	void transform::Translate(const glm::vec3& translation)
	{
		mTranslation += translation;
		DataUpdated();
	}

	void transform::Rotate(const glm::vec3& axis, const float angle)
	{
		mRotation = glm::rotate(angle, axis) * mRotation;
		DataUpdated();
	}

	void transform::Rotate(const glm::mat4& mat)
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

	void transform::Scale(const glm::vec3& scale)
	{
		mScale += scale;
		DataUpdated();
	}

	void transform::Scale(const float scale)
	{
		mScale += glm::vec3(scale);
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
		glm::mat4 itM = glm::inverseTranspose(mM);
		return glm::normalize(glm::vec3(itM * kFrontVec4));
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
		glm::mat4 itM = glm::inverseTranspose(mM);
		return glm::normalize(glm::vec3(itM * kFrontVec4));
	}

	glm::vec3 transform::GetUpVector()
	{
		glm::mat4 mM = GetModelMatrix();
		glm::mat4 itM = glm::inverseTranspose(mM);
		return glm::normalize(glm::vec3(itM * kUpVec4));
	}

	glm::vec3 transform::GetSideVector()
	{
		return glm::cross(GetFrontVector(), GetUpVector());
	}


	void transform::LookAt(transform* target)
	{
		assert(target);
		LookAt(target->GetPosition());
	}

	void transform::LookAt(const glm::vec3& target)
	{
		glm::vec3 direction = glm::normalize(target - GetPosition());
		glm::vec2 anglesToTarget = get_angles_from_direction_yaw_pitch(direction);
		glm::mat4 rotationToTarget = glm::rotate(anglesToTarget.x, kUnitVec3Y) * glm::rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}

	void transform::LookAlong(const glm::vec3& direction)
	{
		glm::vec2 anglesToTarget = get_angles_from_direction_yaw_pitch(direction);
		glm::mat4 rotationToTarget = glm::rotate(anglesToTarget.x, kUnitVec3Y) * glm::rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}


	void transform::AlignUpVectorTowards(transform* target)
	{
		assert(target);
		AlignUpVectorTowards(target->GetPosition());
	}

	void transform::AlignUpVectorTowards(const glm::vec3& target)
	{
		glm::vec3 directon = glm::normalize(target - GetPosition());
		glm::vec2 anglesToTarget = get_angles_from_direction_roll_pitch(directon);
		glm::mat4 rotationToTarget = glm::rotate(anglesToTarget.x, kUnitVec3Z) * glm::rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}

	void transform::AlignUpVectorAlong(const glm::vec3& direction)
	{
		glm::vec2 anglesToTarget = get_angles_from_direction_roll_pitch(direction);
		glm::mat4 rotationToTarget = glm::rotate(anglesToTarget.x, kUnitVec3Z) * glm::rotate(anglesToTarget.y, kUnitVec3X);
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
