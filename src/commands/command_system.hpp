#pragma once
#include <memory>
#include <deque>
#include <vector>

// Forward declarations
class Canvas;
class Layer;
class Command;
class CommandManager;

// Base command class
class Command {
public:
    virtual ~Command() = default;
    virtual void init() = 0;
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
    
    void addCommand(std::unique_ptr<Command> command) {
        // Clear redo stack when new command is executed
        redoStack.clear();
        
        // Store the command
        command->canvas = canvas;
        command->init();
        undoStack.push_back(std::move(command));
        
        // Limit undo stack size if needed
        if (undoStack.size() > maxUndoSteps) {
            undoStack.pop_front();
        }
    }
    
    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }
    
    void undo() {
        if (!canUndo()) return;
        
        // Get the last command
        auto command = std::move(undoStack.back());
        undoStack.pop_back();
        
        // Undo it and move to redo stack
        command->undo();
        redoStack.push_back(std::move(command));
    }
    
    void redo() {
        if (!canRedo()) return;
        
        // Get the last undone command
        auto command = std::move(redoStack.back());
        redoStack.pop_back();
        
        // Execute it again and move to undo stack
        command->execute();
        undoStack.push_back(std::move(command));
    }
    
    void clear() {
        undoStack.clear();
        redoStack.clear();
    }
    
private:
    Canvas* canvas;
    std::deque<std::unique_ptr<Command>> undoStack;
    std::deque<std::unique_ptr<Command>> redoStack;
    static const size_t maxUndoSteps = 50;
};