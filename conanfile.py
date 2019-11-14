from conans import ConanFile, tools, CMake

class rtsplib_Conan(ConanFile):
    name = "rtsplib"
    version = "0.1"
    generators = "cmake"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {
        "shared": True, 
        }
    exports_sources = "include*", "src*", "tests*", "CMakeLists.txt"

    description = "rtsp streaming library"

    requires = (
        "spdlog/1.4.2",
    )

    def build(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_SHARED_LIBS"] = self.options.shared
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy(pattern="*.h", dst="include", src="include")
        self.copy(pattern="*.so", dst="lib", keep_path=False)
        self.copy(pattern="*.lib", dst="lib", keep_path=False)
        self.copy(pattern="*.dll", dst="bin", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)

