add_rules("mode.debug", "mode.release")

-- Add packages before target
add_requires("glfw")
add_requires("glad")
add_requires("imgui", {configs = {glfw = true, opengl3 = true}})
add_requires("stb")
add_requires("glm")
add_requires("nativefiledialog-extended")

target("PaintingApp")
    set_kind("binary")
    set_languages("c++17")
    
    -- Add source files
    add_files("src/*.cpp")
    add_files("src/*/*.cpp")
    add_files("src/3rdparty/*.cpp")
    
    -- Add include directories
    add_includedirs("src")
    add_includedirs("src/3rdparty")
    
    -- Add dependencies with their include paths
    add_packages("glfw", "glad", "imgui", "stb", "glm", "nativefiledialog-extended")
    
    -- Set platform-specific options
    if is_plat("windows") then
        add_cxflags("/utf-8")
    end
    
    -- Enable OpenGL
    if is_plat("linux") then
        add_links("GL")
        add_links("X11")
        add_links("pthread")
        add_links("dl")
    end

    after_build(function (target)
        os.mkdir(path.join(target:targetdir(), "assets"))
        os.cp("assets/**", path.join(target:targetdir(), "assets"))
    end)