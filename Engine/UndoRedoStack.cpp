#include "UndoRedoStack.h"

UndoRedoStack::UndoRedoStack(DrawList* list) : m_draw_list(list)
{
	m_undo_stack = std::stack<Operation>();
	m_redo_stack = std::stack<Operation>();
}

void UndoRedoStack::on_operation_performed(const Operation& operation)
{
	if (m_undo_stack.size() == 5)
	{
		// Delete the bottom of the stack
		std::stack<Operation> last_four_reversed;
		while (last_four_reversed.size() < 4)
		{
			Operation tmp = m_undo_stack.top();
			m_undo_stack.pop();
			last_four_reversed.push(tmp);
		}
		m_undo_stack.pop(); // pop the last item
		while (!last_four_reversed.empty())
		{
			Operation tmp = last_four_reversed.top();
			last_four_reversed.pop();
			m_undo_stack.push(tmp);
		}
	}
	m_undo_stack.push(operation);
	while (!m_redo_stack.empty())
	{
		Operation op = m_redo_stack.top();
		ShapeModel* manipulated_shape = op.shape_manipulated();
		if (op.type() == Operation::OperationType::AddPredefined ||
			op.type() == Operation::OperationType::FinishPoly)
		{
			if (manipulated_shape != nullptr)
			{
				m_draw_list->remove_shape(manipulated_shape);
			}
		}
		m_redo_stack.pop();
	}
}

void UndoRedoStack::on_undo()
{
	if (!m_undo_stack.empty())
	{
		std::cout << "Undo!" << std::endl;
		Operation top = Operation(m_undo_stack.top());
		m_undo_stack.pop();

		switch (top.type())
		{
		case Operation::OperationType::AddPredefined:
			m_draw_list->undo_add_predefined(top.shape_manipulated());
			break;
		case Operation::OperationType::FinishPoly:
			m_draw_list->undo_finish_poly(top.shape_manipulated());
			break;
		case Operation::OperationType::MoveShape:
			m_draw_list->undo_move(top.shape_manipulated(), top.move_amount());
			break;
		case Operation::OperationType::RotateShape:
			m_draw_list->undo_rotate(top.shape_manipulated(), top.rotate_amount());
			break;
		default:
			ASSERT(false);
			break;
		}
		m_redo_stack.push(top);
	}
}

void UndoRedoStack::on_redo()
{
	if (!m_redo_stack.empty())
	{
		std::cout << "Redo!" << std::endl;
		Operation top = Operation(m_redo_stack.top());
		m_redo_stack.pop();

		switch (top.type())
		{
		case Operation::OperationType::AddPredefined:
			m_draw_list->redo_add_predefined(top.shape_manipulated());
			break;
		case Operation::OperationType::FinishPoly:
			m_draw_list->redo_finish_poly(top.shape_manipulated());
			break;
		case Operation::OperationType::MoveShape:
			m_draw_list->redo_move(top.shape_manipulated(), top.move_amount());
			break;
		case Operation::OperationType::RotateShape:
			m_draw_list->redo_rotate(top.shape_manipulated(), top.rotate_amount());
			break;
		default:
			ASSERT(false);
			break;
		}
		m_undo_stack.push(top);
	}
}

bool UndoRedoStack::is_undo_empty()
{
	return m_undo_stack.empty();
}

bool UndoRedoStack::is_redo_empty()
{
	return m_redo_stack.empty();
}

void UndoRedoStack::clear_stacks()
{
	while (!m_redo_stack.empty())
	{
		m_redo_stack.pop();
	}
	while (!m_undo_stack.empty())
	{
		m_undo_stack.pop();
	}
}
