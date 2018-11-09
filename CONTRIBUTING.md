# Contribution Guidelines

If you are contributing to cg_base, please follow these guidelines!

## Naming Conventions

```
namespace namespace_name_lower_case_separated_by_underscores
{
	/** \brief Comment every type in a meaningful manner
	 *	
	 *	The comments should be a help to the users of cg_base
	 */
    class type_name_lower_case_separated_by_underscores
    {
    public:
		/** Comment at least all public members of a class */
        void method_name_lower_case_separated_by_underscores() const
        {
            int localVariablesInCamelCase = 0; // Use explanatory comments whenever neccessary

            while (true) {
                single_statements(); // Prefer to wrap them in braces regardless!
			}
        }

		/** Getter of member variable mValue is named value */
        const complex_type& value() const
        {
            return mValue; // no conflict with value here
        }

		/** Setter of member variable mValue is called set_value 
		 *
		 *  Parameters are prefixed with 'p' and named in camel case.
		 *  The 'p' does NOT mean pointer, there is no pointer-prefix.
		 *  The 'p', instead, means just "parameter".
		 */
        void set_value(const complex_type& pValue)
        {
            mValue = pValue ; // no conflict with mValue or value here
        }

    protected:
        // the more public it is, the more important it is,
        // so order: public on top, then protected then private

        template <typename Template, typename Parameters>
        void templated_method()
        {
            // Template type parameters are named in camel case,
			// starting with a capital letter.
        }

    private:
		// Prefix members with 'm', variable name in camel case
        complex_type mValue;
    };
}

// Thanks to StackOverflow user GManNickG for the template for this
```