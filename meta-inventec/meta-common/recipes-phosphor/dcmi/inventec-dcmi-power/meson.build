project(
    'inventec-dcmi-power',
    'cpp',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++17'
    ],
    license: 'Apache-2.0',
    version: '0.1',
)

add_project_arguments(
    '-DBOOST_SYSTEM_NO_DEPRECATED',
    '-DBOOST_ERROR_CODE_HEADER_ONLY',
    '-DBOOST_NO_RTTI',
    '-DBOOST_NO_TYPEID',
    '-DBOOST_ALL_NO_LIB',
    '-DBOOST_ASIO_DISABLE_THREADS',
    '-DBOOST_ALLOW_DEPRECATED_HEADERS',
    language: 'cpp',
)
env = environment()
env.set('PKG_CONFIG_PATH','/usr/bin/pkg-config')

sdbusplus = dependency('sdbusplus')
systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_pkgconfig_variable(
    'systemdsystemunitdir',
    define_variable: ['prefix', get_option('prefix')])
threads = dependency('threads')
nlohmann_json = dependency('nlohmann_json')


subdir('service_files')
subdir('src')
