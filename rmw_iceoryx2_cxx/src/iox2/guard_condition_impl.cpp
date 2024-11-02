// Copyright (c) 2024 by Ekxide IO GmbH All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "rmw_iceoryx2_cxx/iox2/guard_condition_impl.hpp"

#include "iox/optional.hpp"
#include "iox2/event_id.hpp"
#include "rmw_iceoryx2_cxx/error_handling.hpp"
#include "rmw_iceoryx2_cxx/iox2/names.hpp"
#include "rmw_iceoryx2_cxx/iox2/node_impl.hpp"
#include <iostream>

namespace rmw::iox2
{

GuardConditionImpl::GuardConditionImpl(
    CreationLock, iox::optional<ErrorType>& error, NodeImpl& node, const uint32_t context_id, const uint32_t trigger_id)
    : m_trigger_id{trigger_id}
    , m_service_name{names::guard_condition(context_id, m_trigger_id)} {
    using ::iox2::ServiceName;

    auto service_name = ServiceName::create(m_service_name.c_str());
    if (service_name.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox2::error_string(service_name.error()));
        error.emplace(ErrorType::SERVICE_NAME_CREATION_FAILURE);
        return;
    }

    auto service = node.as_iox2().service_builder(service_name.value()).event().open_or_create();
    if (service.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox2::error_string(service.error()));
        error.emplace(ErrorType::SERVICE_CREATION_FAILURE);
        return;
    }

    auto notifier = service.value().notifier_builder().create();
    if (notifier.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox2::error_string(notifier.error()));
        error.emplace(ErrorType::NOTIFIER_CREATION_FAILURE);
        return;
    }

    m_notifier.emplace(std::move(notifier.value()));
};

auto GuardConditionImpl::trigger_id() const -> uint32_t {
    return m_trigger_id;
}

auto GuardConditionImpl::unique_id() -> iox::optional<RawIdType>& {
    auto& bytes = m_unique_id->bytes();
    return bytes;
}


auto GuardConditionImpl::service_name() const -> const std::string& {
    return m_service_name;
}

auto GuardConditionImpl::trigger() -> iox::expected<void, ErrorType> {
    using ::iox::err;
    using ::iox::ok;
    using ::iox2::EventId;

    if (auto result = m_notifier->notify_with_custom_event_id(EventId(trigger_id())); result.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox2::error_string(result.error()));
        return err(ErrorType::NOTIFICATION_FAILURE);
    };

    return ok();
}

} // namespace rmw::iox2
