licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "tap_interface",
    hdrs = ["tap.h"],
    deps = [
        ":tap_matcher",
        "//include/envoy/http:header_map_interface",
        "@envoy_api//envoy/data/tap/v2alpha:wrapper_cc",
        "@envoy_api//envoy/service/tap/v2alpha:common_cc",
    ],
)

envoy_cc_library(
    name = "tap_config_base",
    srcs = ["tap_config_base.cc"],
    hdrs = ["tap_config_base.h"],
    deps = [
        ":tap_interface",
        ":tap_matcher",
        "//source/common/common:assert_lib",
    ],
)

envoy_cc_library(
    name = "tap_matcher",
    srcs = ["tap_matcher.cc"],
    hdrs = ["tap_matcher.h"],
    deps = [
        "//source/common/http:header_utility_lib",
        "@envoy_api//envoy/service/tap/v2alpha:common_cc",
    ],
)

envoy_cc_library(
    name = "admin",
    srcs = ["admin.cc"],
    hdrs = ["admin.h"],
    deps = [
        ":tap_interface",
        "//include/envoy/server:admin_interface",
        "//include/envoy/singleton:manager_interface",
        "//source/common/buffer:buffer_lib",
        "@envoy_api//envoy/admin/v2alpha:tap_cc",
    ],
)

envoy_cc_library(
    name = "extension_config_base",
    srcs = ["extension_config_base.cc"],
    hdrs = ["extension_config_base.h"],
    deps = [
        "//include/envoy/thread_local:thread_local_interface",
        "//source/extensions/common/tap:admin",
        "@envoy_api//envoy/config/common/tap/v2alpha:common_cc",
    ],
)
