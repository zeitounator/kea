// Copyright (C) 2016-2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>
#include <agent/ca_cfg_mgr.h>
#include <agent/parser_context.h>
#include <process/testutils/d_test_stubs.h>
#include <process/d_cfg_mgr.h>
#include <agent/tests/test_libraries.h>
#include <hooks/libinfo.h>
#include <boost/scoped_ptr.hpp>
#include <gtest/gtest.h>

using namespace isc::agent;
using namespace isc::data;
using namespace isc::hooks;
using namespace isc::process;

namespace  {

/// @brief Almost regular agent CfgMgr with internal parse method exposed.
class NakedAgentCfgMgr : public CtrlAgentCfgMgr {
public:
    using CtrlAgentCfgMgr::parse;
};

// Tests construction of CtrlAgentCfgMgr class.
TEST(CtrlAgentCfgMgr, construction) {
    boost::scoped_ptr<CtrlAgentCfgMgr> cfg_mgr;

    // Verify that configuration manager constructions without error.
    ASSERT_NO_THROW(cfg_mgr.reset(new CtrlAgentCfgMgr()));

    // Verify that the context can be retrieved and is not null.
    CtrlAgentCfgContextPtr context;
    ASSERT_NO_THROW(context = cfg_mgr->getCtrlAgentCfgContext());
    EXPECT_TRUE(context);

    // Verify that the manager can be destructed without error.
    EXPECT_NO_THROW(cfg_mgr.reset());
}

// Tests if getContext can be retrieved.
TEST(CtrlAgentCfgMgr, getContext) {
    CtrlAgentCfgMgr cfg_mgr;

    CtrlAgentCfgContextPtr ctx;
    ASSERT_NO_THROW(ctx = cfg_mgr.getCtrlAgentCfgContext());
    ASSERT_TRUE(ctx);
}

// Tests if context can store and retrieve HTTP parameters
TEST(CtrlAgentCfgMgr, contextHttpParams) {
    CtrlAgentCfgContext ctx;

    // Check http parameters
    ctx.setHttpPort(12345);
    EXPECT_EQ(12345, ctx.getHttpPort());

    ctx.setHttpHost("alnitak");
    EXPECT_EQ("alnitak", ctx.getHttpHost());
}

// Tests if context can store and retrieve control socket information.
TEST(CtrlAgentCfgMgr, contextSocketInfo) {

    CtrlAgentCfgContext ctx;

    // Check control socket parameters
    // By default, there are no control sockets stored.
    EXPECT_FALSE(ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2));
    EXPECT_FALSE(ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4));
    EXPECT_FALSE(ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6));

    ConstElementPtr socket1 = Element::fromJSON("{ \"socket-type\": \"unix\",\n"
                                                "  \"socket-name\": \"socket1\" }");
    ConstElementPtr socket2 = Element::fromJSON("{ \"socket-type\": \"unix\",\n"
                                                "  \"socket-name\": \"socket2\" }");
    ConstElementPtr socket3 = Element::fromJSON("{ \"socket-type\": \"unix\",\n"
                                                "  \"socket-name\": \"socket3\" }");
    // Ok, now set the control socket for D2
    EXPECT_NO_THROW(ctx.setControlSocketInfo(socket1, CtrlAgentCfgContext::TYPE_D2));

    // Now check the values returned
    EXPECT_EQ(socket1, ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2));
    EXPECT_FALSE(ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4));
    EXPECT_FALSE(ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6));

    // Now set the v6 socket and sanity check again
    EXPECT_NO_THROW(ctx.setControlSocketInfo(socket2, CtrlAgentCfgContext::TYPE_DHCP6));

    // Should be possible to retrieve two sockets.
    EXPECT_EQ(socket1, ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2));
    EXPECT_EQ(socket2, ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6));
    EXPECT_FALSE(ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4));

    // Finally, set the third control socket.
    EXPECT_NO_THROW(ctx.setControlSocketInfo(socket3, CtrlAgentCfgContext::TYPE_DHCP4));
    EXPECT_EQ(socket1, ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2));
    EXPECT_EQ(socket2, ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6));
    EXPECT_EQ(socket3, ctx.getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4));
}

