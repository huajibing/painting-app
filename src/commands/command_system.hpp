#pragma once
#include "../core/canvas.hpp"
#include "../core/layer.hpp"
#include <memory>
#include <stack>
#include <vector>

// Forward declarations
class Command;
class CommandManager;

// Base command class
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    
protected:
    friend class CommandManager;
    Canvas* canvas;
};

// Command manager - handles undo/redo stacks
class CommandManager {
public:
    CommandManager(Canvas* canvas) : canvas(canvas) {}
    
    void executeCommand(std::unique_ptr<Command> command) {
        // Clear redo stack when new command is executed
        while (!redoStack.empty()) {
            redoStack.pop();
        }
        
        // Execute and store the command
        command->canvas = canvas;
        command->execute();
        undoStack.push(std::move(command));
        
        // Limit undo stack size if needed
        if (undoStack.size() > maxUndoSteps) {
            undoStack.pop();
        }
    }
    
    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }
    
    void undo() {
        if (!canUndo()) return;
        
        // Get the last command
        auto command = std::move(undoStack.top());
        undoStack.pop();
        
        // Undo it and move to redo stack
        command->undo();
        redoStack.push(std::move(command));
    }
    
    void redo() {
        if (!canRedo()) return;
        
        // Get the last undone command
        auto command = std::move(redoStack.top());
        redoStack.pop();
        
        // Execute it again and move to undo stack
        command->execute();
        undoStack.push(std::move(command));
    }
    
    void clear() {
        while (!undoStack.empty()) undoStack.pop();
        while (!redoStack.empty()) redoStack.pop();
    }
    
private:
    Canvas* canvas;
    std::stack<std::unique_ptr<Command>> undoStack;
    std::stack<std::unique_ptr<Command>> redoStack;
    static const size_t maxUndoSteps = 50; // Limit memory usage
};