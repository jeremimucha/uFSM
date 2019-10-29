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
    options = {"cxx_standard": [11, 14, 17]
              ,"build_tests": [True, False]
              ,"build_examples": [True, False]
              ,"build_benchmarks": [True, False]
              ,"install_examples": [True, False]
              ,"install_benchmarks": [True, False]
              }
    default_options = {"cxx_standard": 17
                      ,"build_tests": False
                      ,"build_examples": False
                      ,"build_benchmarks": False
                      ,"install_examples": False
                      ,"install_benchmarks": False
                      }
    generators = "cmake", "cmake_paths"
    scm = {
         "type": "git",  # Use "type": "svn", if local repo is managed using SVN
         "subfolder": "",
         "url": "auto",
         "revision": "auto"
      }

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_CXX_STANDARD"] = self.options.cxx_standard
        cmake.definitions["UFSM_TESTS"] = self.options.build_tests
        cmake.definitions["UFSM_BUILD_EXAMPLES"] = self.options.build_examples
        cmake.definitions["UFSM_BUILD_BENCHMARKS"] = self.options.build_benchmarks
        cmake.definitions["UFSM_INSTALL_EXAMPLES"] = self.options.build_examples and self.options.install_examples
        cmake.definitions["UFSM_INSTALL_BENCHMARKS"] = self.options.build_benchmarks and self.options.install_benchmarks
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