// Tests if copied context retains all parameters.
TEST(CtrlAgentCfgMgr, contextSocketInfoCopy) {

    CtrlAgentCfgContext ctx;

    ConstElementPtr socket1 = Element::fromJSON("{ \"socket-type\": \"unix\",\n"
                                                "  \"socket-name\": \"socket1\" }");
    ConstElementPtr socket2 = Element::fromJSON("{ \"socket-type\": \"unix\",\n"
                                                "  \"socket-name\": \"socket2\" }");
    ConstElementPtr socket3 = Element::fromJSON("{ \"socket-type\": \"unix\",\n"
                                                "  \"socket-name\": \"socket3\" }");
    // Ok, now set the control sockets
    EXPECT_NO_THROW(ctx.setControlSocketInfo(socket1, CtrlAgentCfgContext::TYPE_D2));
    EXPECT_NO_THROW(ctx.setControlSocketInfo(socket2, CtrlAgentCfgContext::TYPE_DHCP4));
    EXPECT_NO_THROW(ctx.setControlSocketInfo(socket3, CtrlAgentCfgContext::TYPE_DHCP6));

    EXPECT_NO_THROW(ctx.setHttpPort(12345));
    EXPECT_NO_THROW(ctx.setHttpHost("bellatrix"));

    HookLibsCollection libs;
    string exp_name("testlib1.so");
    ConstElementPtr exp_param(new StringElement("myparam"));
    libs.push_back(make_pair(exp_name, exp_param));
    ctx.setLibraries(libs);

    // Make a copy.
    DCfgContextBasePtr copy_base(ctx.clone());
    CtrlAgentCfgContextPtr copy = boost::dynamic_pointer_cast<CtrlAgentCfgContext>(copy_base);
    ASSERT_TRUE(copy);

    // Now check the values returned
    EXPECT_EQ(12345, copy->getHttpPort());
    EXPECT_EQ("bellatrix", copy->getHttpHost());

    // Check socket info
    ASSERT_TRUE(copy->getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2));
    ASSERT_TRUE(copy->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4));
    ASSERT_TRUE(copy->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6));
    EXPECT_EQ(socket1->str(), copy->getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2)->str());
    EXPECT_EQ(socket2->str(), copy->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4)->str());
    EXPECT_EQ(socket3->str(), copy->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6)->str());

    // Check hook libs
    HookLibsCollection libs2 = copy->getLibraries();
    ASSERT_EQ(1, libs2.size());
    EXPECT_EQ(exp_name, libs2[0].first);
    ASSERT_TRUE(libs2[0].second);
    EXPECT_EQ(exp_param->str(), libs2[0].second->str());
}


// Tests if the context can store and retrieve hook libs information.
TEST(CtrlAgentCfgMgr, contextHookParams) {
    CtrlAgentCfgContext ctx;

    // By default there should be no hooks.
    HookLibsCollection libs = ctx.getLibraries();
    EXPECT_TRUE(libs.empty());

    libs.push_back(std::make_pair("libone.so", ConstElementPtr()));
    libs.push_back(std::make_pair("libtwo.so", Element::fromJSON("{\"foo\": true}")));
    libs.push_back(std::make_pair("libthree.so", Element::fromJSON("{\"bar\": 42}")));

    ctx.setLibraries(libs);

    HookLibsCollection stored_libs = ctx.getLibraries();
    EXPECT_EQ(3, stored_libs.size());

    EXPECT_EQ(libs, stored_libs);
}

