#pragma once
#include "ShapeModel.h"
#include <iostream>

class Operation
{
public:
	enum class OperationType
	{
		AddPredefined,
		FinishPoly,
		MoveShape,
		RotateShape
	};
private:
	ShapeModel* m_shape_manipulated;
	Angel::vec3 m_move_amount;
	Angel::vec3 m_rotate_amount;
	OperationType m_type;
public:
	Operation(OperationType t,
		ShapeModel* s,
		const Angel::vec3& move = Angel::vec3(0, 0, 0),
		const Angel::vec3& rotate = Angel::vec3(0, 0, 0)
	)
	{
		m_type = t;
		m_shape_manipulated = s;
		m_move_amount = {};
		m_rotate_amount = {};
		if (t == OperationType::MoveShape)
		{
			m_move_amount = move;
		}
		else if (t == OperationType::RotateShape)
		{
			m_rotate_amount = rotate;
		}
		std::cout << "Operation Performed!" << std::endl;
	}

	ShapeModel* shape_manipulated() {	return m_shape_manipulated;	}
	const Angel::vec3& move_amount() { return m_move_amount; }
	const Angel::vec3& rotate_amount() { return m_rotate_amount; }
	const OperationType type() { return m_type; }
};

