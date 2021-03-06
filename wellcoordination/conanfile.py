from conans import ConanFile, CMake


class DoryDemoConan(ConanFile):
    name = "mu"
    version = "0.0.1"
    license = "MIT"
    # url = "TODO"
    description = "Hamsaz RDMA"
    settings = {
        "os": None,
        "compiler": {
            "gcc": {"libcxx": "libstdc++11", "cppstd": ["17", "20"], "version": None},
            "clang": {"libcxx": "libstdc++11", "cppstd": ["17", "20"], "version": None},
        },
        "build_type": None,
        "arch": None,
    }
    options = {
        "shared": [True, False],
        "log_level": ["TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"],
        "lto": [True, False],
    }
    default_options = {
        "shared": False,
        "log_level": "TRACE",
        "dory-ctrl:log_level": "TRACE",
        "dory-connection:log_level": "TRACE",
        "dory-crypto:log_level": "TRACE",
        "lto": True,
    }
    generators = "cmake"
    exports_sources = "src/*"
    python_requires = "dory-compiler-options/0.0.1@dory/stable"

    def configure(self):
        pass

    def requirements(self):
        self.requires("dory-connection/0.0.1")
        self.requires("dory-memstore/0.0.1")
        self.requires("dory-ctrl/0.0.1")
        self.requires("dory-shared/0.0.1")
        self.requires("dory-external/0.0.1")
        self.requires("dory-log/0.0.1")
        self.requires("dory-memory/0.0.1")
        self.requires("dory-crash-consensus/0.0.1")
        

    def build(self):
        generator = self.python_requires["dory-compiler-options"].module.generator()
        print(self.python_requires["dory-compiler-options"].module)
        cmake = CMake(self, generator = generator)
        self.python_requires["dory-compiler-options"].module.set_options(cmake)
        cmake.definitions["DORY_LTO"] = str(self.options.lto).upper()
        # cmake.definitions["CONAN_CXX_FLAGS"] = "-Wno-conversion -Wno-error=noexcept -Wno-error=undef -Wno-error=old-style-cast -Wno-error=return-type"
        # cmake.definitions['SPDLOG_ACTIVE_LEVEL'] = "SPDLOG_LEVEL_{}".format(self.options.log_level)
        cmake.configure(source_folder="src")
        cmake.build()

        #../oneTBB/build/linux_intel64_gcc_cc4.9.3_libc2.17_kernel3.10.0_release/ -ltbb -ltbbmalloc -lrt -Iinclude;
