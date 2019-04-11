#pragma once

namespace cgb
{
	class transform : std::enable_shared_from_this<transform>
	{
	public:
		using ptr = std::shared_ptr<transform>;

		friend void attach_transform(transform::ptr pParent, transform::ptr pChild);
		friend void detach_transform(transform::ptr pParent, transform::ptr pChild);

		/** Constructs a transform with separate values for translation, rotation, and scale */
		transform(glm::vec3 pTranslation = { 0.f, 0.f, 0.f }, glm::quat pRotation = { 0.f, 0.f, 0.f, 1.f }, glm::vec3 pScale = { 1.f, 1.f, 1.f }) noexcept;
		/** Constructs a transform with coordinate transform basis vectors, and a translation */
		transform(glm::vec3 pBasisX = { 1.f, 0.f, 0.f }, glm::vec3 pBasisY = { 0.f, 1.f, 0.f }, glm::vec3 pBasisZ = { 0.f, 0.f, 1.f }, glm::vec3 pTranslation = { 0.f, 0.f, 0.f }) noexcept;
		/** Steal the guts of another transform */
		transform(transform&& other) noexcept;
		/** Copy of another transform */
		transform(const transform& other) noexcept;
		/** Steal the guts of another transform */
		transform& operator=(transform&& other) noexcept;
		/** Copy of another transform */
		transform& operator=(const transform& other) noexcept;
		virtual ~transform();

		/** sets a new position, current position is overwritten */
		transform& set_translation(const glm::vec3& value);
		/** sets a new rotation, current rotation is overwritten */
		transform& set_rotation(const glm::quat& value);
		/** sets a new scale, current scale is overwritten */
		transform& set_scale(const glm::vec3& value);

		/** translates the current basis */
		transform& translate_local(const glm::vec3& pTranslation);
		/** rotates the current basis */
		transform& rotate_local(const glm::quat& pRotation);
		/** scales the current basis */
		transform& scale_local(const glm::vec3& pScale);

		/** returns the local transformation matrix, disregarding parent transforms */
		const glm::mat4& local_transformation_matrix() const;
		/** returns the global transformation matrix, taking parent transforms into account */
		glm::mat4 global_transformation_matrix() const;

		/** Get's the front-vector in global coordinates, which is the vector pointing in negative z-direction of the basis */
		glm::vec3 front_vector();
		/** Get's the basis' z-axis in global coordinates, which is the vector pointing in positive z-direction */
		glm::vec3 x_axis();
		/** Get's the basis' y-axis in global coordinates, which is the vector pointing in positive y-direction */
		glm::vec3 y_axis();
		/** Get's the basis' x-axis in global coordinates, which is the vector pointing in positive x-direction */
		glm::vec3 z_axis();

		/** returns the quaternion representing the local rotation, disregarding parent transforms */
		glm::vec4 local_rotation();
		/** returns the quaternion representing the global rotation, taking parent transforms into account */
		glm::vec4 global_rotation();

		/** returns the local scale, disregarding parent transformations */
		glm::vec3 local_scale();
		/** returns the global scale, taking parent transformations into account */
		glm::vec3 global_scale();

		/** returns the local translation, disregarding parent transformations */
		glm::vec3 local_translation();
		/** returns the global translation, taking parent transformations into account */
		glm::vec3 global_translation();

		/**  */
		void LookAt(transform* target);
		/**  */
		void LookAt(const glm::vec3& target);
		/**  */
		void LookAlong(const glm::vec3& direction);

		/**  */
		void AlignUpVectorTowards(transform* target);
		/**  */
		void AlignUpVectorTowards(const glm::vec3& target);
		/**  */
		void AlignUpVectorAlong(const glm::vec3& direction);

		/** Returns true if this transform is a child has a parent transform. */
		bool has_parent();
		/** Returns true if this transform has child transforms. */
		bool has_childs();
		/** Returns the parent of this transform or nullptr */
		transform::ptr parent();

	private:
		/**  */
		void update_matrix_from_transforms();
		/**  */
		void update_transforms_from_matrix();

	protected:
		/** Orthogonal basis + translation in a 4x4 matrix */
		glm::mat4 mTransformationMatrix;
		/** Offset from the coordinate origin */
		glm::vec3 mTranslation;
		/** Rotation quaternion */
		glm::quat mRotation;
		/** Local scale vector */
		glm::vec3 mScale;

		/** Parent transform or nullptr */
		transform::ptr mParent;
		/** List of child transforms */
		std::vector<transform::ptr> mChilds;
	};

	void attach_transform(transform::ptr pParent, transform::ptr pChild);
	void detach_transform(transform::ptr pParent, transform::ptr pChild);
}
