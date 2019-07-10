#pragma once

namespace cgb
{
	/** Configuration data for a binding, containing a set-index, binding data, 
	*	and the shader stages where the bound resource might be used.
	*/
	struct binding_data
	{
		uint32_t mSetId;
		vk::DescriptorSetLayoutBinding mLayoutBinding;
		shader_type mShaderStages;
	};

	/** Compares two `binding_data` instances for equality, but only in
	*	in terms of their set-ids and binding-ids. 
	*	It does not consider equality or inequality of other members 
	*  (like of the `mLayoutBinding` or the `mShaderStages` members - they are simply ignored)
	*/
	inline bool operator ==(const binding_data& first, const binding_data& second)
	{
		return first.mSetId == second.mSetId
			&& first.mLayoutBinding.binding == second.mLayoutBinding.binding;
	}

	/** Compares two `binding_data` instances for inequality, but only in
	*	in terms of their set-ids and binding-ids. 
	*	It does not consider equality or inequality of other members 
	*  (like of the `mLayoutBinding` or the `mShaderStages` members - they are simply ignored)
	*/
	inline bool operator !=(const binding_data& first, const binding_data& second)
	{
		return !(first == second);
	}

	/** Returns true if the first binding is less than the second binding
	*	in terms of their set-ids and binding-ids. 
	*	It does not consider other members (like of the `mLayoutBinding` 
	*	or the `mShaderStages` members - they are simply ignored)
	*/
	inline bool operator <(const binding_data& first, const binding_data& second)
	{
		return	first.mSetId < second.mSetId
			|| (first.mSetId == second.mSetId && first.mLayoutBinding.binding < second.mLayoutBinding.binding);
	}

	/** Returns true if the first binding is less than or equal to the second binding
	*	in terms of their set-ids and binding-ids. 
	*	It does not consider other members (like of the `mLayoutBinding` 
	*	or the `mShaderStages` members - they are simply ignored)
	*/
	inline bool operator <=(const binding_data& first, const binding_data& second)
	{
		return first < second || first == second;
	}

	/** Returns true if the first binding is greater than the second binding
	*	in terms of their set-ids and binding-ids. 
	*	It does not consider other members (like of the `mLayoutBinding` 
	*	or the `mShaderStages` members - they are simply ignored)
	*/
	inline bool operator >(const binding_data& first, const binding_data& second)
	{
		return !(first <= second);
	}

	/** Returns true if the first binding is greater than or equal to the second binding
	*	in terms of their set-ids and binding-ids. 
	*	It does not consider other members (like of the `mLayoutBinding` 
	*	or the `mShaderStages` members - they are simply ignored)
	*/
	bool operator >=(const binding_data& first, const binding_data& second)
	{
		return !(first < second);
	}
}
