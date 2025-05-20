# Painting App

This is a lightweight painting application built with C++, OpenGL, and Dear ImGui. It uses XMake for its build system. The application offers a range of features including various brush tools, a comprehensive layer system, file operations, undo/redo functionality, and gamepad support, aiming to provide a user-friendly digital painting experience.

## Table of Contents

- [Features](#features)
- [Screenshots](#screenshots)
- [Prerequisites/Dependencies](#prerequisitesdependencies)
- [Building the Project](#building-the-project)
- [Running the Application](#running-the-application)
- [Basic Usage and Controls](#basic-usage-and-controls)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Versatile Brush Engine**: Includes a variety of brush tools such as standard pencil, textured brushes (e.g., crayon, oil), calligraphy pen, and an eraser.
- **Comprehensive Layer System**: Manage artwork with layers, supporting creation, deletion, reordering (drag-and-drop), visibility toggles, opacity adjustment, and layer renaming.
- **Color and Brush Customization**: Features a color picker with opacity control, along with sliders for adjusting brush size and opacity.
- **Selection Tools**: Basic selection functionality for isolating parts of the artwork.
- **File Management**:
    - Create new canvases.
    - Open and Save projects in a native `.paint` format.
    - Export artwork to common image formats (e.g., PNG).
- **Undo/Redo**: Robust undo and redo capabilities for non-linear workflow.
- **User-Friendly Interface**: Built with Dear ImGui, providing an intuitive and accessible UI.
- **Gamepad Controls**: Navigate the UI and perform drawing actions using a connected gamepad.
- **Cross-Platform Support**: Designed to build and run on Windows and Linux (as indicated by the XMake build configuration).

## Screenshots

(Coming Soon - We encourage users to add screenshots of the application in action! You can insert images using markdown like this: `![Description of screenshot](path_or_url_to_screenshot.png)`)

## Prerequisites/Dependencies

1.  **XMake**: This project uses [XMake](https://xmake.io/) as its build system. You'll need to install XMake on your system first. Please follow the [official installation guide](https://xmake.io/#/guide/installation).
2.  **Git**: To clone the repository.

Once XMake is installed, you can proceed with cloning the repository:
```bash
git clone https://github.com/huajibing/painting-app.git
cd painting-app
```

The following core libraries are managed by XMake and will be downloaded automatically when you run `xmake require` or during the build process:
- GLFW (for windowing and input)
- GLAD (OpenGL loading library)
- Dear ImGui (for the graphical user interface)
- stb_image (for image loading/saving tasks, though the project uses it for loading)
- GLM (OpenGL Mathematics library)
- NativeFileDialogExtended (for native system file dialogs)

After cloning, to ensure all dependencies are fetched (though `xmake build` often handles this too):
```bash
xmake require
```

## Building the Project

After ensuring prerequisites are met and dependencies are fetched (via `xmake require`), you can build the project.

1.  **For a debug build** (default):
    ```bash
    xmake build
    ```
    Or simply:
    ```bash
    xmake
    ```

2.  **For a release build**:
    ```bash
    xmake build -m release
    ```
    Or:
    ```bash
    xmake config -m release
    xmake
    ```

Build artifacts (executables, etc.) are typically located in the `build` directory within the project's root, under a path corresponding to the platform and build mode (e.g., `build/windows/release/PaintingApp.exe` or `build/linux/release/PaintingApp`).

## Running the Application

There are two main ways to run the application after building it:

1.  **Using XMake** (recommended for ease of use):
    *   To run the default (debug) build:
        ```bash
        xmake run
        ```
    *   To run a specific build mode (e.g., release):
        ```bash
        xmake run -m release
        ```
    XMake handles finding the executable and setting the correct working directory.

2.  **Running the executable directly**:
    You can also run the compiled executable directly from its location in the `build` folder (e.g., `build/windows/release/PaintingApp.exe` or `build/linux/release/PaintingApp`).
    **Important**: When running the executable directly, ensure that the `assets` folder (containing fonts, brushes, etc.) is present in the same directory as the executable. The build process copies the `assets` folder to the target directory alongside the executable.
    For example, if your executable is at `build/windows/release/PaintingApp.exe`, the assets should be at `build/windows/release/assets/`.

## Basic Usage and Controls

The application features a main menu bar, a left toolbar for tools, and a right panel for brush settings and layer management.

### Keyboard Shortcuts:

A more detailed list can be found in the application via the Help (question mark icon) button.
-   **File Operations**:
    -   `Ctrl + N`: New File (Clears the canvas)
    -   `Ctrl + O`: Open File (`.paint` project files or image files)
    -   `Ctrl + S`: Save File (as `.paint` project)
    -   `Ctrl + Shift + S`: Save As (as `.paint` project)
    -   `Ctrl + E`: Export (as image, e.g., PNG)
-   **Edit Operations**:
    -   `Ctrl + Z`: Undo
    -   `Ctrl + Y` or `Ctrl + Shift + Z`: Redo
-   **Selection Operations (when a selection is active)**:
    -   `Ctrl + C`: Copy Selection
    -   `Ctrl + X`: Cut Selection
    -   `Ctrl + V`: Paste Selection
    -   `Delete`: Delete Selection

### Main Interface:

-   **Left Toolbar**:
    -   **Brush Tool** (Pencil Icon): For free-form drawing. Select different brush types from the right panel.
    -   **Eraser Tool** (Eraser Icon): Erases parts of the image. Uses a basic circular brush.
    -   **Pointer Tool** (Arrow Pointer Icon): Standard cursor for UI interaction, can be used to move selected content.
    -   **Selection Tool** (Crop Icon): Allows you to draw a rectangular selection area.
-   **Right Panel**:
    -   **Brush Type**: Dropdown to select different brushes (e.g., Pencil, Crayon, Oil, Calligraphy).
    -   **Brush Settings**:
        -   **Color**: Opens a color picker to choose the brush color.
        -   **Size**: Slider to control the brush diameter.
        -   **Opacity**: Slider to control the brush stroke opacity.
    -   **Layers**:
        -   **Add Layer** (`+` Icon): Creates a new layer.
        -   **Layer List**: Displays existing layers.
            -   Click to select a layer.
            -   Double-click layer name to rename.
            -   Drag and drop to reorder layers.
            -   Eye icon: Toggle layer visibility.
            -   Opacity slider (on layer item): Adjust individual layer opacity.
            -   Trash icon: Delete the selected layer.
-   **Top Menu Bar**:
    -   **File Operations**: New, Open, Save, Export buttons.
    -   **Edit Operations**: Undo, Redo buttons.
    -   **Help**: Question mark icon opens a help guide with shortcuts and tool info.

### Gamepad Controls:

The application also supports gamepad input for core actions:
-   **Left Stick**: Move cursor.
-   **Right Bumper (RB/R1)**: Draw with the current tool.
-   **Right Trigger (RT/R2)**: Accelerate cursor movement.
-   **Left Trigger (LT/L2)**: Adjust brush size (hold and move stick).
-   **D-Pad Up**: Select Brush tool.
-   **D-Pad Right**: Select Eraser tool.
-   **D-Pad Down**: Select Selection tool.
-   **Y Button (or Triangle)**: Undo.
-   **X Button (or Square)**: Redo.

## Project Structure

(Coming Soon)

## Contributing

(Coming Soon)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.