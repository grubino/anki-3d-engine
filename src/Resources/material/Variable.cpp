#include "Variable.h"
#include "../shader_program/Variable.h"
#include "../ShaderProgram.h"
#include "Util/Assert.h"
#include "Util/Exception.h"


namespace material {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Variable::Variable(
	Type type_,
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr)
:	type(type_),
 	oneSProgVar(NULL)
{
	// For all shader progs point to the variables
	for(uint i = 0; i < shaderProgsArr.size(); i++)
	{
		if(shaderProgsArr[i]->variableExists(shaderProgVarName))
		{
			sProgsVars[i] = &shaderProgsArr[i]->getVariable(shaderProgVarName);

			if(!oneSProgVar)
			{
				oneSProgVar = sProgsVars[i];
			}

			// All the sprog vars need to have same GL data type
			if(oneSProgVar->getGlDataType() != sProgsVars[i]->getGlDataType() ||
				oneSProgVar->getType() != sProgsVars[i]->getType())
			{
				throw EXCEPTION("Incompatible shader program variables: " +
					shaderProgVarName);
			}
		}
	}

	// Extra sanity checks
	if(!oneSProgVar)
	{
		throw EXCEPTION("Variable not found in any of the shader programs: " +
					shaderProgVarName);
	}
}


} // end namespace