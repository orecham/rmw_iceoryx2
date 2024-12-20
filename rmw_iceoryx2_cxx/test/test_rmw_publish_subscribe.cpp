// Copyright (c) 2024 by Ekxide IO GmbH All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include <gtest/gtest.h>

#include "rmw/rmw.h"
#include "rmw_iceoryx2_cxx_test_msgs/msg/defaults.hpp"
#include "testing/assertions.hpp"
#include "testing/base.hpp"

namespace
{
using namespace rmw::iox2::testing;

class RmwPublishSubscribeTest : public TestBase
{
protected:
    void SetUp() override {
        initialize();
    }

    void TearDown() override {
        cleanup();
        print_rmw_errors();
    }
};

TEST_F(RmwPublishSubscribeTest, publish_subscribe_loaned_defaults) {
    using rmw_iceoryx2_cxx_test_msgs::msg::Defaults;

    auto* publisher = create_default_publisher<Defaults>(create_test_topic());
    EXPECT_NE(publisher, nullptr);
    auto* subscription = create_default_subscriber<Defaults>(create_test_topic());
    EXPECT_NE(subscription, nullptr);

    void* publisher_loan = nullptr;
    EXPECT_RMW_OK(rmw_borrow_loaned_message(publisher, test_type_support<Defaults>(), &publisher_loan));
    new (publisher_loan) Defaults{};
    EXPECT_RMW_OK(rmw_publish_loaned_message(publisher, publisher_loan, nullptr));

    void* subscriber_loan = nullptr;
    bool taken{false};
    EXPECT_RMW_OK(rmw_take_loaned_message(subscription, &subscriber_loan, &taken, nullptr));
    ASSERT_NE(subscriber_loan, nullptr);
    ASSERT_TRUE(taken);

    ASSERT_EQ(*reinterpret_cast<Defaults*>(subscriber_loan), Defaults{});

    ASSERT_RMW_OK(rmw_return_loaned_message_from_subscription(subscription, subscriber_loan));
}

TEST_F(RmwPublishSubscribeTest, publish_subscribe_copied_defaults) {
    using rmw_iceoryx2_cxx_test_msgs::msg::Defaults;

    auto* publisher = create_default_publisher<Defaults>(create_test_topic());
    auto* subscription = create_default_subscriber<Defaults>(create_test_topic());

    auto ros_message = Defaults{};
    EXPECT_RMW_OK(rmw_publish(publisher, &ros_message, nullptr));

    void* subscriber_loan = nullptr;
    bool taken{false};
    EXPECT_RMW_OK(rmw_take_loaned_message(subscription, &subscriber_loan, &taken, nullptr));
    ASSERT_NE(subscriber_loan, nullptr);
    ASSERT_TRUE(taken);

    ASSERT_EQ(*reinterpret_cast<Defaults*>(subscriber_loan), Defaults{});

    ASSERT_RMW_OK(rmw_return_loaned_message_from_subscription(subscription, subscriber_loan));
}

} // namespace
