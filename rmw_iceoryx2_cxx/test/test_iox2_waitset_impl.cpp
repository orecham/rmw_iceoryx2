// Copyright (c) 2024 by Ekxide IO GmbH All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include <gtest/gtest.h>

#include "iox/optional.hpp"
#include "rmw_iceoryx2_cxx/create.hpp"
#include "rmw_iceoryx2_cxx/iox2/waitset_impl.hpp"
#include "testing/base.hpp"

namespace
{

using namespace rmw::iox2::testing;

class RmwWaitSetImplTest : public TestBase
{
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(RmwWaitSetImplTest, construction) {
    using ::rmw::iox2::ContextImpl;
    using ::rmw::iox2::create_in_place;
    using ::rmw::iox2::WaitSetImpl;

    iox::optional<ContextImpl> context_storage;
    create_in_place(context_storage, test_id()).expect("failed to create context for waitset creation");
    auto& context = context_storage.value();

    iox::optional<WaitSetImpl> waitset_storage;
    ASSERT_FALSE(create_in_place(waitset_storage, context).has_error());
}

} // namespace
