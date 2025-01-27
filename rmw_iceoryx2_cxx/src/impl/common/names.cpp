// Copyright (c) 2024 by Ekxide IO GmbH All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "rmw_iceoryx2_cxx/impl/common/names.hpp"

#include <unistd.h>

namespace rmw::iox2::names
{

std::string context(const uint32_t context_id) {
    return "ros2://context/" + std::to_string(context_id);
}

std::string node(const uint32_t context_id, const char* name, const char* ns) {
    auto s = "ros2://context/" + std::to_string(context_id) + "/nodes/";
    if (ns && ns[0] != '\0') {
        s += std::string(ns) + "/";
    }
    s += std::string(name);
    return s;
}

std::string guard_condition(const uint32_t context_id, const uint32_t guard_condition_id) {
    return "ros2://context/" + std::to_string(context_id) + "/guard_conditions/" + std::to_string(guard_condition_id);
}

std::string topic(const char* topic) {
    auto s = "ros2://topics" + std::string(topic);
    return s;
}

} // namespace rmw::iox2::names