/// Control Agent configurations used in tests.
const char* AGENT_CONFIGS[] = {

    // configuration 0: empty (nothing specified)
    "{ }",

    // Configuration 1: http parameters only (no control sockets, not hooks)
    "{  \"http-host\": \"betelguese\",\n"
    "    \"http-port\": 8001\n"
    "}",

    // Configuration 2: http and 1 socket
    "{\n"
    "    \"http-host\": \"betelguese\",\n"
    "    \"http-port\": 8001,\n"
    "    \"control-sockets\": {\n"
    "        \"dhcp4-server\": {\n"
    "            \"socket-name\": \"/tmp/socket-v4\"\n"
    "        }\n"
    "    }\n"
    "}",

    // Configuration 3: http and all 3 sockets
    "{\n"
    "    \"http-host\": \"betelguese\",\n"
    "    \"http-port\": 8001,\n"
    "    \"control-sockets\": {\n"
    "        \"dhcp4-server\": {\n"
    "            \"socket-name\": \"/tmp/socket-v4\"\n"
    "        },\n"
    "        \"dhcp6-server\": {\n"
    "            \"socket-name\": \"/tmp/socket-v6\"\n"
    "        },\n"
    "        \"d2-server\": {\n"
    "            \"socket-name\": \"/tmp/socket-d2\"\n"
    "        }\n"
    "   }\n"
    "}",

    // Configuration 4: http, 1 socket and hooks
    // CA is able to load hook libraries that augment its operation.
    // The primary functionality is the ability to add new commands.
    "{\n"
    "    \"http-host\": \"betelguese\",\n"
    "    \"http-port\": 8001,\n"
    "    \"control-sockets\": {\n"
    "        \"dhcp4-server\": {\n"
    "            \"socket-name\": \"/tmp/socket-v4\"\n"
    "        }\n"
    "   },\n"
    "    \"hooks-libraries\": ["
    "        {"
    "          \"library\": \"%LIBRARY%\","
    "              \"parameters\": {\n"
    "              \"param1\": \"foo\"\n"
    "            }\n"
    "        }\n"
    "     ]\n"
    "}",

    // Configuration 5: http and 1 socket (d2 only)
    "{\n"
    "    \"http-host\": \"betelguese\",\n"
    "    \"http-port\": 8001,\n"
    "    \"control-sockets\": {\n"
    "        \"d2-server\": {\n"
    "            \"socket-name\": \"/tmp/socket-d2\"\n"
    "        }\n"
    "    }\n"
    "}",

    // Configuration 6: http and 1 socket (dhcp6 only)
    "{\n"
    "    \"http-host\": \"betelguese\",\n"
    "    \"http-port\": 8001,\n"
    "    \"control-sockets\": {\n"
    "        \"dhcp6-server\": {\n"
    "            \"socket-name\": \"/tmp/socket-v6\"\n"
    "        }\n"
    "    }\n"
    "}"
};

/// @brief Class used for testing CfgMgr
class AgentParserTest : public isc::process::ConfigParseTest {
public:

    /// @brief Tries to load input text as a configuration
    ///
    /// @param config text containing input configuration
    /// @param expected_answer expected result of configuration (0 = success)
    void configParse(const char* config, int expected_answer) {
        isc::agent::ParserContext parser;
        ConstElementPtr json = parser.parseString(config, ParserContext::PARSER_SUB_AGENT);

        EXPECT_NO_THROW(answer_ = cfg_mgr_.parse(json, false));
        EXPECT_TRUE(checkAnswer(expected_answer));
    }

    /// @brief Replaces %LIBRARY% with specified library name
    ///
    /// @param config input config text (should contain "%LIBRARY%" string)
    /// @param lib_name %LIBRARY% will be replaced with that name
    /// @return configuration text with library name replaced
    std::string pathReplacer(const char* config, const char* lib_name) {
        string txt(config);
        txt.replace(txt.find("%LIBRARY%"), strlen("%LIBRARY%"), string(lib_name));
        return (txt);
    }

    /// Configuration Manager (used in tests)
    NakedAgentCfgMgr cfg_mgr_;
};

// This test verifies if an empty config is handled properly. In practice such
// a config makes little sense, but perhaps it's ok for a default deployment.
// Sadly, our bison parser requires at last one parameter to be present.
// Until we determine whether we want the empty config to be allowed or not,
// this test remains disabled.
TEST_F(AgentParserTest, DISABLED_configParseEmpty) {
    configParse(AGENT_CONFIGS[0], 0);
}

// This test checks if a config with only HTTP parameters is parsed properly.
TEST_F(AgentParserTest, configParseHttpOnly) {
    configParse(AGENT_CONFIGS[1], 0);

    CtrlAgentCfgContextPtr ctx = cfg_mgr_.getCtrlAgentCfgContext();
    ASSERT_TRUE(ctx);
    EXPECT_EQ("betelguese", ctx->getHttpHost());
    EXPECT_EQ(8001, ctx->getHttpPort());
}

