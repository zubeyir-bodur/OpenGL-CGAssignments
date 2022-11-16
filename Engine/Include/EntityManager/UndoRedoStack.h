#pragma once
#include "EntityManager/Operation.h"
#include "EntityManager/DrawList.h"
#include "Core/ErrorManager.h"
#include <stack>

class UndoRedoStack
{
private:
	std::stack<Operation> m_undo_stack;
	std::stack<Operation> m_redo_stack;
	DrawList* m_draw_list;
public:
	UndoRedoStack(DrawList* list);;

	void on_operation_performed(const Operation&);
	void on_undo();
	void on_redo();
	bool is_undo_empty();
	bool is_redo_empty();
	void clear_stacks();
};
