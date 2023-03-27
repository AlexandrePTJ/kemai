from conan import ConanFile


class KemaiConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    requires = ("spdlog/1.11.0", "magic_enum/0.8.2")

    def requirements(self):
        if self.settings.os == "Macos":
            self.requires("range-v3/0.12.0")