// Tests if a single socket can be configured. BTW this test also checks
// if default value for socket-type is specified (the config doesn't have it,
// so the default value should be filed in).
TEST_F(AgentParserTest, configParseSocketDhcp4) {
    configParse(AGENT_CONFIGS[2], 0);

    CtrlAgentCfgContextPtr ctx = cfg_mgr_.getCtrlAgentCfgContext();
    ASSERT_TRUE(ctx);
    ConstElementPtr socket = ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4);
    ASSERT_TRUE(socket);
    EXPECT_EQ("{ \"socket-name\": \"/tmp/socket-v4\", \"socket-type\": \"unix\" }",
              socket->str());
    EXPECT_FALSE(ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6));
    EXPECT_FALSE(ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2));
}

// Tests if a single socket can be configured. BTW this test also checks
// if default value for socket-type is specified (the config doesn't have it,
// so the default value should be filed in).
TEST_F(AgentParserTest, configParseSocketD2) {
    configParse(AGENT_CONFIGS[5], 0);

    CtrlAgentCfgContextPtr ctx = cfg_mgr_.getCtrlAgentCfgContext();
    ASSERT_TRUE(ctx);
    ConstElementPtr socket = ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2);
    ASSERT_TRUE(socket);
    EXPECT_EQ("{ \"socket-name\": \"/tmp/socket-d2\", \"socket-type\": \"unix\" }",
              socket->str());

    EXPECT_FALSE(ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4));
    EXPECT_FALSE(ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6));
}

// Tests if a single socket can be configured. BTW this test also checks
// if default value for socket-type is specified (the config doesn't have it,
// so the default value should be filed in).
TEST_F(AgentParserTest, configParseSocketDhcp6) {
    configParse(AGENT_CONFIGS[6], 0);

    CtrlAgentCfgContextPtr ctx = cfg_mgr_.getCtrlAgentCfgContext();
    ASSERT_TRUE(ctx);
    ConstElementPtr socket = ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6);
    ASSERT_TRUE(socket);
    EXPECT_EQ("{ \"socket-name\": \"/tmp/socket-v6\", \"socket-type\": \"unix\" }",
              socket->str());
    EXPECT_FALSE(ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4));
    EXPECT_FALSE(ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2));
}

// This tests if all 3 sockets can be configured and makes sure the parser
// doesn't confuse them.
TEST_F(AgentParserTest, configParse3Sockets) {
    configParse(AGENT_CONFIGS[3], 0);
    CtrlAgentCfgContextPtr ctx = cfg_mgr_.getCtrlAgentCfgContext();
    ASSERT_TRUE(ctx);
    ConstElementPtr socket2 = ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_D2);
    ConstElementPtr socket4 = ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP4);
    ConstElementPtr socket6 = ctx->getControlSocketInfo(CtrlAgentCfgContext::TYPE_DHCP6);
    ASSERT_TRUE(socket2);
    EXPECT_EQ("{ \"socket-name\": \"/tmp/socket-d2\", \"socket-type\": \"unix\" }",
              socket2->str());
    ASSERT_TRUE(socket4);
    EXPECT_EQ("{ \"socket-name\": \"/tmp/socket-v4\", \"socket-type\": \"unix\" }",
              socket4->str());
    ASSERT_TRUE(socket6);
    EXPECT_EQ("{ \"socket-name\": \"/tmp/socket-v6\", \"socket-type\": \"unix\" }",
              socket6->str());
}

// This test checks that the config file with hook library specified can be
// loaded. This one is a bit tricky, because the parser sanity checks the lib
// name. In particular, it checks if such a library exists. Therefore we
// can't use AGENT_CONFIGS[4] as is, but need to run it through path replacer.
TEST_F(AgentParserTest, configParseHooks) {
    // Create the configuration with proper lib path.
    std::string cfg = pathReplacer(AGENT_CONFIGS[4], BASIC_CALLOUT_LIBRARY);
    // The configuration should be successful.
    configParse(cfg.c_str(), 0);

    // The context now should have the library specified.
    CtrlAgentCfgContextPtr ctx = cfg_mgr_.getCtrlAgentCfgContext();
    HookLibsCollection libs = ctx->getLibraries();
    ASSERT_EQ(1, libs.size());
    EXPECT_EQ(string(BASIC_CALLOUT_LIBRARY), libs[0].first);
    ASSERT_TRUE(libs[0].second);
    EXPECT_EQ("{ \"param1\": \"foo\" }", libs[0].second->str());
}


}; // end of anonymous namespace
