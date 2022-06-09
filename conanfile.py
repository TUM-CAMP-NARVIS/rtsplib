from conans import ConanFile, tools, CMake

class rtsplib_Conan(ConanFile):
    name = "rtsplib"
    version = "0.1.6"
    generators = "cmake"
    settings = "os", "compiler", "build_type", "arch"
    options = {
            "shared": [True, False],
            "max_rtp_payload_size": "ANY",
    }
    default_options = {
        "shared": True,
        "max_rtp_payload_size": 1024,
        }
    exports_sources = "examples/*", "include/*", "src/*", "tests/*", "CMakeLists.txt"

    description = "rtsp streaming library"

    requires = (
        "spdlog/1.9.1",
        "gtest/1.10.0",
    )

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.definitions["BUILD_SHARED_LIBS"] = self.options.shared
        cmake.definitions["MAX_RTP_PAYLOAD_SIZE"] = self.options.max_rtp_payload_size
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy(pattern="*.h", dst="include", src="include")
        self.copy(pattern="*.so", dst="lib", keep_path=False)
        self.copy(pattern="*.lib", dst="lib", keep_path=False)
        self.copy(pattern="*.dll", dst="bin", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)

