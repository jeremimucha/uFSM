from conans import ConanFile, CMake, tools


class UfsmConan(ConanFile):
    name = "uFSM"
    version = "0.1.0"
    license = "MIT License"
    author = "Jeremi Mucha jam.sfinae@gmail.com"
    url = "https://github.com/jamcodes/uFSM.git"
    description = "micro (hierarhical) final state machine library"
    topics = ("C++", "FSM")
    # settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "cmake_paths"
    scm = {
         "type": "git",  # Use "type": "svn", if local repo is managed using SVN
         "subfolder": "",
         "url": "auto",
         "revision": "auto"
      }

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure(source_folder=self.source_folder, build_folder=self.build_folder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["ufsm"]
