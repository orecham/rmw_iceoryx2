// Copyright (c) 2024 by Ekxide IO GmbH All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "rmw_iceoryx2_cxx/iox2/subscriber_impl.hpp"

#include "rmw_iceoryx2_cxx/error_message.hpp"
#include "rmw_iceoryx2_cxx/iox2/iceoryx2.hpp"
#include "rmw_iceoryx2_cxx/iox2/names.hpp"

namespace rmw::iox2
{

SubscriberImpl::SubscriberImpl(
    CreationLock, iox::optional<ErrorType>& error, NodeImpl& node, const char* topic, const char* type)
    : m_topic{topic}
    , m_type{type}
    , m_service_name{::rmw::iox2::names::topic(topic)} {
    auto iox2_service_name = Iceoryx2::ServiceName::create(m_service_name.c_str());
    if (iox2_service_name.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox::into<const char*>(iox2_service_name.error()));
        error.emplace(ErrorType::SERVICE_NAME_CREATION_FAILURE);
        return;
    }

    auto iox2_pubsub_service = node.iox2()
                                   .ipc()
                                   .service_builder(iox2_service_name.value())
                                   .publish_subscribe<Payload>()
                                   // TODO: make configurable
                                   .max_publishers(64)
                                   .max_subscribers(64)
                                   .payload_alignment(8) // All ROS2 messages have alignment 8. Maybe?
                                   .open_or_create();    // TODO: set attribute for ROS typename
    if (iox2_pubsub_service.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox::into<const char*>(iox2_pubsub_service.error()));
        error.emplace(ErrorType::SERVICE_CREATION_FAILURE);
        return;
    }

    auto iox2_subscriber = iox2_pubsub_service.value().subscriber_builder().create();
    if (iox2_subscriber.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox::into<const char*>(iox2_subscriber.error()));
        error.emplace(ErrorType::SUBSCRIBER_CREATION_FAILURE);
        return;
    }
    m_iox2_unique_id.emplace(iox2_subscriber->id());
    m_iox2_subscriber.emplace(std::move(iox2_subscriber.value()));
}

auto SubscriberImpl::unique_id() -> const iox::optional<RawIdType>& {
    auto& bytes = m_iox2_unique_id->bytes();
    return bytes;
}

auto SubscriberImpl::topic() const -> const std::string& {
    return m_topic;
}

auto SubscriberImpl::type() const -> const std::string& {
    return m_type;
}

auto SubscriberImpl::service_name() const -> const std::string& {
    return m_service_name;
}

auto SubscriberImpl::take_copy(void* dest) -> iox::expected<bool, ErrorType> {
    using iox::err;
    using iox::nullopt;
    using iox::ok;
    using iox::optional;

    auto result = m_iox2_subscriber->receive();
    if (result.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox::into<const char*>(result.error()));
        return err(ErrorType::RECV_FAILURE);
    }
    auto sample = std::move(result.value());

    if (sample.has_value()) {
        auto payload = sample.value().payload();
        std::memcpy(dest, payload.data(), payload.number_of_bytes());
        return ok(true);
    } else {
        return ok(false);
    }
}

auto SubscriberImpl::take_loan() -> iox::expected<iox::optional<const void*>, ErrorType> {
    using iox::err;
    using iox::nullopt;
    using iox::ok;
    using iox::optional;

    auto result = m_iox2_subscriber->receive();
    if (result.has_error()) {
        RMW_IOX2_CHAIN_ERROR_MSG(::iox::into<const char*>(result.error()));
        return err(ErrorType::RECV_FAILURE);
    }
    auto sample = std::move(result.value());

    if (sample.has_value()) {
        auto ptr = sample.value().payload().data();
        m_registry.store(std::move(sample.value()));
        return ok(optional<const void*>(ptr));
    } else {
        return ok(optional<const void*>{nullopt});
    }
}

auto SubscriberImpl::return_loan(void* loaned_memory) -> iox::expected<void, ErrorType> {
    using ::iox::err;
    using ::iox::ok;

    if (auto result = m_registry.release(static_cast<uint8_t*>(loaned_memory)); result.has_error()) {
        switch (result.error()) {
        case SampleRegistryError::INVALID_PAYLOAD:
            return err(ErrorType::INVALID_PAYLOAD);
        }
    }
    return ok();
}

} // namespace rmw::iox2
